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

#include "CppUTest/TestHarness_c.h"
#include "MockPeriphIO.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    OP_DIRECTION_UNSPECIFIED,
    OP_DIRECTION_READ,
    OP_DIRECTION_WRITE
} OP_Direction;

typedef struct IOOperationDescr
{
    int32_t     direction;
    uint32_t    ioAddress;
    uint32_t    ioValue;
} IOOperationDescr;

static uint32_t filledInExpectations = 0;
static uint32_t verifiedExpectations = 0;
static uint32_t maxQtyOfExpectations = 0;
static int suppressFailureReport = 0;

static IOOperationDescr actualOp;
static IOOperationDescr * pExpectedOps = 0;

static void fail(const char * msg);
static void failWhenNotInitialized(void);
static void failWhenOperationDirectionDoesNotMatch(void);
static const char * const nameOperation(void);
static void failWhenOperationAddressDoesNotMatch(void);
static void failWhenOperationDataDoesNotMatch(void);
static void failWhenNotAllExpectationsWereUsed(void);
static void failWhenTooManyExpectations(void);
static void failWhenRunOutOfExpectaions(void);
static void setOpDescr(IOOperationDescr * pOp,
    int direction, uint32_t addr, uint32_t value);

/* ====================================================================== */

#define MSG_SZ 256
static const char * const report_MockPeriphIOisNotInitialized =
    "MockPeriphIO is not initialized";
static const char * const report_TooManyExpectations =
    "MockPeriphIO: Failed to add expectation: table is full";
static const char * const report_NoMoreExpectationsLeft =
    "MockPeriphIO: An IO called, but expectations list has exhausted";
static const char * const fmtIODirectonMismatch_ExpectRead =
    "Act [%2d/%2d]: I/O direction mismatch\n"
    "\t...Expected: READ  <%08X>\n"
    "\t.....Actual: WRITE <%08X>";
static const char * const fmtIODirectonMismatch_ExpectWrite =
    "Act [%2d/%2d]: I/O direction mismatch\n"
    "\t...Expected: WRITE <%08X>\n"
    "\t.....Actual: READ  <%08X>";
static const char * const fmtIOOperation_AddressMismatch =
    "Act [%2d/%2d]: %5s address mismatch\n"
    "\t...Expected: <%08X>\n"
    "\t.....Actual: <%08X>";
static const char * const fmtWriteOperation_ValueMismatch =
    "Act [%2d/%2d]: WRITE data mismatch\n"
    "\t...Expected: % 11d (%08X)\n"
    "\t.....Actual: % 11d (%08X)";
static const char * const fmtNotAllExpectationsUsed =
    "Too few I/O operations were done\n"
    "\t...Expected: %d\n"
    "\t.....Actual: %d";

static void
fail(const char * msg)
{
    if (suppressFailureReport) {
        FAIL_C();
    } else {
        suppressFailureReport = 1;
        FAIL_TEXT_C(msg);
    }
}

static void
failWhenNotInitialized(void)
{
    if (0 == pExpectedOps)
        fail(report_MockPeriphIOisNotInitialized);
}

static void
failWhenOperationDirectionDoesNotMatch(void)
{
    char msg[MSG_SZ];
    IOOperationDescr * pExpectedOp = &pExpectedOps[verifiedExpectations];

    if (pExpectedOp->direction == actualOp.direction)
        return;

    if (OP_DIRECTION_READ == pExpectedOp->direction) {
        snprintf(msg, MSG_SZ, fmtIODirectonMismatch_ExpectRead,
            (verifiedExpectations + 1), filledInExpectations,
            pExpectedOp->ioAddress, actualOp.ioAddress);
    } else if (OP_DIRECTION_WRITE == pExpectedOp->direction) {
        snprintf(msg, MSG_SZ, fmtIODirectonMismatch_ExpectWrite,
            (verifiedExpectations + 1), filledInExpectations,
            pExpectedOp->ioAddress, actualOp.ioAddress);
    } else {
        snprintf(msg, MSG_SZ, "unexpected operation direction");
    }

    fail(msg);
}

static const char * const
nameOperation(void)
{
    int direction = pExpectedOps[verifiedExpectations].direction;

    if (OP_DIRECTION_WRITE == direction)
        return "WRITE";
    if (OP_DIRECTION_READ  == direction)
        return "READ";
    return "UNKNOWN";
}

static void
failWhenOperationAddressDoesNotMatch(void)
{
    char msg[MSG_SZ];

    if (pExpectedOps[verifiedExpectations].ioAddress == actualOp.ioAddress)
        return;

    snprintf(msg, MSG_SZ, fmtIOOperation_AddressMismatch,
        (verifiedExpectations + 1), filledInExpectations,
        nameOperation(),
        pExpectedOps[verifiedExpectations].ioAddress, actualOp.ioAddress);
    fail(msg);
}

static void
failWhenOperationDataDoesNotMatch(void)
{
    char msg[MSG_SZ];
    IOOperationDescr * pExpectedOp = &pExpectedOps[verifiedExpectations];

    if (pExpectedOp->ioValue == actualOp.ioValue)
        return;

    snprintf(msg, MSG_SZ, fmtWriteOperation_ValueMismatch,
        (verifiedExpectations + 1), filledInExpectations,
        pExpectedOp->ioValue, pExpectedOp->ioValue, // two views: DEC and HEX
        actualOp.ioValue, actualOp.ioValue);
    fail(msg);
}

static void
failWhenNotAllExpectationsWereUsed(void)
{
    char msg[MSG_SZ];

    if (verifiedExpectations == filledInExpectations)
        return;

    snprintf(msg, MSG_SZ, fmtNotAllExpectationsUsed,
        filledInExpectations, verifiedExpectations);
    fail(msg);
}

static void
failWhenTooManyExpectations(void)
{
    char msg[MSG_SZ];

    if (filledInExpectations < maxQtyOfExpectations)
        return;

    snprintf(msg, MSG_SZ, report_TooManyExpectations);
    fail(msg);
}

static void
failWhenRunOutOfExpectaions(void)
{
    char msg[MSG_SZ];

    if (verifiedExpectations < filledInExpectations)
        return;

    snprintf(msg, MSG_SZ, report_NoMoreExpectationsLeft);
    fail(msg);
}

static void
setOpDescr(IOOperationDescr * pOp, int direction, uint32_t addr,
        uint32_t value)
{
    if (0 == pOp) return;

    pOp->direction = direction;
    pOp->ioAddress = addr;
    pOp->ioValue   = value;
}

/* ====================================================================== */

void
MockPeriphIO_Create(int maxExpectations)
{
    maxQtyOfExpectations = maxExpectations;
    pExpectedOps = calloc(maxQtyOfExpectations,
                            sizeof(IOOperationDescr));
    setOpDescr(&actualOp, OP_DIRECTION_UNSPECIFIED, 0, 0);
    filledInExpectations = 0;
    verifiedExpectations = 0;
    suppressFailureReport = 0;
}

void
MockPeriphIO_Destroy(void)
{
    if (0 == pExpectedOps)
        return;

    free(pExpectedOps);
    pExpectedOps = 0;
}

void
MockPeriphIO_Expect_Write(uint32_t regId, uint32_t value)
{
    failWhenNotInitialized();
    failWhenTooManyExpectations();

    setOpDescr(&pExpectedOps[filledInExpectations],
        OP_DIRECTION_WRITE, regId, value);

    ++filledInExpectations;
}

void
MockPeriphIO_Expect_ReadThenReturn(uint32_t regId, uint32_t value)
{
    failWhenNotInitialized();
    failWhenTooManyExpectations();

    setOpDescr(&pExpectedOps[filledInExpectations],
        OP_DIRECTION_READ, regId, value);

    ++filledInExpectations;
}

void
MockPeriphIO_Verify_Complete(void)
{
    failWhenNotInitialized();
    failWhenNotAllExpectationsWereUsed();
}

uint32_t
MockPeriphIO_Read(uint32_t regId)
{
    uint32_t value;

    failWhenNotInitialized();
    failWhenRunOutOfExpectaions();

    setOpDescr(&actualOp, OP_DIRECTION_READ, regId, 0);

    failWhenOperationDirectionDoesNotMatch();
    failWhenOperationAddressDoesNotMatch();

    value = pExpectedOps[verifiedExpectations].ioValue;
    ++verifiedExpectations;

    return value;
}

void
MockPeriphIO_Write(uint32_t regId, uint32_t value)
{
    failWhenNotInitialized();
    failWhenRunOutOfExpectaions();

    setOpDescr(&actualOp, OP_DIRECTION_WRITE, regId, value);

    failWhenOperationDirectionDoesNotMatch();
    failWhenOperationAddressDoesNotMatch();
    failWhenOperationDataDoesNotMatch();
    ++verifiedExpectations;
}

