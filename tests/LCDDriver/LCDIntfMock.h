#ifndef D_LCDIntfMock_h
#define D_LCDIntfMock_h

#include <stdint.h>
extern "C" {
#include "MockPeriphIO.h"
};

enum {
    LCDINTFMOCK_WRITE_INSTRUCTION_CALL = 1,
    LCDINTFMOCK_WRITE_DATA_CALL,
    LCDINTFMOCK_READ_INSTRUCTION_CALL,
    LCDINTFMOCK_READ_DATA_CALL,
    LCDINTFMOCK_WAIT_WHILE_BUSY_CALL,
    LCDINTFMOCK_WAIT_COMPLETE,
    LCDINTFMOCK_WAIT_TIMEOUT,
};

inline void
LCDIntfMock_Expect_WriteInstruction(int32_t i)
{
    MockPeriphIO_Expect_Write(LCDINTFMOCK_WRITE_INSTRUCTION_CALL, i);
}

inline void
LCDIntfMock_Expect_WriteData(int32_t d)
{
    MockPeriphIO_Expect_Write(LCDINTFMOCK_WRITE_DATA_CALL, d);
}

inline void
LCDIntfMock_Expect_ReadDataThenReturn(int32_t retVal)
{
    MockPeriphIO_Expect_ReadThenReturn(LCDINTFMOCK_READ_DATA_CALL, retVal);
}

inline void
LCDIntfMock_Expect_ReadInstructionThenReturn(int32_t retVal)
{
    MockPeriphIO_Expect_ReadThenReturn(LCDINTFMOCK_READ_INSTRUCTION_CALL,
        retVal);
}

inline void
LCDIntfMock_Expect_WaitWhileBusyThenReturn(int32_t retVal)
{
    MockPeriphIO_Expect_ReadThenReturn(LCDINTFMOCK_WAIT_WHILE_BUSY_CALL,
        retVal);
}

#endif /* #ifndef D_LCDIntfMock_h */
