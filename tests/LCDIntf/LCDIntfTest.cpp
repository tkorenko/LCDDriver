#include "CppUTest/TestHarness.h"
#include <stdint.h>
extern "C"
{
#include "LCDPort.h"
#include "LCDIntf.h"
#include "MockPeriphIO.h"
};
#include "LCDPortSpy.h"
#include <stdio.h>

#define HI_NIB(byte) ((byte >> 4) & 0x0F)
#define LO_NIB(byte) (byte & 0x0F)


/* ====================================================================== */
/*   There are a couple of delays cruical within the LCD Controller       */
/* initialization procedures.  Lets intercept them, too: a call for delay */
/* will be directed to MockIO.                                            */
/* ====================================================================== */
enum {
    DELAY_MICROSECONDS_FAKE_CALL = 0x3FFFFFEF,
};

extern "C" void
Delay_microseconds(uint32_t microseconds)
{
    MockPeriphIO_Write(DELAY_MICROSECONDS_FAKE_CALL, microseconds);
}

/* ====================================================================== */
TEST_GROUP(AnLCDIntf_InitAndDestroy)
{
    void setup() override {
        LCDPortSpy_ResetToDefaultState();
    }
};

TEST(AnLCDIntf_InitAndDestroy, LCDPortIsNotInitializedOnStartup) {
    LONGS_EQUAL(LINE_STATE_UNDEFINED, LCDPortSpy_GetCE());
}

TEST(AnLCDIntf_InitAndDestroy, InitializationPropagatesToLCDPort) {
    LCDIntf_Init(LCD_PORT_DATA_WIDTH_8_BIT);

    LONGS_EQUAL(LINE_STATE_DEASSERTED, LCDPortSpy_GetCE());
}

TEST(AnLCDIntf_InitAndDestroy, InitSetsPortDataWidthTo8Bits) {
    LCDIntf_Init(LCD_PORT_DATA_WIDTH_8_BIT);

    LONGS_EQUAL(LCD_PORT_DATA_WIDTH_8_BIT, LCDIntf_GetPortDataWidth());
}

TEST(AnLCDIntf_InitAndDestroy, DeinitializationPropagatesToLCDPort) {
    LCDIntf_Init(LCD_PORT_DATA_WIDTH_8_BIT);

    LCDIntf_Deinit();

    LONGS_EQUAL(LINE_STATE_UNDEFINED, LCDPortSpy_GetCE());
}

TEST(AnLCDIntf_InitAndDestroy, DeinitUndefinesPortDataWidth) {
    LCDIntf_Init(LCD_PORT_DATA_WIDTH_8_BIT);

    LCDIntf_Deinit();

    LONGS_EQUAL(LCD_PORT_DATA_WIDTH_UNDEFINED, LCDIntf_GetPortDataWidth());
}

TEST(AnLCDIntf_InitAndDestroy, InitCanSetPortDataWidthTo4Bits) {
    LCDIntf_Init(LCD_PORT_DATA_WIDTH_4_BIT);

    LONGS_EQUAL(LCD_PORT_DATA_WIDTH_4_BIT, LCDIntf_GetPortDataWidth());
}

/* ====================================================================== */
struct LCDIntf : public Utest
{
    void Expect_SetRS() {
        MockPeriphIO_Expect_Write(LCD_RS_ADDR, LINE_STATE_ASSERTED);
    }

    void Expect_SetRW() {
        MockPeriphIO_Expect_Write(LCD_RW_ADDR, LINE_STATE_ASSERTED);
    }

    void Expect_SetCE() {
        MockPeriphIO_Expect_Write(LCD_CE_ADDR, LINE_STATE_ASSERTED);
    }

    void Expect_ClearRS() {
        MockPeriphIO_Expect_Write(LCD_RS_ADDR, LINE_STATE_DEASSERTED);
    }

    void Expect_ClearRW() {
        MockPeriphIO_Expect_Write(LCD_RW_ADDR, LINE_STATE_DEASSERTED);
    }

    void Expect_ClearCE() {
        MockPeriphIO_Expect_Write(LCD_CE_ADDR, LINE_STATE_DEASSERTED);
    }

    void ExpectCall_Delay_microseconds(uint32_t microseconds) {
        MockPeriphIO_Expect_Write(DELAY_MICROSECONDS_FAKE_CALL, microseconds);
    }
};

struct LCDIntf_11Wires : public LCDIntf
{
    void Expect_GetData8ThenReturn(int32_t data) {
        MockPeriphIO_Expect_ReadThenReturn(LCD_DATA8_ADDR, data);
    }

    void Expect_PutData8(int32_t data) {
        MockPeriphIO_Expect_Write(LCD_DATA8_ADDR, data);
    }

    void Expect_SetDirection_In8() {
        MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION8_REG,
            LCD_PORT_DATA_DIR_INPUT8);
    }

    void Expect_SetDirection_Out8() {
        MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION8_REG,
            LCD_PORT_DATA_DIR_OUTPUT8);
    }
};

/* ====================================================================== */
TEST_GROUP_BASE(AnLCDIntf_11Wires, LCDIntf_11Wires)
{
    void setup() override {
        MockPeriphIO_Create(10);
        LCDPortSpy_ResetToDefaultState();
        LCDIntf_Init(LCD_PORT_DATA_WIDTH_8_BIT);
    }

    void teardown() override {
        LCDIntf_Deinit();
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

TEST(AnLCDIntf_11Wires, WritesInstruction) {
    Expect_ClearRS();
    Expect_ClearRW();
    Expect_SetCE();
    Expect_PutData8(0x5A);
    Expect_SetDirection_Out8();
    Expect_ClearCE();
    Expect_SetDirection_In8();
    Expect_SetRW();
    
    LCDIntf_WriteInstruction(0x5A);
}

TEST(AnLCDIntf_11Wires, WritesData) {
    Expect_SetRS();
    Expect_ClearRW();
    Expect_SetCE();
    Expect_PutData8(0xA5);
    Expect_SetDirection_Out8();
    Expect_ClearCE();
    Expect_SetDirection_In8();
    Expect_SetRW();

    LCDIntf_WriteData(0xA5);
}

TEST(AnLCDIntf_11Wires, ReadsData) {
    Expect_SetRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData8ThenReturn(0x77);
    Expect_ClearCE();

    LONGS_EQUAL(0x77, LCDIntf_ReadData());
}

TEST(AnLCDIntf_11Wires, ReadsBusyFlagAndAddress) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData8ThenReturn(0x87);
    Expect_ClearCE();

    LONGS_EQUAL(0x87, LCDIntf_ReadInstruction());
}

TEST(AnLCDIntf_11Wires, WaitForBusyFlag_ReadyImmediately) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData8ThenReturn(READ_INSTRUCTION__NO_BUSY_FLAG);
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_OK, status);
}

TEST(AnLCDIntf_11Wires, WaitForBusyFlag_NotImmediatelyReady) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData8ThenReturn(READ_INSTRUCTION__BUSY_FLAG);
    Expect_GetData8ThenReturn(READ_INSTRUCTION__BUSY_FLAG);
    Expect_GetData8ThenReturn(READ_INSTRUCTION__NO_BUSY_FLAG);
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_OK, status);
}

TEST(AnLCDIntf_11Wires, WaitForBusyFlag_ActsUponBusyFlagOnly) {
    int32_t anyAddr = 0x05;
    int32_t busyFlagWithAddr  =   READ_INSTRUCTION__BUSY_FLAG  | anyAddr;
    int32_t noBusyFlagButAddr = (~READ_INSTRUCTION__BUSY_FLAG) & anyAddr;

    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData8ThenReturn(busyFlagWithAddr);
    Expect_GetData8ThenReturn(noBusyFlagButAddr);
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_OK, status);
}

TEST(AnLCDIntf_11Wires, WaitForBusyFlag_ReadTimeout) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    for (int i = 0; i < BUSY_FLAG_READS_BEFORE_GIVING_UP; ++i) 
        Expect_GetData8ThenReturn(READ_INSTRUCTION__BUSY_FLAG);
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, status);
}

/* ====================================================================== */
struct LCDControllerInit_11Wires : public LCDIntf_11Wires
{
    void ExpectSequence_8BitWriteInstruction(int32_t cmd) {
        Expect_ClearRS();
        Expect_ClearRW();
        Expect_SetCE();
        Expect_PutData8(cmd);
        Expect_SetDirection_Out8();
        Expect_ClearCE();
        Expect_SetDirection_In8();
        Expect_SetRW();
    }

    void ExpectSequence_8BitReadInstruction(int32_t retVal) {
        Expect_ClearRS();
        Expect_SetRW();
        Expect_SetCE();
        Expect_GetData8ThenReturn(retVal);
        Expect_ClearCE();
    }

    void ExpectSequence_8BitRead_NoBusyFlag() {
        ExpectSequence_8BitReadInstruction(READ_INSTRUCTION__NO_BUSY_FLAG);
    }

    // XXX consider better name
    void ExpectSequence_8BitRead_BusyFlag_ReadTimeout() {
        Expect_ClearRS();
        Expect_SetRW();
        Expect_SetCE();
        for (int i = 0; i < BUSY_FLAG_READS_BEFORE_GIVING_UP; ++i) 
            Expect_GetData8ThenReturn(READ_INSTRUCTION__BUSY_FLAG);
        Expect_ClearCE();
    }
};

/* ====================================================================== */
TEST_GROUP_BASE(AnLCDControllerInit_11Wires, LCDControllerInit_11Wires)
{
    void setup() override {
        MockPeriphIO_Create(70);
        LCDPortSpy_ResetToDefaultState();
        LCDIntf_Init(LCD_PORT_DATA_WIDTH_8_BIT);
    }

    void teardown() override {
        LCDIntf_Deinit();
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

TEST(AnLCDControllerInit_11Wires, PreparesDisplayFor8BitInterfacing) {
    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(39); // XXX Magic Number from datasheet

    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_8BitWriteInstruction(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(DISPLAY_CLEAR);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(ENTRY_MODE_SET__I_D_SH);
    ExpectSequence_8BitRead_NoBusyFlag();

 
    LCDIntf_InitializeLCDController();
}

TEST(AnLCDControllerInit_11Wires, ReportsSuccessInSetupOf8BitMode) {
    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_8BitWriteInstruction(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(DISPLAY_CLEAR);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(ENTRY_MODE_SET__I_D_SH);
    ExpectSequence_8BitRead_NoBusyFlag();

 
    LONGS_EQUAL(LCD_OPERATION_OK, LCDIntf_InitializeLCDController());
}

TEST(AnLCDControllerInit_11Wires, DetectsReadTimeoutOnDisplayControlInstruction)
{
    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_8BitWriteInstruction(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF);
    ExpectSequence_8BitRead_BusyFlag_ReadTimeout();


    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, LCDIntf_InitializeLCDController());
}

TEST(AnLCDControllerInit_11Wires, DetectsReadTimeoutOnDisplayClearInstruction)
{
    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_8BitWriteInstruction(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(DISPLAY_CLEAR);
    ExpectSequence_8BitRead_BusyFlag_ReadTimeout();


    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, LCDIntf_InitializeLCDController());
}

TEST(AnLCDControllerInit_11Wires, DetectsReadTimeoutOnEntryModeSetInstruction)
{
    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_8BitWriteInstruction(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_8BitWriteInstruction(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(DISPLAY_CLEAR);
    ExpectSequence_8BitRead_NoBusyFlag();

    ExpectSequence_8BitWriteInstruction(ENTRY_MODE_SET__I_D_SH);
    ExpectSequence_8BitRead_BusyFlag_ReadTimeout();


    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, LCDIntf_InitializeLCDController());
}

/* ====================================================================== */
struct LCDIntf_7Wires : public LCDIntf
{
    void Expect_PutData4(int32_t data) {
        MockPeriphIO_Expect_Write(LCD_DATA4_ADDR, data);
    }

    void Expect_GetData4ThenReturn(int32_t data) {
        MockPeriphIO_Expect_ReadThenReturn(LCD_DATA4_ADDR, data);
    }

    void Expect_SetDirection_In4() {
        MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION4_REG,
            LCD_PORT_DATA_DIR_INPUT4);
    }

    void Expect_SetDirection_Out4() {
        MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION4_REG,
            LCD_PORT_DATA_DIR_OUTPUT4);
    }
};

TEST_GROUP_BASE(AnLCDIntf_7Wires, LCDIntf_7Wires)
{
    void setup() override {
        MockPeriphIO_Create(15);
        LCDPortSpy_ResetToDefaultState();
        LCDIntf_Init(LCD_PORT_DATA_WIDTH_4_BIT);
    }

    void teardown() override {
        LCDIntf_Deinit();
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

TEST(AnLCDIntf_7Wires, WritesInstruction) {
    int32_t cmd = 0x7B;
    Expect_ClearRS();
    Expect_ClearRW();
    Expect_SetCE();
    Expect_PutData4( HI_NIB(cmd) );
    Expect_SetDirection_Out4();
    Expect_ClearCE();
    Expect_SetCE();
    Expect_PutData4( LO_NIB(cmd) );
    Expect_ClearCE();
    Expect_SetDirection_In4();
    Expect_SetRW();

    LCDIntf_WriteInstruction(0x7B);
}

TEST(AnLCDIntf_7Wires, WritesData) {
    int32_t data = 0x23;
    Expect_SetRS();
    Expect_ClearRW();
    Expect_SetCE();
    Expect_PutData4( HI_NIB(data) );
    Expect_SetDirection_Out4();
    Expect_ClearCE();
    Expect_SetCE();
    Expect_PutData4( LO_NIB(data) );
    Expect_ClearCE();
    Expect_SetDirection_In4();
    Expect_SetRW();

    LCDIntf_WriteData(data);
}

TEST(AnLCDIntf_7Wires, ReadsData) {
    int32_t data = 0x34;
    Expect_SetRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData4ThenReturn( HI_NIB(data) );
    Expect_ClearCE();
    Expect_SetCE();
    Expect_GetData4ThenReturn( LO_NIB(data) );
    Expect_ClearCE();

    LONGS_EQUAL(data, LCDIntf_ReadData());
}

TEST(AnLCDIntf_7Wires, ReadsBusyFlagAndAddress) {
    int32_t busyFlagAndAddr = 0x83;
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData4ThenReturn( HI_NIB(busyFlagAndAddr) );
    Expect_ClearCE();
    Expect_SetCE();
    Expect_GetData4ThenReturn( LO_NIB(busyFlagAndAddr) );
    Expect_ClearCE();

    LONGS_EQUAL(busyFlagAndAddr, LCDIntf_ReadInstruction());
}

TEST(AnLCDIntf_7Wires, WaitForBusyFlag_ReadyImmediately) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__NO_BUSY_FLAG) );
    Expect_ClearCE();
    Expect_SetCE();
    Expect_GetData4ThenReturn( LO_NIB(READ_INSTRUCTION__NO_BUSY_FLAG) );
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_OK, status);
}

TEST(AnLCDIntf_7Wires, WaitForBusyFlag_NotImmediatelyReady) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__BUSY_FLAG) );
    Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__BUSY_FLAG) );
    Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__NO_BUSY_FLAG) );
    Expect_ClearCE();
    Expect_SetCE();
    Expect_GetData4ThenReturn( LO_NIB(READ_INSTRUCTION__NO_BUSY_FLAG) );
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_OK, status);
}

TEST(AnLCDIntf_7Wires, WaitForBusyFlag_ActsUponBusyFlagOnly) {
    int32_t anyAddr = 0x05;
    int32_t busyFlagWithAddr  =   READ_INSTRUCTION__BUSY_FLAG  | anyAddr;
    int32_t noBusyFlagButAddr = (~READ_INSTRUCTION__BUSY_FLAG) & anyAddr;

    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    Expect_GetData4ThenReturn( HI_NIB(busyFlagWithAddr) );
    Expect_GetData4ThenReturn( HI_NIB(noBusyFlagButAddr) );
    Expect_ClearCE();
    Expect_SetCE();
    Expect_GetData4ThenReturn( LO_NIB(noBusyFlagButAddr) );
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_OK, status);
}

TEST(AnLCDIntf_7Wires, WaitForBusyFlag_ReadTimeout) {
    Expect_ClearRS();
    Expect_SetRW();
    Expect_SetCE();
    for (int i = 0; i < BUSY_FLAG_READS_BEFORE_GIVING_UP; ++i) 
        Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__BUSY_FLAG) );
    Expect_ClearCE();
    Expect_SetCE();
    Expect_GetData4ThenReturn( LO_NIB(READ_INSTRUCTION__BUSY_FLAG) );
    Expect_ClearCE();

    int status = LCDIntf_WaitWhileBusy();

    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, status);
}

/* ====================================================================== */
struct LCDControllerInit_7Wires : public LCDIntf_7Wires
{
    void ExpectSequence_SetOutDirectionAndPutNibble(int32_t n) {
        Expect_ClearRW();
        Expect_SetCE();
        Expect_PutData4(n);
        Expect_SetDirection_Out4();
        Expect_ClearCE();
    }

    void ExpectSequence_PutNibble(int32_t n) {
        Expect_SetCE();
        Expect_PutData4(n);
        Expect_ClearCE();
    }

    void ExpectSequence_PutNibbleAndSetInputDirection(int32_t n) {
        Expect_SetCE();
        Expect_PutData4(n);
        Expect_ClearCE();
        Expect_SetDirection_In4();
        Expect_SetRW();
    }

    void ExpectSequence_4BitRead_BusyFlag_ReadTimeout() {
        Expect_ClearRS();
        Expect_SetRW();
        Expect_SetCE();
        for (int i = 0; i < BUSY_FLAG_READS_BEFORE_GIVING_UP; ++i) 
            Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__BUSY_FLAG) );
        Expect_ClearCE();
        Expect_SetCE();
        Expect_GetData4ThenReturn( LO_NIB(READ_INSTRUCTION__BUSY_FLAG) );
        Expect_ClearCE();
    }

    void ExpectSequence_4BitRead_NoBusyFlag() {
        Expect_ClearRS();
        Expect_SetRW();
        Expect_SetCE();
        Expect_GetData4ThenReturn( HI_NIB(READ_INSTRUCTION__NO_BUSY_FLAG) );
        Expect_ClearCE();
        Expect_SetCE();
        Expect_GetData4ThenReturn( LO_NIB(READ_INSTRUCTION__NO_BUSY_FLAG) );
        Expect_ClearCE();
    }
};

TEST_GROUP_BASE(AnLCDControllerInit_7Wires, LCDControllerInit_7Wires)
{
    void setup() override {
        MockPeriphIO_Create(80);
        LCDPortSpy_ResetToDefaultState();
        LCDIntf_Init(LCD_PORT_DATA_WIDTH_4_BIT);
    }

    void teardown() override {
        LCDIntf_Deinit();
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

TEST(AnLCDControllerInit_7Wires, PreparesDisplayFor4BitInterfacing) {
    // step #1: kinda turn '8bit' interfacing at '4bit' data bus
    Expect_ClearRS();
    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(FUNCTION_SET__8BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    // step #2: now turn '4bit' interfacing
    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    // step #3: turn '4bit' again -- the datasheet insists
    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(37);

    // step #4: Display On/Off Control
    ExpectSequence_PutNibble(HI_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_PutNibbleAndSetInputDirection(
                             LO_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_4BitRead_NoBusyFlag();

    // step #5: Display Clear
    ExpectSequence_SetOutDirectionAndPutNibble(  HI_NIB(DISPLAY_CLEAR));
    ExpectSequence_PutNibbleAndSetInputDirection(LO_NIB(DISPLAY_CLEAR));
    ExpectSequence_4BitRead_NoBusyFlag();

    // step #6: Entry Mode Set
    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(ENTRY_MODE_SET__I_D_SH));
    ExpectSequence_PutNibbleAndSetInputDirection(
        LO_NIB(ENTRY_MODE_SET__I_D_SH));
    ExpectSequence_4BitRead_NoBusyFlag();


    LCDIntf_InitializeLCDController();
}

TEST(AnLCDControllerInit_7Wires, ReportsSuccessInSetupOf4BitMode) {
    Expect_ClearRS();
    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(FUNCTION_SET__8BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_PutNibble(HI_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_PutNibbleAndSetInputDirection(
                             LO_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_4BitRead_NoBusyFlag();

    ExpectSequence_SetOutDirectionAndPutNibble(  HI_NIB(DISPLAY_CLEAR));
    ExpectSequence_PutNibbleAndSetInputDirection(LO_NIB(DISPLAY_CLEAR));
    ExpectSequence_4BitRead_NoBusyFlag();

    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(ENTRY_MODE_SET__I_D_SH));
    ExpectSequence_PutNibbleAndSetInputDirection(
        LO_NIB(ENTRY_MODE_SET__I_D_SH));
    ExpectSequence_4BitRead_NoBusyFlag();


    LONGS_EQUAL(LCD_OPERATION_OK, LCDIntf_InitializeLCDController());
}

TEST(AnLCDControllerInit_7Wires, DetectsReadTimeoutOnDisplayControlInstruction)
{
    Expect_ClearRS();
    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(FUNCTION_SET__8BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_PutNibble(HI_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_PutNibbleAndSetInputDirection(
                             LO_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));


    ExpectSequence_4BitRead_BusyFlag_ReadTimeout();

    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, LCDIntf_InitializeLCDController());
}

TEST(AnLCDControllerInit_7Wires, DetectsReadTimeoutOnDisplayClearInstruction) {
    Expect_ClearRS();
    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(FUNCTION_SET__8BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_PutNibble(HI_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_PutNibbleAndSetInputDirection(
                             LO_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_4BitRead_NoBusyFlag();

    ExpectSequence_SetOutDirectionAndPutNibble(  HI_NIB(DISPLAY_CLEAR));
    ExpectSequence_PutNibbleAndSetInputDirection(LO_NIB(DISPLAY_CLEAR));

    ExpectSequence_4BitRead_BusyFlag_ReadTimeout();


    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, LCDIntf_InitializeLCDController());
}

TEST(AnLCDControllerInit_7Wires, DetectsReadTimeoutOnEntryModeSetInstruction) {
    Expect_ClearRS();
    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(FUNCTION_SET__8BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(39);

    ExpectSequence_PutNibble(HI_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectSequence_PutNibble(LO_NIB(FUNCTION_SET__4BIT_2LINE_8x11FONT));
    ExpectCall_Delay_microseconds(37);

    ExpectSequence_PutNibble(HI_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_PutNibbleAndSetInputDirection(
                             LO_NIB(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF));
    ExpectSequence_4BitRead_NoBusyFlag();

    ExpectSequence_SetOutDirectionAndPutNibble(  HI_NIB(DISPLAY_CLEAR));
    ExpectSequence_PutNibbleAndSetInputDirection(LO_NIB(DISPLAY_CLEAR));
    ExpectSequence_4BitRead_NoBusyFlag();

    ExpectSequence_SetOutDirectionAndPutNibble(
        HI_NIB(ENTRY_MODE_SET__I_D_SH));
    ExpectSequence_PutNibbleAndSetInputDirection(
        LO_NIB(ENTRY_MODE_SET__I_D_SH));


    ExpectSequence_4BitRead_BusyFlag_ReadTimeout();

    LONGS_EQUAL(LCD_OPERATION_TIMEOUT, LCDIntf_InitializeLCDController());
}

