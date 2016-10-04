#include "CppUTest/TestHarness.h"
#include "CppUTest/TestTestingFixture.h"
#include <stdint.h>

extern "C" {
#include "MockPeriphIO.h"
};

static void canMatchReadExpectations();
static void failsExpectingReadButGettingWrite();
static void failsExpectingWriteButGettingRead();
static void failsWhenWriteAddressesDoNotMatch();
static void failsWhenReadAddressesDoNotMatch();
static void failsWhenWriteDataDoNotMatch();
static void createTwoUnusedExpectations();
static void leaveOneUnusedWriteExpectation();
static void leaveOneUnusedReadExpectation();
static void produceOverflowOfWriteExpectations();
static void produceOverflowOfReadExpectations();

TEST_GROUP(AMockPeriphIOTemp)
{
    void setup()
    {
        MockPeriphIO_Create(2);
    }
    void teardown()
    {
        MockPeriphIO_Destroy();
    }
};

//IGNORE_TEST(AMockPeriphIOTemp, CanMatchReadExpectations) {
//    canMatchReadExpectations();
//}

//TEST(AMockPeriphIOTemp, FailsExpectingReadButGettingWrite) {
//    failsExpectingReadButGettingWrite();
//}

//TEST(AMockPeriphIOTemp, FailsExpectingWriteButGettingRead) {
//    failsExpectingWriteButGettingRead();
//}

//TEST(AMockPeriphIOTemp, FailsWhenWriteAddressesDoNotMatch) {
//    failsWhenWriteAddressesDoNotMatch();
//    FAIL("should not be reached");  // XXX moot point
//}

//TEST(AMockPeriphIOTemp, FailsWhenReadAddressesDoNotMatch) {
//    failsWhenReadAddressesDoNotMatch();
//    FAIL("should not be reached");  // XXX moot point
//}

//TEST(AMockPeriphIOTemp, FailsWhenWriteDataDoNotMatch) {
//    failsWhenWriteDataDoNotMatch();
//    FAIL("should not be reached");  // XXX moot point
//}

//TEST(AMockPeriphIOTemp, TooFewExpectations) {
//    createTwoUnusedExpectations();
//    MockPeriphIO_Verify_Complete();
//    FAIL("should not be reached");  // XXX moot point
//}

static void MockPeriphIOSetup()
{
    MockPeriphIO_Create(2);
}

static void MockPeriphIOTeardown()
{
    MockPeriphIO_Destroy();
}

TEST_GROUP(AMockPeriphIO)
{
    TestTestingFixture * fixture;
    int expectedErrors;

    void setup()
    {
        fixture = new TestTestingFixture();
        fixture->setSetup(MockPeriphIOSetup);
        fixture->setTeardown(MockPeriphIOTeardown);
        expectedErrors = 1;
    }

    void teardown()
    {
        delete fixture;
    }

    void testFailureWith(void (*method)())
    {
        fixture->setTestFunction(method);
        fixture->runAllTests();
        LONGS_EQUAL(expectedErrors, fixture->getFailureCount());
    }
};

static void reportsNotInitializedBy_Expect_Write()
{
    MockPeriphIO_Destroy();
    MockPeriphIO_Expect_Write(1, 2);
}

TEST(AMockPeriphIO, ReportsNoninitializedFailureBy_Expect_Write) {
    testFailureWith(reportsNotInitializedBy_Expect_Write);
    fixture->assertPrintContains("MockPeriphIO is not initialized");
}

static void reportsNotInitializedBy_Expect_ReadThenReturn() {
    MockPeriphIO_Destroy();
    MockPeriphIO_Expect_ReadThenReturn(1, 3);
}

TEST(AMockPeriphIO, ReportsNoninitializedFailureBy_Expect_ReadThenReturn) {
    testFailureWith(reportsNotInitializedBy_Expect_ReadThenReturn);
    fixture->assertPrintContains("MockPeriphIO is not initialized");
}

static void reportsNotInitializedBy_Verify_Complete()
{
    MockPeriphIO_Destroy();
    MockPeriphIO_Verify_Complete();
}

TEST(AMockPeriphIO, ReportsNoninitializedFailureBy_Verify_Complete) {
    testFailureWith(reportsNotInitializedBy_Verify_Complete);
    fixture->assertPrintContains("MockPeriphIO is not initialized");
}

TEST(AMockPeriphIO, ReportsNoninitializedFailureBy_Read) {
    testFailureWith(reportsNotInitializedBy_Verify_Complete);
    fixture->assertPrintContains("MockPeriphIO is not initialized");
}

static void reportsNotInitializedBy_Write()
{
    MockPeriphIO_Destroy();
    MockPeriphIO_Write(32, 0);
}

TEST(AMockPeriphIO, ReportsNoninitializedFailureBy_Write) {
    testFailureWith(reportsNotInitializedBy_Write);
    fixture->assertPrintContains("MockPeriphIO is not initialized");
}

static void failsExpectingReadButGettingWrite(void)
{
    MockPeriphIO_Expect_ReadThenReturn(1, 2);
    MockPeriphIO_Write(1, 2);
}

TEST(AMockPeriphIO, FailsExpectingReadButGettingWrite) {
    testFailureWith(failsExpectingReadButGettingWrite);
    fixture->assertPrintContains("I/O direction mismatch");
    fixture->assertPrintContains("Expected: READ");
}

static void failsExpectingWriteButGettingRead(void)
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Read(1);
}

TEST(AMockPeriphIO, FailsExpectingWriteButGettingRead) {
    testFailureWith(failsExpectingWriteButGettingRead);
    fixture->assertPrintContains("I/O direction mismatch");
    fixture->assertPrintContains("Expected: WRITE");
}
static void failsWhenWriteAddressesDoNotMatch(void)
{
    MockPeriphIO_Expect_Write(1, 3);
    MockPeriphIO_Write(2, 3);
}

TEST(AMockPeriphIO, FailsWhenWriteAddressesDoNotMatch) {
    testFailureWith(failsWhenWriteAddressesDoNotMatch);
    fixture->assertPrintContains("WRITE address mismatch");
    fixture->assertPrintContains("00000001");
    fixture->assertPrintContains("00000002");
}

static void failsWhenReadAddressesDoNotMatch()
{
    MockPeriphIO_Expect_ReadThenReturn(1, 2);
    MockPeriphIO_Read(3);
}

TEST(AMockPeriphIO, FailsWhenReadAddressesDoNotMatch) {
    testFailureWith(failsWhenReadAddressesDoNotMatch);
    fixture->assertPrintContains("READ address mismatch");
    fixture->assertPrintContains("00000001");
    fixture->assertPrintContains("00000003");
}

static void
failsWhenWriteDataDoNotMatch()
{
    MockPeriphIO_Expect_Write(1, 4);
    MockPeriphIO_Write(1, 5);
}

TEST(AMockPeriphIO, FailsWhenWriteDataDoNotMatch) {
    testFailureWith(failsWhenWriteDataDoNotMatch);
    fixture->assertPrintContains("WRITE data mismatch");
    fixture->assertPrintContains("00000004");
    fixture->assertPrintContains("00000005");
}

static void canMatchWriteExpectation()
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Write(1, 2);
}

TEST(AMockPeriphIO, CanMatchWriteExpectation) {
    expectedErrors = 0;
    testFailureWith(canMatchWriteExpectation);
    fixture->assertPrintContains("OK");
}

static void canMatchReadExpectations()
{
    MockPeriphIO_Expect_ReadThenReturn(3, 4);
    MockPeriphIO_Read(3);
}

TEST(AMockPeriphIO, CanMatchReadExpectation) {
    expectedErrors = 0;
    testFailureWith(canMatchReadExpectations);
    fixture->assertPrintContains("OK");
}

static void readOperationReturnsExpectedValue()
{
    uint32_t expectedReturnValue = 42;
    MockPeriphIO_Expect_ReadThenReturn(3, expectedReturnValue);
    UNSIGNED_LONGS_EQUAL(expectedReturnValue,
        MockPeriphIO_Read(3));
}

TEST(AMockPeriphIO, ReturnsExpectedValueOnReadOperation) {
    expectedErrors = 0;
    testFailureWith(readOperationReturnsExpectedValue);
    fixture->assertPrintContains("OK");
}

static void createTwoUnusedExpectations()
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Expect_ReadThenReturn(3, 4);
    MockPeriphIO_Verify_Complete();
}

TEST(AMockPeriphIO, FailsOnUnusedExpectations) {
    testFailureWith(createTwoUnusedExpectations);
    fixture->assertPrintContains("Too few I/O operations");
    fixture->assertPrintContains("Expected: 2");
    fixture->assertPrintContains("Actual: 0");
}

static void leaveOneUnusedWriteExpectation()
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Write(1, 2);
    MockPeriphIO_Verify_Complete();
}

TEST(AMockPeriphIO, FailsOnOneUnusedWriteExpectation) {
    testFailureWith(leaveOneUnusedWriteExpectation);
    fixture->assertPrintContains("Too few I/O operations");
    fixture->assertPrintContains("Expected: 2");
    fixture->assertPrintContains("Actual: 1");
}

static void leaveOneUnusedReadExpectation()
{
    MockPeriphIO_Expect_ReadThenReturn(3, 4);
    MockPeriphIO_Expect_ReadThenReturn(3, 4);
    MockPeriphIO_Read(3);
    MockPeriphIO_Verify_Complete();
}

TEST(AMockPeriphIO, FailsOnOneUnusedReadExpectation) {
    testFailureWith(leaveOneUnusedReadExpectation);
    fixture->assertPrintContains("Too few I/O operations");
    fixture->assertPrintContains("Expected: 2");
    fixture->assertPrintContains("Actual: 1");
}

static void produceOverflowOfWriteExpectations()
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Expect_Write(1, 2);
}

TEST(AMockPeriphIO, FailsOnOverflowOfWriteExpectations) {
    testFailureWith(produceOverflowOfWriteExpectations);
    fixture->assertPrintContains("Failed to add expectation: table is full");
}

static void produceOverflowOfReadExpectations()
{
    MockPeriphIO_Expect_ReadThenReturn(1, 2);
    MockPeriphIO_Expect_ReadThenReturn(1, 2);
    MockPeriphIO_Expect_ReadThenReturn(1, 2);
}

TEST(AMockPeriphIO, FailsOnOverflowOfReadExpectations) {
    testFailureWith(produceOverflowOfReadExpectations);
    fixture->assertPrintContains("Failed to add expectation: table is full");
}

static void requestWriteOperationWithEmptyExpectationsList()
{
    MockPeriphIO_Write(1, 2);
}

TEST(AMockPeriphIO, WriteFailsOnEmptyExpectationList) {
    testFailureWith(requestWriteOperationWithEmptyExpectationsList);
    fixture->assertPrintContains("An IO called, but expectations list"
        " has exhausted");
}

static void requestReadOperationWithEmptyExpectationsList()
{
    MockPeriphIO_Read(1);
}

TEST(AMockPeriphIO, ReadFailsOnEmptyExpectationList) {
    testFailureWith(requestReadOperationWithEmptyExpectationsList);
    fixture->assertPrintContains("An IO called, but expectations list"
        " has exhausted");
}

// XXX rename me
static void exhaustExpectationList()
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Write(1, 2);
    MockPeriphIO_Write(1, 2);
}

TEST(AMockPeriphIO, DetectsExpectationListStarvation) {
    testFailureWith(exhaustExpectationList);
    fixture->assertPrintContains("An IO called, but expectations list"
        " has exhausted");
}



static void matchMultipleExpectations()
{
    MockPeriphIO_Expect_Write(1, 2);
    MockPeriphIO_Expect_ReadThenReturn(3, 4);

    MockPeriphIO_Write(1, 2);
    UNSIGNED_LONGS_EQUAL(4, MockPeriphIO_Read(3));

    MockPeriphIO_Verify_Complete();
}

TEST(AMockPeriphIO, HandlesMultipleExpectations) {
    expectedErrors = 0;
    testFailureWith(matchMultipleExpectations);
    fixture->assertPrintContains("OK");
}

