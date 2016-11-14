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
#include "LCDDriver.h"
#include "LCDIntf.h"

static int16_t screenWidth  = 8;
static int16_t screenHeight = 1;

int32_t
LCDDriver_Clear(void)
{
    LCDIntf_WriteInstruction(DISPLAY_CLEAR);

    return LCDIntf_WaitWhileBusy();
}

void
LCDDriver_SetupScreenDimensions(int16_t width, int16_t height)
{
    screenWidth  = width;
    screenHeight = height;
}

static void
resetInvalidValuesOfCoordinates(int16_t * pX, int16_t * pY)
{
    if ((*pX < 0) || (*pX >= screenWidth))
        *pX = 0;
    if ((*pY < 0) || (*pY >= screenHeight))
        *pY = 0;
}

enum {
    DDRAM_2ND_LINE_ADDR = 0x40,
    DDRAM_ADDR_MASK = 0x7F,
};

int32_t
LCDDriver_GotoXY(int16_t x, int16_t y)
{
    uint32_t addr;

    resetInvalidValuesOfCoordinates(&x, &y);

    addr =  x + DDRAM_2ND_LINE_ADDR * (y & 0x01) + screenWidth * (y >> 1);
    addr &= DDRAM_ADDR_MASK;

    LCDIntf_WriteInstruction(SET_DDRAM_ADDRESS_CMD | addr);

    return LCDIntf_WaitWhileBusy();
}

static void
resetInvalidCharCodeToSafeDefault(int32_t * pCh)
{
    if ((*pCh < 0) || (*pCh > 255))
        *pCh = ' ';
}

int32_t
LCDDriver_Putc(int32_t ch)
{
    resetInvalidCharCodeToSafeDefault(&ch);

    LCDIntf_WriteData(ch);

    return LCDIntf_WaitWhileBusy();
}

int32_t
LCDDriver_Puts(int8_t * str)
{
    int32_t rs = -1;
    uint32_t i = 0;

    if ((str == 0) || (*str == '\0'))
        return LCDIntf_WaitWhileBusy();

    for (i = 0; str[i] && i < screenWidth; ++i)
        rs = LCDDriver_Putc(str[i]);

    return rs;
}

