#include "CppUTest/TestHarness.h"
#include <stdint.h>
extern "C"
{
#include "LCDDriver.h"
#include "LCDIntf.h"
#include "MockPeriphIO.h"
};
#include "LCDIntfMock.h"

struct LCDDriver : public Utest
{
    void setup() override {
        MockPeriphIO_Create(25);
    }
    void teardown() override {
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

/* ====================================================================== */
TEST_GROUP_BASE(AnLCDDriver_Clear, LCDDriver)
{
};

TEST(AnLCDDriver_Clear, SendsCorrectCommandSequence) {
    LCDIntfMock_Expect_WriteInstruction(DISPLAY_CLEAR);
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_COMPLETE);

    int32_t status = LCDDriver_Clear();

    LONGS_EQUAL(LCDINTFMOCK_WAIT_COMPLETE, status);
}

TEST(AnLCDDriver_Clear, ReportsOperationStatus) {
    LCDIntfMock_Expect_WriteInstruction(DISPLAY_CLEAR);
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_TIMEOUT);

    LONGS_EQUAL(LCDINTFMOCK_WAIT_TIMEOUT, LCDDriver_Clear());
}

/* ====================================================================== */
TEST_GROUP_BASE(AnLCDDriver_GotoXY, LCDDriver)
{
    int16_t screenWidth, screenHeight;

    void setup() override {
        LCDDriver::setup();
        screenWidth  = 20;
        screenHeight = 4;
        LCDDriver_SetupScreenDimensions(screenWidth, screenHeight);
    }
    void teardown() override {
        LCDDriver::teardown();
    }
    void Expect_Command_Sequence(int32_t lcdWriteInstruction) {
        LCDIntfMock_Expect_WriteInstruction(lcdWriteInstruction);
        LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_COMPLETE);
    }
};

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForTopLeftPosition) {
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);

    LCDDriver_GotoXY(0, 0);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForTopRightPosition) {
    int16_t x = screenWidth - 1, y = 0;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | x);

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForLeftmostPositionOfSecondRow) {
    int16_t x = 0, y = 1;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | 0x40);

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForRightmostPositionOfSecondRow) {
    int16_t x = (screenWidth - 1), y = 1;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | (0x40 + x));

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForLeftmostPositionOfThirdRow) {
    int16_t x = 0, y = 2;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | screenWidth);

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForRightmostPositionOfThirdRow) {
    int16_t x = (screenWidth - 1), y = 2;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | (screenWidth + x));

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForLeftmostPositionOfFourhRow) {
    int16_t x = 0, y = 3;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | (0x40 + screenWidth + x));

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, SetsDDRAMAddrForRightmostPositionOfFourhRow) {
    int16_t x = (screenWidth - 1), y = 3;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | (0x40 + screenWidth + x));

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, HandlesOtherScreenSizes_RightmostBottomPosition) {
    screenWidth  = 16;
    screenHeight = 4;
    LCDDriver_SetupScreenDimensions(screenWidth, screenHeight);

    int16_t x = (screenWidth - 1), y = 3;
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD | (0x40 + screenWidth + x));

    LCDDriver_GotoXY(x, y);
}

TEST(AnLCDDriver_GotoXY, IgnoresNegativeXValues) {
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);

    LCDDriver_GotoXY(-1, 0);
    LCDDriver_GotoXY(-2, 0);
}

TEST(AnLCDDriver_GotoXY, IgnoresNegativeYValues) {
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);

    LCDDriver_GotoXY(0, -1);
    LCDDriver_GotoXY(0, -2);
}

TEST(AnLCDDriver_GotoXY, IgnoresXValuePastTheScreenWidth) {
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);

    LCDDriver_GotoXY(screenWidth,     0);
    LCDDriver_GotoXY(screenWidth + 1, 0);
}

// XXX consider better test name
TEST(AnLCDDriver_GotoXY, IgnoresYValuePastTheScreenHeight) {
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);
    Expect_Command_Sequence(SET_DDRAM_ADDRESS_CMD);

    LCDDriver_GotoXY(0, screenHeight);
    LCDDriver_GotoXY(0, screenHeight + 1);
}

TEST(AnLCDDriver_GotoXY, DetectsCommandTimeout) {
    LCDIntfMock_Expect_WriteInstruction(SET_DDRAM_ADDRESS_CMD);
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_TIMEOUT);

    int32_t status = LCDDriver_GotoXY(0, 0);

    LONGS_EQUAL(LCDINTFMOCK_WAIT_TIMEOUT, status);
}

/* ====================================================================== */
struct LCDDriver_PutX : public LCDDriver
{
    void Expect_Data_Sequence(int32_t ch,
            int32_t waitStatus = LCDINTFMOCK_WAIT_COMPLETE) {
        LCDIntfMock_Expect_WriteData(ch);
        LCDIntfMock_Expect_WaitWhileBusyThenReturn(waitStatus);
    }
};

TEST_GROUP_BASE(AnLCDDriver_Putc, LCDDriver_PutX)
{
};

TEST(AnLCDDriver_Putc, SendsCorrectDataSequence) {
    Expect_Data_Sequence('A');

    LCDDriver_Putc('A');
}

// XXX consider better test name
TEST(AnLCDDriver_Putc, ReturnsCommandStatus) {
    Expect_Data_Sequence('B');

    int32_t status = LCDDriver_Putc('B');

    LONGS_EQUAL(LCDINTFMOCK_WAIT_COMPLETE, status);
}

TEST(AnLCDDriver_Putc, DetectsCommandTimeout) {
    Expect_Data_Sequence('T', LCDINTFMOCK_WAIT_TIMEOUT);

    int32_t status = LCDDriver_Putc('T');

    LONGS_EQUAL(LCDINTFMOCK_WAIT_TIMEOUT, status);
}

TEST(AnLCDDriver_Putc, ReplacesNegativeCharCodeBySpace) {
    Expect_Data_Sequence(' ');

    LCDDriver_Putc(-1);
}

TEST(AnLCDDriver_Putc, ReplacesLargeCharCodesBySpace) {
    Expect_Data_Sequence(' ');
    Expect_Data_Sequence(' ');

    LCDDriver_Putc(256);
    LCDDriver_Putc(12345);
}

/* ====================================================================== */
TEST_GROUP_BASE(AnLCDDriver_Puts, LCDDriver_PutX)
{
    int16_t screenWidth, screenHeight;

    void setup() override {
        LCDDriver::setup();
        screenWidth  = 20;
        screenHeight = 4;
        LCDDriver_SetupScreenDimensions(screenWidth, screenHeight);
    }
};

TEST(AnLCDDriver_Puts, IsNullPointerTolerant) {
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_COMPLETE);

    LCDDriver_Puts(NULL);
}

TEST(AnLCDDriver_Puts, HandlesEmptyStrings) {
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_COMPLETE);

    LCDDriver_Puts((int8_t*)"");
}

TEST(AnLCDDriver_Puts, OutputsOneCharacter) {
    Expect_Data_Sequence('S');

    LCDDriver_Puts((int8_t*)"S");
}

TEST(AnLCDDriver_Puts, DetectsWriteDataTimeout) {
    Expect_Data_Sequence('S', LCDINTFMOCK_WAIT_TIMEOUT);

    int32_t status = LCDDriver_Puts((int8_t*)"S");

    LONGS_EQUAL(LCDINTFMOCK_WAIT_TIMEOUT, status);
}

TEST(AnLCDDriver_Puts, OutputsTwoChars) {
    Expect_Data_Sequence('S');
    Expect_Data_Sequence('t');

    LCDDriver_Puts((int8_t*)"St");
}

TEST(AnLCDDriver_Puts, OutputsSeveralChars) {
    Expect_Data_Sequence('S');
    Expect_Data_Sequence('t');
    Expect_Data_Sequence('r');
    Expect_Data_Sequence('i');
    Expect_Data_Sequence('n');
    Expect_Data_Sequence('g');

    LCDDriver_Puts((int8_t*)"String");
}


TEST(AnLCDDriver_Puts, OutputsNoMoreThanScreenWidthChars) {
    screenWidth  = 4;
    LCDDriver_SetupScreenDimensions(screenWidth, screenHeight);

    Expect_Data_Sequence('S');
    Expect_Data_Sequence('t');
    Expect_Data_Sequence('r');
    Expect_Data_Sequence('i');

    LCDDriver_Puts((int8_t*)"String");
}

TEST(AnLCDDriver_Puts, ReportsLastCommandStatus) {
    // Current driver implementation prefers simplicity to effectiveness,
    // thus we won't break operation at (first) timeout.
    Expect_Data_Sequence('a', LCDINTFMOCK_WAIT_TIMEOUT);
    Expect_Data_Sequence('b', LCDINTFMOCK_WAIT_TIMEOUT);
    Expect_Data_Sequence('c');

    int32_t status = LCDDriver_Puts((int8_t*)"abc");

    LONGS_EQUAL(LCDINTFMOCK_WAIT_COMPLETE, status);
}

