#include "CppUTest/TestHarness.h"
#include <stdint.h>
extern "C"
{
#include "LCDIntf.h"
#include "MockPeriphIO.h"
};
#include "LCDIntfMock.h"

TEST_GROUP(AnLCDMock)
{
    int32_t anyValue, expectedValue;

    void setup() override {
        MockPeriphIO_Create(5);
    }
    void teardown() override {
        MockPeriphIO_Verify_Complete();
        MockPeriphIO_Destroy();
    }
};

TEST(AnLCDMock, InterceptsWriteInstructionCalls) {
    anyValue = 0x1234;
    LCDIntfMock_Expect_WriteInstruction(anyValue);

    LCDIntf_WriteInstruction(anyValue);
}

TEST(AnLCDMock, InterceptsWriteDataCalls) {
    anyValue = 0x1238;
    LCDIntfMock_Expect_WriteData(anyValue);

    LCDIntf_WriteData(anyValue);
}

TEST(AnLCDMock, ControlsReadDataCalls) {
    expectedValue = 0x2020;
    LCDIntfMock_Expect_ReadDataThenReturn(expectedValue);

    LONGS_EQUAL(expectedValue, LCDIntf_ReadData());
}

TEST(AnLCDMock, ControlsReadInstructionCalls) {
    expectedValue = 0x2024;
    LCDIntfMock_Expect_ReadInstructionThenReturn(expectedValue);

    LONGS_EQUAL(expectedValue, LCDIntf_ReadInstruction());
}

TEST(AnLCDMock, InterceptsWaitWhileBusyCalls) {
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_TIMEOUT);
    LCDIntfMock_Expect_WaitWhileBusyThenReturn(LCDINTFMOCK_WAIT_COMPLETE);

    LONGS_EQUAL(LCDINTFMOCK_WAIT_TIMEOUT,  LCDIntf_WaitWhileBusy());
    LONGS_EQUAL(LCDINTFMOCK_WAIT_COMPLETE, LCDIntf_WaitWhileBusy());
}

