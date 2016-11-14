#include <stdint.h>
extern "C" {
#include "LCDIntf.h"
#include "MockPeriphIO.h"
};
#include "LCDIntfMock.h"

// extern "C" int32_t LCDIntf_Init(int32_t lcdPortDataWidth) { }
// extern "C" void    LCDIntf_Deinit(void) { }
// extern "C" int32_t LCDIntf_GetPortDataWidth(void) { }
// extern "C" int32_t LCDIntf_InitializeLCDController(void) { }

extern "C" void
LCDIntf_WriteInstruction(int32_t i)
{
    MockPeriphIO_Write(LCDINTFMOCK_WRITE_INSTRUCTION_CALL, i);
}

extern "C" void
LCDIntf_WriteData(int32_t d)
{
    MockPeriphIO_Write(LCDINTFMOCK_WRITE_DATA_CALL, d);
}

extern "C" int32_t
LCDIntf_ReadData(void)
{
    return MockPeriphIO_Read(LCDINTFMOCK_READ_DATA_CALL);
}

extern "C" int32_t
LCDIntf_ReadInstruction(void)
{
    return MockPeriphIO_Read(LCDINTFMOCK_READ_INSTRUCTION_CALL);
}

extern "C" int32_t
LCDIntf_WaitWhileBusy(void)
{
    return MockPeriphIO_Read(LCDINTFMOCK_WAIT_WHILE_BUSY_CALL);
}
