#include "CppUTest/TestHarness.h"
#include <stdint.h>
extern "C"
{
#include "LCDPort.h"
#include "MockPeriphIO.h"
};
#include "LCDPortSpy.h"

TEST_GROUP(ALCDPort_InitDeinit)
{
    void setup() override {
        LCDPortSpy_ResetToDefaultState();
    }

    void CHECK_PORT_STATE_UNDEFINED() {
        LONGS_EQUAL_TEXT(LINE_STATE_UNDEFINED, LCDPortSpy_GetRS(), "RS");
        LONGS_EQUAL_TEXT(LINE_STATE_UNDEFINED, LCDPortSpy_GetRW(), "RW");
        LONGS_EQUAL_TEXT(LINE_STATE_UNDEFINED, LCDPortSpy_GetCE(), "CE");

        LONGS_EQUAL_TEXT(LCD_PORT_DATA_DIR_UNDEFINED,
            LCDPortSpy_GetDataDirection(), "Data Direction");
    }

    void CHECK_PORT_STATE_INITIALIZED_AS(int dataDirection) {
        LONGS_EQUAL_TEXT(LINE_STATE_DEASSERTED, LCDPortSpy_GetRS(), "RS");
        LONGS_EQUAL_TEXT(LINE_STATE_ASSERTED,   LCDPortSpy_GetRW(), "RW");
        LONGS_EQUAL_TEXT(LINE_STATE_DEASSERTED, LCDPortSpy_GetCE(), "CE");

        LONGS_EQUAL_TEXT(dataDirection,
            LCDPortSpy_GetDataDirection(), "Data Direction");
    }
};

TEST(ALCDPort_InitDeinit, HasUndefinedLineStatesForNoninitializedPort) {
    CHECK_PORT_STATE_UNDEFINED();
}

TEST(ALCDPort_InitDeinit, InitializationSetsSpyLinesToKnownState) {
    LCDPort_Init(LCD_PORT_DATA_WIDTH_8_BIT);

    CHECK_PORT_STATE_INITIALIZED_AS(LCD_PORT_DATA_DIR_INPUT8);
}

TEST(ALCDPort_InitDeinit, DeinitializationResetsSpyToUndefinedState) {
    LCDPort_Init(LCD_PORT_DATA_WIDTH_8_BIT);

    LCDPort_Deinit();

    CHECK_PORT_STATE_UNDEFINED();
}

/* ====================================================================== */
class MockedLCDPort : public Utest
{
public:
    void setup() override {
        MockPeriphIO_Create(5);
    }
    void teardown() override {
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

TEST_GROUP_BASE(AMockedLCDPort, MockedLCDPort)
{
    void setup() override {
        LCDPort_Init(LCD_PORT_DATA_WIDTH_8_BIT);
        MockedLCDPort::setup();
    }
    void teardown() override {
        MockedLCDPort::teardown();
        LCDPort_Deinit();
    }
};

TEST(AMockedLCDPort, DetectsRSLineAssertionAndRecordsState) {
    MockPeriphIO_Expect_Write(LCD_RS_ADDR, LINE_STATE_ASSERTED);

    LCDPort_SetRS();
    LONGS_EQUAL(LINE_STATE_ASSERTED, LCDPortSpy_GetRS());
}

TEST(AMockedLCDPort, DetectsRSLineDeassertionAndReflectsItsFinalState) {
    MockPeriphIO_Expect_Write(LCD_RS_ADDR, LINE_STATE_ASSERTED);
    MockPeriphIO_Expect_Write(LCD_RS_ADDR, LINE_STATE_DEASSERTED);
    LCDPort_SetRS();

    LCDPort_ClearRS();
    LONGS_EQUAL(LINE_STATE_DEASSERTED, LCDPortSpy_GetRS());
}

TEST(AMockedLCDPort, DetectsRWLineDeassertionAndRecordsState) {
    MockPeriphIO_Expect_Write(LCD_RW_ADDR, LINE_STATE_DEASSERTED);

    LCDPort_ClearRW();
    LONGS_EQUAL(LINE_STATE_DEASSERTED, LCDPortSpy_GetRW());
}

TEST(AMockedLCDPort, DetectsRWLineAssertionAndRecordsItsFinalState) {
    MockPeriphIO_Expect_Write(LCD_RW_ADDR, LINE_STATE_DEASSERTED);
    MockPeriphIO_Expect_Write(LCD_RW_ADDR, LINE_STATE_ASSERTED);
    LCDPort_ClearRW();

    LCDPort_SetRW();
    LONGS_EQUAL(LINE_STATE_ASSERTED, LCDPortSpy_GetRW());
}

TEST(AMockedLCDPort, DetectsCELineAssertionAndReflectsItsState) {
    MockPeriphIO_Expect_Write(LCD_CE_ADDR, LINE_STATE_ASSERTED);

    LCDPort_SetCE();
    LONGS_EQUAL(LINE_STATE_ASSERTED, LCDPortSpy_GetCE());
}

TEST(AMockedLCDPort, DetectsCELineDeassertionAndReflectsItsFinalState) {
    MockPeriphIO_Expect_Write(LCD_CE_ADDR, LINE_STATE_ASSERTED);
    MockPeriphIO_Expect_Write(LCD_CE_ADDR, LINE_STATE_DEASSERTED);
    LCDPort_SetCE();

    LCDPort_ClearCE();
    LONGS_EQUAL(LINE_STATE_DEASSERTED, LCDPortSpy_GetCE());
}

/* ====================================================================== */
TEST_GROUP_BASE(AMockedLCDPort8Bit, MockedLCDPort)
{
    void setup() override {
        LCDPort_Init(LCD_PORT_DATA_WIDTH_8_BIT);
        MockedLCDPort::setup();
    }
    void teardown() override {
        MockedLCDPort::teardown();
        LCDPort_Deinit();
    }
};

TEST(AMockedLCDPort8Bit, SetsOutputDataDirectionAndSavesDirectionState) {
    MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION8_REG,
        LCD_PORT_DATA_DIR_OUTPUT8);

    LCDPort_SetDirection_Output8();
    LONGS_EQUAL(LCD_PORT_DATA_DIR_OUTPUT8, LCDPortSpy_GetDataDirection());
}

TEST(AMockedLCDPort8Bit, SetsInputDataDirectionAndSavesDirectionState) {
    MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION8_REG,
        LCD_PORT_DATA_DIR_OUTPUT8);
    MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION8_REG,
        LCD_PORT_DATA_DIR_INPUT8);
    LCDPort_SetDirection_Output8();

    LCDPort_SetDirection_Input8();
    LONGS_EQUAL(LCD_PORT_DATA_DIR_INPUT8, LCDPortSpy_GetDataDirection());
}

TEST(AMockedLCDPort8Bit, CapturesDataWrites) {
    MockPeriphIO_Expect_Write(LCD_DATA8_ADDR, 0x12341234);

    LCDPort_Out8(0x12341234);
}

TEST(AMockedLCDPort8Bit, InterceptsDataReads) {
    MockPeriphIO_Expect_ReadThenReturn(LCD_DATA8_ADDR, 0x10002000);

    LONGS_EQUAL(0x10002000, LCDPort_In8());
}

/* ====================================================================== */
TEST_GROUP_BASE(AMockedLCDPort4Bit, MockedLCDPort)
{
    void setup() override {
        LCDPort_Init(LCD_PORT_DATA_WIDTH_4_BIT);
        MockedLCDPort::setup();
    }
    void teardown() override {
        MockedLCDPort::teardown();
        LCDPort_Deinit();
    }
};

TEST(AMockedLCDPort4Bit, SetsOutputDataDirectionAndSavesDirectionState) {
    MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION4_REG,
        LCD_PORT_DATA_DIR_OUTPUT4);

    LCDPort_SetDirection_Output4();
    LONGS_EQUAL(LCD_PORT_DATA_DIR_OUTPUT4, LCDPortSpy_GetDataDirection());
}

TEST(AMockedLCDPort4Bit, SetsInputDataDirectionAndSavesDirectionState) {
    MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION4_REG,
        LCD_PORT_DATA_DIR_OUTPUT4);
    MockPeriphIO_Expect_Write(LCD_FAKE_DIRECTION4_REG,
        LCD_PORT_DATA_DIR_INPUT4);
    LCDPort_SetDirection_Output4();

    LCDPort_SetDirection_Input4();
    LONGS_EQUAL(LCD_PORT_DATA_DIR_INPUT4, LCDPortSpy_GetDataDirection());
}

TEST(AMockedLCDPort4Bit, CapturesDataWrites) {
    MockPeriphIO_Expect_Write(LCD_DATA4_ADDR, 0x22224444);

    LCDPort_Out4(0x22224444);
}

TEST(AMockedLCDPort4Bit, InterceptsDataReads) {
    MockPeriphIO_Expect_ReadThenReturn(LCD_DATA4_ADDR, 0x20002000);

    LONGS_EQUAL(0x20002000, LCDPort_In4());
}

