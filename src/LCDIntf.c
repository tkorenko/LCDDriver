/*
 * Copyright (c) 2016, Taras Korenko
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include "LCDIntf.h"
#include "LCDPort.h"

#define HI_NIBBLE(byte) ((byte >> 4) & 0x0F)
#define LO_NIBBLE(byte) (byte & 0x0F)

static void writeInstruction_8BitIntf(int32_t instr);
static void writeInstruction_4BitIntf(int32_t instr);
static void writeData_8BitIntf(int32_t data);
static void writeData_4BitIntf(int32_t data);
static int32_t readInstruction_8BitIntf(void);
static int32_t readInstruction_4BitIntf(void);
static int32_t readData_8BitIntf(void);
static int32_t readData_4BitIntf(void);
static int32_t waitWhileBusy_8BitIntf(void);
static int32_t waitWhileBusy_4BitIntf(void);
static int32_t setup8BitMode(void);
static int32_t setup4BitMode(void);
static void select8BitInterfaceImplementation(void);
static void select4BitInterfaceImplementation(void);

typedef void (*pFvi_t)(int32_t);
typedef int32_t (*pFiv_t)(void);

static void stubFuncVI(int32_t x) {
}
static int32_t stubFuncIV(void) {
    return -1;
}

static pFvi_t writeInstruction = stubFuncVI;
static pFvi_t writeData = stubFuncVI;
static pFiv_t readInstruction = stubFuncIV;
static pFiv_t readData = stubFuncIV;
static pFiv_t waitWhileBusy = stubFuncIV;
static pFiv_t initializeLCDController = stubFuncIV;

static int8_t lcdPortDataWidth = LCD_PORT_DATA_WIDTH_UNDEFINED;

/* ==== Public Interface ================================================ */

int32_t
LCDIntf_Init(int32_t dataWidth)
{
    lcdPortDataWidth = dataWidth;

    LCDPort_Init(lcdPortDataWidth);

    if (LCD_PORT_DATA_WIDTH_8_BIT == lcdPortDataWidth) {
        select8BitInterfaceImplementation();
    } else if (LCD_PORT_DATA_WIDTH_4_BIT == lcdPortDataWidth) {
        select4BitInterfaceImplementation();
    }

	return 0;
}

void
LCDIntf_Deinit(void)
{
    LCDPort_Deinit();

    lcdPortDataWidth = LCD_PORT_DATA_WIDTH_UNDEFINED;
}

int32_t
LCDIntf_GetPortDataWidth(void)
{
    return lcdPortDataWidth;
}
void
LCDIntf_WriteInstruction(int32_t instr)
{
    writeInstruction(instr);
}

void
LCDIntf_WriteData(int32_t data)
{
    writeData(data);
}

int32_t
LCDIntf_ReadData(void)
{
    return readData();
}

int32_t
LCDIntf_ReadInstruction(void)
{
    return readInstruction();
}

int32_t
LCDIntf_WaitWhileBusy(void)
{
    return (waitWhileBusy()) ? LCD_OPERATION_TIMEOUT : LCD_OPERATION_OK;
}

int32_t
LCDIntf_InitializeLCDController(void)
{
    return initializeLCDController();
}

/* ==== Private Implementation ========================================== */

static void
select8BitInterfaceImplementation(void)
{
    writeInstruction = writeInstruction_8BitIntf;
    writeData = writeData_8BitIntf;
    readInstruction = readInstruction_8BitIntf;
    readData = readData_8BitIntf;
    waitWhileBusy = waitWhileBusy_8BitIntf;
    initializeLCDController = setup8BitMode;
}

static void
select4BitInterfaceImplementation(void)
{
    writeInstruction = writeInstruction_4BitIntf;
    writeData = writeData_4BitIntf;
    readInstruction = readInstruction_4BitIntf;
    readData = readData_4BitIntf;
    waitWhileBusy = waitWhileBusy_4BitIntf;
    initializeLCDController = setup4BitMode;
}

static void
writeInstruction_8BitIntf(int32_t instr)
{
    LCDPort_ClearRS();
    LCDPort_ClearRW();
    LCDPort_SetCE();
    LCDPort_Out8(instr);
    LCDPort_SetDirection_Output8();
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input8();
    LCDPort_SetRW();
}

static void
writeInstruction_4BitIntf(int32_t instr)
{
    LCDPort_ClearRS();
    LCDPort_ClearRW();
    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(instr) );
    LCDPort_SetDirection_Output4();
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(instr) );
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input4();
    LCDPort_SetRW();
}

static void
writeData_8BitIntf(int32_t data)
{
    LCDPort_SetRS();
    LCDPort_ClearRW();
    LCDPort_SetCE();
    LCDPort_Out8(data);
    LCDPort_SetDirection_Output8();
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input8();
    LCDPort_SetRW();
}
static void
writeData_4BitIntf(int32_t data)
{
    LCDPort_SetRS();
    LCDPort_ClearRW();
    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(data) );
    LCDPort_SetDirection_Output4();
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(data) );
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input4();
    LCDPort_SetRW();
}

static int32_t
readData_8BitIntf(void)
{
    int32_t rs;

    LCDPort_SetRS();
    LCDPort_SetRW();
    LCDPort_SetCE();
    rs = LCDPort_In8();
    LCDPort_ClearCE();

    return rs;
}

static int32_t
readData_4BitIntf(void)
{
    int32_t rs;

    LCDPort_SetRS();
    LCDPort_SetRW();
    LCDPort_SetCE();
    rs =  LO_NIBBLE( LCDPort_In4() ) << 4;
    LCDPort_ClearCE();
    LCDPort_SetCE();
    rs |= LO_NIBBLE( LCDPort_In4() );
    LCDPort_ClearCE();

    return rs;
}

static int32_t
readInstruction_8BitIntf(void)
{
    int rs;

    LCDPort_ClearRS();
    LCDPort_SetRW();
    LCDPort_SetCE();
    rs = LCDPort_In8();
    LCDPort_ClearCE();

    return rs;
}

static int32_t
readInstruction_4BitIntf(void)
{
    int rs;

    LCDPort_ClearRS();
    LCDPort_SetRW();
    LCDPort_SetCE();
    rs =  LO_NIBBLE( LCDPort_In4() ) << 4;
    LCDPort_ClearCE();
    LCDPort_SetCE();
    rs |= LO_NIBBLE( LCDPort_In4() );
    LCDPort_ClearCE();

    return rs;
}

#define WRITE_8BIT_INSTRUCTION_SEQUENCE(cmd)                                \
    do {                                                                    \
        LCDPort_ClearRS();                                                  \
        LCDPort_ClearRW();                                                  \
        LCDPort_SetCE();                                                    \
        LCDPort_Out8( (cmd) );                                              \
        LCDPort_SetDirection_Output8();                                     \
        LCDPort_ClearCE();                                                  \
        LCDPort_SetDirection_Input8();                                      \
        LCDPort_SetRW();                                                    \
    } while (0)

static int32_t
setup8BitMode(void)
{
    int32_t rs;

    WRITE_8BIT_INSTRUCTION_SEQUENCE(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    Delay_microseconds(39);

    WRITE_8BIT_INSTRUCTION_SEQUENCE(FUNCTION_SET__8BIT_2LINE_8x11FONT);
    Delay_microseconds(37);

    WRITE_8BIT_INSTRUCTION_SEQUENCE(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF);
    if (LCD_OPERATION_OK != (rs = LCDIntf_WaitWhileBusy()))
        goto out;

    WRITE_8BIT_INSTRUCTION_SEQUENCE(DISPLAY_CLEAR);
    if (LCD_OPERATION_OK != (rs = LCDIntf_WaitWhileBusy()))
        goto out;

    WRITE_8BIT_INSTRUCTION_SEQUENCE(ENTRY_MODE_SET__I_D_SH);
    rs = LCDIntf_WaitWhileBusy();

out:
    return rs;
}

static int32_t
setup4BitMode(void)
{
    int32_t rs = LCD_OPERATION_OK;

    LCDPort_ClearRS();
    LCDPort_ClearRW();

    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(FUNCTION_SET__8BIT_2LINE_8x11FONT) );
    LCDPort_SetDirection_Output4();
    LCDPort_ClearCE();
    Delay_microseconds(39);

    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(FUNCTION_SET__4BIT_2LINE_8x11FONT) );
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(FUNCTION_SET__4BIT_2LINE_8x11FONT) );
    LCDPort_ClearCE();
    Delay_microseconds(39);

    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(FUNCTION_SET__4BIT_2LINE_8x11FONT) );
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(FUNCTION_SET__4BIT_2LINE_8x11FONT) );
    LCDPort_ClearCE();
    Delay_microseconds(37);

    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF) );
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(DISPLAY_CONTROL__D_ON_C_OFF_B_OFF) );
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input4();
    LCDPort_SetRW();

    if (LCD_OPERATION_OK != (rs = LCDIntf_WaitWhileBusy()))
        goto out;

    LCDPort_ClearRW();
    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(DISPLAY_CLEAR) );
    LCDPort_SetDirection_Output4();
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(DISPLAY_CLEAR) );
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input4();
    LCDPort_SetRW();

    if (LCD_OPERATION_OK != (rs = LCDIntf_WaitWhileBusy()))
        goto out;

    LCDPort_ClearRW();
    LCDPort_SetCE();
    LCDPort_Out4( HI_NIBBLE(ENTRY_MODE_SET__I_D_SH) );
    LCDPort_SetDirection_Output4();
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_Out4( LO_NIBBLE(ENTRY_MODE_SET__I_D_SH) );
    LCDPort_ClearCE();
    LCDPort_SetDirection_Input4();
    LCDPort_SetRW();
    rs = LCDIntf_WaitWhileBusy();

out:
    return rs;
}

static int32_t
waitWhileBusy_8BitIntf(void)
{
    int32_t rs, busyFlagReads;

    LCDPort_ClearRS();
    LCDPort_SetRW();
    LCDPort_SetCE();
    for (rs = 1, busyFlagReads = 0;
            rs && (busyFlagReads < BUSY_FLAG_READS_BEFORE_GIVING_UP);
            ++busyFlagReads) {
        rs = LCDPort_In8() & READ_INSTRUCTION__BUSY_FLAG_MASK;
    }
    LCDPort_ClearCE();

    return rs;
}

static int32_t
waitWhileBusy_4BitIntf(void)
{
    int32_t rs, busyFlagReads;

    LCDPort_ClearRS();
    LCDPort_SetRW();
    LCDPort_SetCE();
    for (rs = 1, busyFlagReads = 0;
            rs && (busyFlagReads < BUSY_FLAG_READS_BEFORE_GIVING_UP);
            ++busyFlagReads) {
        // XXX explain better:
        rs = (LCDPort_In4() << 4) & READ_INSTRUCTION__BUSY_FLAG_MASK;
    }
    LCDPort_ClearCE();
    LCDPort_SetCE();
    LCDPort_In4();
    LCDPort_ClearCE();

    return rs;
}

