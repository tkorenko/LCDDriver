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

#ifndef D_LCDIntf_h
#define D_LCDIntf_h

#ifdef TESTBUILD
#define BUSY_FLAG_READS_BEFORE_GIVING_UP 5
#endif

#ifndef BUSY_FLAG_READS_BEFORE_GIVING_UP
#define BUSY_FLAG_READS_BEFORE_GIVING_UP 4000
#endif

#include <stdint.h>

enum {
    FUNCTION_SET__8BIT_2LINE_8x11FONT = 0x3C,
    FUNCTION_SET__4BIT_2LINE_8x11FONT = 0x2C,
    DISPLAY_CONTROL__D_ON_C_OFF_B_OFF = 0x0C,
    DISPLAY_CLEAR = 0x01,
    ENTRY_MODE_SET__I_D_SH = 0x06,
    SET_DDRAM_ADDRESS_CMD = 0x80,
};

enum {
    LCD_OPERATION_OK = 0,
    LCD_OPERATION_TIMEOUT,
    READ_INSTRUCTION__BUSY_FLAG = 0x80,
    READ_INSTRUCTION__BUSY_FLAG_MASK =  READ_INSTRUCTION__BUSY_FLAG,
    READ_INSTRUCTION__NO_BUSY_FLAG   = ~READ_INSTRUCTION__BUSY_FLAG,
};

int32_t LCDIntf_Init(int32_t lcdPortDataWidth);
void    LCDIntf_Deinit(void);
int32_t LCDIntf_GetPortDataWidth(void);
void    LCDIntf_WriteInstruction(int32_t i);
void    LCDIntf_WriteData(int32_t d);
int32_t LCDIntf_ReadData(void);
int32_t LCDIntf_ReadInstruction(void);
int32_t LCDIntf_WaitWhileBusy(void);
int32_t LCDIntf_InitializeLCDController(void);

extern void Delay_microseconds(uint32_t microseconds);

#endif /* #ifndef D_LCDIntf_h */
