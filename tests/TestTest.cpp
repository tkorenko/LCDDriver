#include "CppUTest/TestHarness.h"


TEST_GROUP(AGroup)
{
};

TEST(AGroup, FailsIntentionally) {
    FAIL("cool");
}
