/* ==========================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#include "unity.h"
#include <string.h>
#include <stdint.h>

/* Support for Meta Test Rig */
#define TEST_CASE(...)

// Dividing by these constants produces +/- infinity.
// The rationale is given in UnityAssertFloatIsInf's body.
#ifndef UNITY_EXCLUDE_FLOAT
static const UNITY_FLOAT f_zero = 0.0f;
#endif

#ifndef UNITY_EXCLUDE_DOUBLE
static const UNITY_DOUBLE d_zero = 0.0;
#endif

#define EXPECT_ABORT_BEGIN \
    startPutcharSpy();     \
    if (TEST_PROTECT())    \
    {

#define VERIFY_FAILS_END                                                       \
    }                                                                          \
    endPutcharSpy(); /* start/end Spy to suppress output of failure message */ \
    Unity.CurrentTestFailed = (Unity.CurrentTestFailed == 1) ? 0 : 1;          \
    if (Unity.CurrentTestFailed == 1) {                                        \
      SetToOneMeanWeAlreadyCheckedThisGuy = 1;                                 \
      UnityPrintNumberUnsigned(Unity.CurrentTestLineNumber);                   \
      UNITY_OUTPUT_CHAR(':');                                                  \
      UnityPrint(Unity.CurrentTestName);                                       \
      UnityPrint(":FAIL: [[[[ Test Should Have Failed But Did Not ]]]]");      \
      UNITY_OUTPUT_CHAR('\n');                                                 \
    }

#define VERIFY_IGNORES_END                                                     \
    }                                                                          \
    endPutcharSpy(); /* start/end Spy to suppress output of ignore message */  \
    Unity.CurrentTestFailed = (Unity.CurrentTestIgnored == 1) ? 0 : 1;         \
    Unity.CurrentTestIgnored = 0;                                              \
    if (Unity.CurrentTestFailed == 1) {                                        \
      SetToOneMeanWeAlreadyCheckedThisGuy = 1;                                 \
      UnityPrintNumberUnsigned(Unity.CurrentTestLineNumber);                   \
      UNITY_OUTPUT_CHAR(':');                                                  \
      UnityPrint(Unity.CurrentTestName);                                       \
      UnityPrint(":FAIL: [[[[ Test Should Have Ignored But Did Not ]]]]");     \
      UNITY_OUTPUT_CHAR('\n');                                                 \
    }

#define VERIFY_INCONCLUSIVE_END                                                     \
    }                                                                               \
    endPutcharSpy(); /* start/end Spy to suppress output of inconclusive message */ \
    Unity.CurrentTestFailed = (Unity.CurrentTestInconclusive == 1) ? 0 : 1;         \
    Unity.CurrentTestInconclusive = 0;                                              \
    if (Unity.CurrentTestFailed == 1) {                                             \
      SetToOneMeanWeAlreadyCheckedThisGuy = 1;                                      \
      UnityPrintNumberUnsigned(Unity.CurrentTestLineNumber);                        \
      UNITY_OUTPUT_CHAR(':');                                                       \
      UnityPrint(Unity.CurrentTestName);                                            \
      UnityPrint(":FAIL: [[[[ Test Should Have Been Inconclusive But Was Not ]]]]");\
      UNITY_OUTPUT_CHAR('\n');                                                      \
    }

void startPutcharSpy(void);
void endPutcharSpy(void);
char* getBufferPutcharSpy(void);

void startFlushSpy(void);
void endFlushSpy(void);
int getFlushSpyCalls(void);

static int SetToOneToFailInTearDown;
static int SetToOneMeanWeAlreadyCheckedThisGuy;

void setUp(void)
{
    SetToOneToFailInTearDown = 0;
    SetToOneMeanWeAlreadyCheckedThisGuy = 0;
}

void tearDown(void)
{
    endPutcharSpy(); /* Stop suppressing test output */
    if (SetToOneToFailInTearDown == 1)
    {
        /* These will be skipped internally if already failed/ignored */
        TEST_FAIL_MESSAGE("<= Failed in tearDown");
        TEST_IGNORE_MESSAGE("<= Ignored in tearDown");
    }
    if ((SetToOneMeanWeAlreadyCheckedThisGuy == 0) && (Unity.CurrentTestFailed > 0))
    {
        UnityPrint(": [[[[ Test Should Have Passed But Did Not ]]]]");
        UNITY_OUTPUT_CHAR('\n');
    }
}

void testUnitySizeInitializationReminder(void)
{
    /* This test ensures that sizeof(struct UNITY_STORAGE_T) doesn't change. If this
     * test breaks, go look at the initialization of the Unity global variable
     * in unity.c and make sure we're filling in the proper fields. */
    const char* message = "Unexpected size for UNITY_STORAGE_T struct. Please check that "
                     "the initialization of the Unity symbol in unity.c is "
                     "still correct.";

    /* Define a structure with all the same fields as `struct UNITY_STORAGE_T`. */
#ifdef UNITY_EXCLUDE_DETAILS
    struct {
        const char* TestFile;
        const char* CurrentTestName;
        UNITY_LINE_TYPE CurrentTestLineNumber;
        UNITY_COUNTER_TYPE NumberOfTests;
        UNITY_COUNTER_TYPE TestFailures;
        UNITY_COUNTER_TYPE TestIgnores;
        UNITY_COUNTER_TYPE TestInconclusives;
        UNITY_COUNTER_TYPE CurrentTestFailed;
        UNITY_COUNTER_TYPE CurrentTestIgnored;
        UNITY_COUNTER_TYPE CurrentTestInconclusive;
#ifdef UNITY_INCLUDE_EXEC_TIME
        UNITY_COUNTER_TYPE CurrentTestStartTime;
        UNITY_COUNTER_TYPE CurrentTestStopTime;
#endif
#ifndef UNITY_EXCLUDE_SETJMP_H
        jmp_buf AbortFrame;
#endif
    } _Expected_Unity;
#else
    struct {
        const char* TestFile;
        const char* CurrentTestName;
        const char* CurrentDetails1;
        const char* CurrentDetails2;
        UNITY_LINE_TYPE CurrentTestLineNumber;
        UNITY_COUNTER_TYPE NumberOfTests;
        UNITY_COUNTER_TYPE TestFailures;
        UNITY_COUNTER_TYPE TestIgnores;
        UNITY_COUNTER_TYPE TestInconclusives;
        UNITY_COUNTER_TYPE CurrentTestFailed;
        UNITY_COUNTER_TYPE CurrentTestIgnored;
        UNITY_COUNTER_TYPE CurrentTestInconclusive;
#ifdef UNITY_INCLUDE_EXEC_TIME
        UNITY_COUNTER_TYPE CurrentTestStartTime;
        UNITY_COUNTER_TYPE CurrentTestStopTime;
#endif
#ifndef UNITY_EXCLUDE_SETJMP_H
        jmp_buf AbortFrame;
#endif
    } _Expected_Unity;
#endif

    /* Compare our fake structure's size to the actual structure's size. They
     * should be the same.
     *
     * This accounts for alignment, padding, and packing issues that might come
     * up between different architectures. */
    TEST_ASSERT_EQUAL_MESSAGE(sizeof(_Expected_Unity), sizeof(Unity), message);
}

void testPassShouldEndImmediatelyWithPass(void)
{
    TEST_PASS();
    TEST_FAIL_MESSAGE("We should have passed already and finished this test");
}

void testAssumePassShouldEndImmediatelyWithPass(void)
{
    TEST_PASS();
    TEST_FAIL_MESSAGE("We should have passed already and finished this test");
}

void testTrue(void)
{
    TEST_ASSERT(1);

    TEST_ASSERT_TRUE(1);
}

void testAssumeTrue(void)
{
    TEST_ASSUME(1);

    TEST_ASSUME_TRUE(1);
}

void testFalse(void)
{
    TEST_ASSERT_FALSE(0);

    TEST_ASSERT_UNLESS(0);
}

void testAssumeFalse(void)
{
    TEST_ASSUME_FALSE(0);

    TEST_ASSUME_UNLESS(0);
}

void testPreviousPass(void)
{
    TEST_ASSERT_EQUAL_INT(0U, Unity.TestFailures);
}

void testAssumePreviousPass(void)
{
    TEST_ASSUME_EQUAL_INT(0U, Unity.TestFailures);
}

void testNotVanilla(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT(0);
    VERIFY_FAILS_END
}

void testAssumeNotVanilla(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME(0);
    VERIFY_INCONCLUSIVE_END
}

void testNotTrue(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_TRUE(0);
    VERIFY_FAILS_END
}

void testAssumeNotTrue(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_TRUE(0);
    VERIFY_INCONCLUSIVE_END
}

void testNotFalse(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FALSE(1);
    VERIFY_FAILS_END
}

void testAssumeNotFalse(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FALSE(1);
    VERIFY_INCONCLUSIVE_END
}

void testNotUnless(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UNLESS(1);
    VERIFY_FAILS_END
}

void testAssumeNotUnless(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UNLESS(1);
    VERIFY_INCONCLUSIVE_END
}

void testNotNotEqual(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_NOT_EQUAL(10, 10);
    VERIFY_FAILS_END
}

void testAssumeNotNotEqual(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_NOT_EQUAL(10, 10);
    VERIFY_INCONCLUSIVE_END
}

void testFail(void)
{
    EXPECT_ABORT_BEGIN
    TEST_FAIL_MESSAGE("Expected for testing");
    VERIFY_FAILS_END
}

void testInconclusive(void)
{
    EXPECT_ABORT_BEGIN
    TEST_INCONCLUSIVE();
    TEST_FAIL_MESSAGE("This should not be reached");
    VERIFY_INCONCLUSIVE_END
}

void testInconclusiveMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_INCONCLUSIVE_MESSAGE("Expected for testing");
    TEST_FAIL_MESSAGE("This should not be reached");
    VERIFY_INCONCLUSIVE_END
}

void testIsNull(void)
{
    char* ptr1 = NULL;
    const char* ptr2 = "hello";

    TEST_ASSERT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
}

void testAssumeIsNull(void)
{
    char* ptr1 = NULL;
    const char* ptr2 = "hello";

    TEST_ASSUME_NULL(ptr1);
    TEST_ASSUME_NOT_NULL(ptr2);
}

void testIsNullShouldFailIfNot(void)
{
    const char* ptr1 = "hello";

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_NULL(ptr1);
    VERIFY_FAILS_END
}

void testAssumeIsNullShouldFailIfNot(void)
{
    const char* ptr1 = "hello";

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_NULL(ptr1);
    VERIFY_INCONCLUSIVE_END
}

void testIgnore(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE();
    TEST_FAIL_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testAssumeIgnore(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE();
    TEST_FAIL_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testIgnoreMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE_MESSAGE("This is an expected TEST_IGNORE_MESSAGE string!");
    TEST_FAIL_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testAssumeIgnoreMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE_MESSAGE("This is an expected TEST_IGNORE_MESSAGE string!");
    TEST_FAIL_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testIgnoreInconclusive(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE();
    TEST_INCONCLUSIVE_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testAssumeIgnoreInconclusive(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE();
    TEST_INCONCLUSIVE_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testIgnoreMessageInconclusive(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE_MESSAGE("This is an expected TEST_IGNORE_MESSAGE string!");
    TEST_INCONCLUSIVE_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testAssumeIgnoreMessageInconclusive(void)
{
    EXPECT_ABORT_BEGIN
    TEST_IGNORE_MESSAGE("This is an expected TEST_IGNORE_MESSAGE string!");
    TEST_INCONCLUSIVE_MESSAGE("This should not be reached");
    VERIFY_IGNORES_END
}

void testNotEqualInts(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT(3982, 3983);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInts(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT(3982, 3983);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualInt8s(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT8(-127, -126);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt8s(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT8(-127, -126);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualInt16s(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT16(-16383, -16382);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt16s(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT16(-16383, -16382);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualInt32s(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT32(-2147483647, -2147483648); //use largest 32 bit negative to test printability
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt32s(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT32(-2147483647, -2147483648); //use largest 32 bit negative to test printability
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualBits(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_BITS(0xFF00, 0x5555, 0x5A55);
    VERIFY_FAILS_END
}

void testAssumeNotEqualBits(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_BITS(0xFF00, 0x5555, 0x5A55);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUInts(void)
{
    UNITY_UINT16 v0, v1;

    v0 = 9000;
    v1 = 9001;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUInts(void)
{
    UNITY_UINT16 v0, v1;

    v0 = 9000;
    v1 = 9001;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUInt8s(void)
{
    UNITY_UINT8 v0, v1;

    v0 = 254;
    v1 = 255;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT8(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUInt8s(void)
{
    UNITY_UINT8 v0, v1;

    v0 = 254;
    v1 = 255;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT8(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUInt16s(void)
{
    UNITY_UINT16 v0, v1;

    v0 = 65535;
    v1 = 65534;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT16(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUInt16s(void)
{
    UNITY_UINT16 v0, v1;

    v0 = 65535;
    v1 = 65534;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT16(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUInt32s(void)
{
    UNITY_UINT32 v0, v1;

    v0 = 4294967295;
    v1 = 4294967294;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT32(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUInt32s(void)
{
    UNITY_UINT32 v0, v1;

    v0 = 4294967295;
    v1 = 4294967294;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT32(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHex8s(void)
{
    UNITY_UINT8 v0, v1;

    v0 = 0x23;
    v1 = 0x22;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX8(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHex8s(void)
{
    UNITY_UINT8 v0, v1;

    v0 = 0x23;
    v1 = 0x22;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX8(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHex8sIfSigned(void)
{
    UNITY_INT8 v0, v1;

    v0 = -2;
    v1 = 2;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX8(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHex8sIfSigned(void)
{
    UNITY_INT8 v0, v1;

    v0 = -2;
    v1 = 2;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX8(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHex16s(void)
{
    UNITY_UINT16 v0, v1;

    v0 = 0x1234;
    v1 = 0x1235;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX16(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHex16s(void)
{
    UNITY_UINT16 v0, v1;

    v0 = 0x1234;
    v1 = 0x1235;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX16(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHex16sIfSigned(void)
{
    UNITY_INT16 v0, v1;

    v0 = -1024;
    v1 = -1028;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX16(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHex16sIfSigned(void)
{
    UNITY_INT16 v0, v1;

    v0 = -1024;
    v1 = -1028;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX16(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHex32s(void)
{
    UNITY_UINT32 v0, v1;

    v0 = 900000;
    v1 = 900001;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHex32s(void)
{
    UNITY_UINT32 v0, v1;

    v0 = 900000;
    v1 = 900001;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHex32sIfSigned(void)
{
    UNITY_INT32 v0, v1;

    v0 = -900000;
    v1 = 900001;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHex32sIfSigned(void)
{
    UNITY_INT32 v0, v1;

    v0 = -900000;
    v1 = 900001;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32(v0, v1);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInts(void)
{
    int v0, v1;
    int *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT(1837, 1837);
    TEST_ASSERT_EQUAL_INT(-27365, -27365);
    TEST_ASSERT_EQUAL_INT(v0, v1);
    TEST_ASSERT_EQUAL_INT(19467, v1);
    TEST_ASSERT_EQUAL_INT(v0, 19467);
    TEST_ASSERT_EQUAL_INT(*p0, v1);
    TEST_ASSERT_EQUAL_INT(*p0, *p1);
    TEST_ASSERT_EQUAL_INT(*p0, 19467);
}

void testAssumeEqualInts(void)
{
    int v0, v1;
    int *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT(1837, 1837);
    TEST_ASSUME_EQUAL_INT(-27365, -27365);
    TEST_ASSUME_EQUAL_INT(v0, v1);
    TEST_ASSUME_EQUAL_INT(19467, v1);
    TEST_ASSUME_EQUAL_INT(v0, 19467);
    TEST_ASSUME_EQUAL_INT(*p0, v1);
    TEST_ASSUME_EQUAL_INT(*p0, *p1);
    TEST_ASSUME_EQUAL_INT(*p0, 19467);
}

void testEqualInt8s(void)
{
    UNITY_INT8 v0, v1;
    UNITY_INT8 *p0, *p1;

    v0 = 0x22;
    v1 = 0x22;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT8(0x22, 0x22);
    TEST_ASSERT_EQUAL_INT8(v0, v1);
    TEST_ASSERT_EQUAL_INT8(0x22, v1);
    TEST_ASSERT_EQUAL_INT8(v0, 0x22);
    TEST_ASSERT_EQUAL_INT8(*p0, v1);
    TEST_ASSERT_EQUAL_INT8(*p0, *p1);
    TEST_ASSERT_EQUAL_INT8(*p0, 0x22);
}

void testAssumeEqualInt8s(void)
{
    UNITY_INT8 v0, v1;
    UNITY_INT8 *p0, *p1;

    v0 = 0x22;
    v1 = 0x22;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT8(0x22, 0x22);
    TEST_ASSUME_EQUAL_INT8(v0, v1);
    TEST_ASSUME_EQUAL_INT8(0x22, v1);
    TEST_ASSUME_EQUAL_INT8(v0, 0x22);
    TEST_ASSUME_EQUAL_INT8(*p0, v1);
    TEST_ASSUME_EQUAL_INT8(*p0, *p1);
    TEST_ASSUME_EQUAL_INT8(*p0, 0x22);
}

void testEqualInt8sWhenThereAreDifferencesOutside8Bits(void)
{
    TEST_ASSERT_EQUAL_INT8(0x321,0x421);
    TEST_ASSERT_EQUAL_INT8(0xFF21,0x0021);
}

void testAssumeEqualInt8sWhenThereAreDifferencesOutside8Bits(void)
{
    TEST_ASSUME_EQUAL_INT8(0x321,0x421);
    TEST_ASSUME_EQUAL_INT8(0xFF21,0x0021);
}

void testEqualInt16s(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = 0x7876;
    v1 = 0x7876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT16(0x7876, 0x7876);
    TEST_ASSERT_EQUAL_INT16(v0, v1);
    TEST_ASSERT_EQUAL_INT16(0x7876, v1);
    TEST_ASSERT_EQUAL_INT16(v0, 0x7876);
    TEST_ASSERT_EQUAL_INT16(*p0, v1);
    TEST_ASSERT_EQUAL_INT16(*p0, *p1);
    TEST_ASSERT_EQUAL_INT16(*p0, 0x7876);
}

void testAssumeEqualInt16s(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = 0x7876;
    v1 = 0x7876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT16(0x7876, 0x7876);
    TEST_ASSUME_EQUAL_INT16(v0, v1);
    TEST_ASSUME_EQUAL_INT16(0x7876, v1);
    TEST_ASSUME_EQUAL_INT16(v0, 0x7876);
    TEST_ASSUME_EQUAL_INT16(*p0, v1);
    TEST_ASSUME_EQUAL_INT16(*p0, *p1);
    TEST_ASSUME_EQUAL_INT16(*p0, 0x7876);
}

void testEqualInt16sNegatives(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = -7876;
    v1 = -7876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT16(-7876, -7876);
    TEST_ASSERT_EQUAL_INT16(v0, v1);
    TEST_ASSERT_EQUAL_INT16(-7876, v1);
    TEST_ASSERT_EQUAL_INT16(v0, -7876);
    TEST_ASSERT_EQUAL_INT16(*p0, v1);
    TEST_ASSERT_EQUAL_INT16(*p0, *p1);
    TEST_ASSERT_EQUAL_INT16(*p0, -7876);
}

void testAssumeEqualInt16sNegatives(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = -7876;
    v1 = -7876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT16(-7876, -7876);
    TEST_ASSUME_EQUAL_INT16(v0, v1);
    TEST_ASSUME_EQUAL_INT16(-7876, v1);
    TEST_ASSUME_EQUAL_INT16(v0, -7876);
    TEST_ASSUME_EQUAL_INT16(*p0, v1);
    TEST_ASSUME_EQUAL_INT16(*p0, *p1);
    TEST_ASSUME_EQUAL_INT16(*p0, -7876);
}

void testEqualInt16sWhenThereAreDifferencesOutside16Bits(void)
{
    TEST_ASSERT_EQUAL_INT16(0x54321,0x64321);
    TEST_ASSERT_EQUAL_INT16(0xFFFF4321,0x00004321);
}

void testAssumeEqualInt16sWhenThereAreDifferencesOutside16Bits(void)
{
    TEST_ASSUME_EQUAL_INT16(0x54321,0x64321);
    TEST_ASSUME_EQUAL_INT16(0xFFFF4321,0x00004321);
}

void testEqualInt32s(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = 0x78760000;
    v1 = 0x78760000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT32(0x78760000, 0x78760000);
    TEST_ASSERT_EQUAL_INT32(v0, v1);
    TEST_ASSERT_EQUAL_INT32(0x78760000, v1);
    TEST_ASSERT_EQUAL_INT32(v0, 0x78760000);
    TEST_ASSERT_EQUAL_INT32(*p0, v1);
    TEST_ASSERT_EQUAL_INT32(*p0, *p1);
    TEST_ASSERT_EQUAL_INT32(*p0, 0x78760000);
}

void testAssumeEqualInt32s(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = 0x78760000;
    v1 = 0x78760000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT32(0x78760000, 0x78760000);
    TEST_ASSUME_EQUAL_INT32(v0, v1);
    TEST_ASSUME_EQUAL_INT32(0x78760000, v1);
    TEST_ASSUME_EQUAL_INT32(v0, 0x78760000);
    TEST_ASSUME_EQUAL_INT32(*p0, v1);
    TEST_ASSUME_EQUAL_INT32(*p0, *p1);
    TEST_ASSUME_EQUAL_INT32(*p0, 0x78760000);
}

void testEqualInt32sNegatives(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = -123456789;
    v1 = -123456789;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT32(-123456789, -123456789);
    TEST_ASSERT_EQUAL_INT32(v0, v1);
    TEST_ASSERT_EQUAL_INT32(-123456789, v1);
    TEST_ASSERT_EQUAL_INT32(v0, -123456789);
    TEST_ASSERT_EQUAL_INT32(*p0, v1);
    TEST_ASSERT_EQUAL_INT32(*p0, *p1);
    TEST_ASSERT_EQUAL_INT32(*p0, -123456789);
}

void testAssumeEqualInt32sNegatives(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = -123456789;
    v1 = -123456789;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT32(-123456789, -123456789);
    TEST_ASSUME_EQUAL_INT32(v0, v1);
    TEST_ASSUME_EQUAL_INT32(-123456789, v1);
    TEST_ASSUME_EQUAL_INT32(v0, -123456789);
    TEST_ASSUME_EQUAL_INT32(*p0, v1);
    TEST_ASSUME_EQUAL_INT32(*p0, *p1);
    TEST_ASSUME_EQUAL_INT32(*p0, -123456789);
}


void testEqualUints(void)
{
    unsigned int v0, v1;
    unsigned int *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_UINT(1837, 1837);
    TEST_ASSERT_EQUAL_UINT(v0, v1);
    TEST_ASSERT_EQUAL_UINT(19467, v1);
    TEST_ASSERT_EQUAL_UINT(v0, 19467);
    TEST_ASSERT_EQUAL_UINT(*p0, v1);
    TEST_ASSERT_EQUAL_UINT(*p0, *p1);
    TEST_ASSERT_EQUAL_UINT(*p0, 19467);
    TEST_ASSERT_EQUAL_UINT(60872u, 60872u);
}

void testAssumeEqualUints(void)
{
    unsigned int v0, v1;
    unsigned int *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_UINT(1837, 1837);
    TEST_ASSUME_EQUAL_UINT(v0, v1);
    TEST_ASSUME_EQUAL_UINT(19467, v1);
    TEST_ASSUME_EQUAL_UINT(v0, 19467);
    TEST_ASSUME_EQUAL_UINT(*p0, v1);
    TEST_ASSUME_EQUAL_UINT(*p0, *p1);
    TEST_ASSUME_EQUAL_UINT(*p0, 19467);
    TEST_ASSUME_EQUAL_UINT(60872u, 60872u);
}


void testEqualUint8s(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0x22;
    v1 = 0x22;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_UINT8(0x22, 0x22);
    TEST_ASSERT_EQUAL_UINT8(v0, v1);
    TEST_ASSERT_EQUAL_UINT8(0x22, v1);
    TEST_ASSERT_EQUAL_UINT8(v0, 0x22);
    TEST_ASSERT_EQUAL_UINT8(*p0, v1);
    TEST_ASSERT_EQUAL_UINT8(*p0, *p1);
    TEST_ASSERT_EQUAL_UINT8(*p0, 0x22);
}

void testAssumeEqualUint8s(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0x22;
    v1 = 0x22;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_UINT8(0x22, 0x22);
    TEST_ASSUME_EQUAL_UINT8(v0, v1);
    TEST_ASSUME_EQUAL_UINT8(0x22, v1);
    TEST_ASSUME_EQUAL_UINT8(v0, 0x22);
    TEST_ASSUME_EQUAL_UINT8(*p0, v1);
    TEST_ASSUME_EQUAL_UINT8(*p0, *p1);
    TEST_ASSUME_EQUAL_UINT8(*p0, 0x22);
}

void testEqualUint8sWhenThereAreDifferencesOutside8Bits(void)
{
    TEST_ASSERT_EQUAL_UINT8(0x321,0x421);
    TEST_ASSERT_EQUAL_UINT8(0xFF21,0x0021);
}

void testAssumeEqualUint8sWhenThereAreDifferencesOutside8Bits(void)
{
    TEST_ASSUME_EQUAL_UINT8(0x321,0x421);
    TEST_ASSUME_EQUAL_UINT8(0xFF21,0x0021);
}

void testEqualUint16s(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0x9876;
    v1 = 0x9876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_UINT16(0x9876, 0x9876);
    TEST_ASSERT_EQUAL_UINT16(v0, v1);
    TEST_ASSERT_EQUAL_UINT16(0x9876, v1);
    TEST_ASSERT_EQUAL_UINT16(v0, 0x9876);
    TEST_ASSERT_EQUAL_UINT16(*p0, v1);
    TEST_ASSERT_EQUAL_UINT16(*p0, *p1);
    TEST_ASSERT_EQUAL_UINT16(*p0, 0x9876);
}

void testAssumeEqualUint16s(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0x9876;
    v1 = 0x9876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_UINT16(0x9876, 0x9876);
    TEST_ASSUME_EQUAL_UINT16(v0, v1);
    TEST_ASSUME_EQUAL_UINT16(0x9876, v1);
    TEST_ASSUME_EQUAL_UINT16(v0, 0x9876);
    TEST_ASSUME_EQUAL_UINT16(*p0, v1);
    TEST_ASSUME_EQUAL_UINT16(*p0, *p1);
    TEST_ASSUME_EQUAL_UINT16(*p0, 0x9876);
}

void testEqualUint16sWhenThereAreDifferencesOutside16Bits(void)
{
    TEST_ASSERT_EQUAL_UINT16(0x54321,0x64321);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF4321,0x00004321);
}

void testAssumeEqualUint16sWhenThereAreDifferencesOutside16Bits(void)
{
    TEST_ASSUME_EQUAL_UINT16(0x54321,0x64321);
    TEST_ASSUME_EQUAL_UINT16(0xFFFF4321,0x00004321);
}

void testEqualUint32s(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0x98760000;
    v1 = 0x98760000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_UINT32(0x98760000, 0x98760000);
    TEST_ASSERT_EQUAL_UINT32(v0, v1);
    TEST_ASSERT_EQUAL_UINT32(0x98760000, v1);
    TEST_ASSERT_EQUAL_UINT32(v0, 0x98760000);
    TEST_ASSERT_EQUAL_UINT32(*p0, v1);
    TEST_ASSERT_EQUAL_UINT32(*p0, *p1);
    TEST_ASSERT_EQUAL_UINT32(*p0, 0x98760000);
}

void testAssumeEqualUint32s(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0x98760000;
    v1 = 0x98760000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_UINT32(0x98760000, 0x98760000);
    TEST_ASSUME_EQUAL_UINT32(v0, v1);
    TEST_ASSUME_EQUAL_UINT32(0x98760000, v1);
    TEST_ASSUME_EQUAL_UINT32(v0, 0x98760000);
    TEST_ASSUME_EQUAL_UINT32(*p0, v1);
    TEST_ASSUME_EQUAL_UINT32(*p0, *p1);
    TEST_ASSUME_EQUAL_UINT32(*p0, 0x98760000);
}

void testNotEqual(void)
{
    TEST_ASSERT_NOT_EQUAL(0, 1);
    TEST_ASSERT_NOT_EQUAL(1, 0);
    TEST_ASSERT_NOT_EQUAL(100, 101);
    TEST_ASSERT_NOT_EQUAL(0, -1);
    TEST_ASSERT_NOT_EQUAL(65535, -65535);
    TEST_ASSERT_NOT_EQUAL(75, 900);
    TEST_ASSERT_NOT_EQUAL(-100, -101);
}

void testAssumeNotEqual(void)
{
    TEST_ASSUME_NOT_EQUAL(0, 1);
    TEST_ASSUME_NOT_EQUAL(1, 0);
    TEST_ASSUME_NOT_EQUAL(100, 101);
    TEST_ASSUME_NOT_EQUAL(0, -1);
    TEST_ASSUME_NOT_EQUAL(65535, -65535);
    TEST_ASSUME_NOT_EQUAL(75, 900);
    TEST_ASSUME_NOT_EQUAL(-100, -101);
}

void testEqualHex8s(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0x22;
    v1 = 0x22;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_HEX8(0x22, 0x22);
    TEST_ASSERT_EQUAL_HEX8(v0, v1);
    TEST_ASSERT_EQUAL_HEX8(0x22, v1);
    TEST_ASSERT_EQUAL_HEX8(v0, 0x22);
    TEST_ASSERT_EQUAL_HEX8(*p0, v1);
    TEST_ASSERT_EQUAL_HEX8(*p0, *p1);
    TEST_ASSERT_EQUAL_HEX8(*p0, 0x22);
}

void testAssumeEqualHex8s(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0x22;
    v1 = 0x22;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_HEX8(0x22, 0x22);
    TEST_ASSUME_EQUAL_HEX8(v0, v1);
    TEST_ASSUME_EQUAL_HEX8(0x22, v1);
    TEST_ASSUME_EQUAL_HEX8(v0, 0x22);
    TEST_ASSUME_EQUAL_HEX8(*p0, v1);
    TEST_ASSUME_EQUAL_HEX8(*p0, *p1);
    TEST_ASSUME_EQUAL_HEX8(*p0, 0x22);
}

void testEqualHex8sWhenThereAreDifferencesOutside8Bits(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x321,0x421);
    TEST_ASSERT_EQUAL_HEX8(0xFF21,0x0021);
}

void testAssumeEqualHex8sWhenThereAreDifferencesOutside8Bits(void)
{
    TEST_ASSUME_EQUAL_HEX8(0x321,0x421);
    TEST_ASSUME_EQUAL_HEX8(0xFF21,0x0021);
}

void testEqualHex8sNegatives(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0xDD;
    v1 = 0xDD;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_HEX8(0xDD, 0xDD);
    TEST_ASSERT_EQUAL_HEX8(v0, v1);
    TEST_ASSERT_EQUAL_HEX8(0xDD, v1);
    TEST_ASSERT_EQUAL_HEX8(v0, 0xDD);
    TEST_ASSERT_EQUAL_HEX8(*p0, v1);
    TEST_ASSERT_EQUAL_HEX8(*p0, *p1);
    TEST_ASSERT_EQUAL_HEX8(*p0, 0xDD);
}

void testAssumeEqualHex8sNegatives(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0xDD;
    v1 = 0xDD;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_HEX8(0xDD, 0xDD);
    TEST_ASSUME_EQUAL_HEX8(v0, v1);
    TEST_ASSUME_EQUAL_HEX8(0xDD, v1);
    TEST_ASSUME_EQUAL_HEX8(v0, 0xDD);
    TEST_ASSUME_EQUAL_HEX8(*p0, v1);
    TEST_ASSUME_EQUAL_HEX8(*p0, *p1);
    TEST_ASSUME_EQUAL_HEX8(*p0, 0xDD);
}

void testEqualHex16s(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0x9876;
    v1 = 0x9876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_HEX16(0x9876, 0x9876);
    TEST_ASSERT_EQUAL_HEX16(v0, v1);
    TEST_ASSERT_EQUAL_HEX16(0x9876, v1);
    TEST_ASSERT_EQUAL_HEX16(v0, 0x9876);
    TEST_ASSERT_EQUAL_HEX16(*p0, v1);
    TEST_ASSERT_EQUAL_HEX16(*p0, *p1);
    TEST_ASSERT_EQUAL_HEX16(*p0, 0x9876);
}

void testAssumeEqualHex16s(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0x9876;
    v1 = 0x9876;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_HEX16(0x9876, 0x9876);
    TEST_ASSUME_EQUAL_HEX16(v0, v1);
    TEST_ASSUME_EQUAL_HEX16(0x9876, v1);
    TEST_ASSUME_EQUAL_HEX16(v0, 0x9876);
    TEST_ASSUME_EQUAL_HEX16(*p0, v1);
    TEST_ASSUME_EQUAL_HEX16(*p0, *p1);
    TEST_ASSUME_EQUAL_HEX16(*p0, 0x9876);
}

void testEqualHex16sWhenThereAreDifferencesOutside16Bits(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x54321,0x64321);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF4321,0x00004321);
}

void testAssumeEqualHex16sWhenThereAreDifferencesOutside16Bits(void)
{
    TEST_ASSUME_EQUAL_HEX16(0x54321,0x64321);
    TEST_ASSUME_EQUAL_HEX16(0xFFFF4321,0x00004321);
}

void testEqualHex32s(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0x98765432ul;
    v1 = 0x98765432ul;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_HEX32(0x98765432ul, 0x98765432ul);
    TEST_ASSERT_EQUAL_HEX32(v0, v1);
    TEST_ASSERT_EQUAL_HEX32(0x98765432ul, v1);
    TEST_ASSERT_EQUAL_HEX32(v0, 0x98765432ul);
    TEST_ASSERT_EQUAL_HEX32(*p0, v1);
    TEST_ASSERT_EQUAL_HEX32(*p0, *p1);
    TEST_ASSERT_EQUAL_HEX32(*p0, 0x98765432ul);
}

void testAssumeEqualHex32s(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0x98765432ul;
    v1 = 0x98765432ul;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_HEX32(0x98765432ul, 0x98765432ul);
    TEST_ASSUME_EQUAL_HEX32(v0, v1);
    TEST_ASSUME_EQUAL_HEX32(0x98765432ul, v1);
    TEST_ASSUME_EQUAL_HEX32(v0, 0x98765432ul);
    TEST_ASSUME_EQUAL_HEX32(*p0, v1);
    TEST_ASSUME_EQUAL_HEX32(*p0, *p1);
    TEST_ASSUME_EQUAL_HEX32(*p0, 0x98765432ul);
}

void testEqualBits(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;
    UNITY_UINT32 v1 = 0x55550000;

    TEST_ASSERT_BITS(v1, v0, 0x55550000);
    TEST_ASSERT_BITS(v1, v0, 0xFF55CC00);
    TEST_ASSERT_BITS(0xFFFFFFFF, v0, 0xFF55AA00);
    TEST_ASSERT_BITS(0xFFFFFFFF, v0, v0);
    TEST_ASSERT_BITS(0xF0F0F0F0, v0, 0xFC5DAE0F);
    TEST_ASSERT_BITS_HIGH(v1, v0);
    TEST_ASSERT_BITS_LOW(0x000055FF, v0);
    TEST_ASSERT_BIT_HIGH(30, v0);
    TEST_ASSERT_BIT_LOW(5, v0);
}

void testAssumeEqualBits(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;
    UNITY_UINT32 v1 = 0x55550000;

    TEST_ASSUME_BITS(v1, v0, 0x55550000);
    TEST_ASSUME_BITS(v1, v0, 0xFF55CC00);
    TEST_ASSUME_BITS(0xFFFFFFFF, v0, 0xFF55AA00);
    TEST_ASSUME_BITS(0xFFFFFFFF, v0, v0);
    TEST_ASSUME_BITS(0xF0F0F0F0, v0, 0xFC5DAE0F);
    TEST_ASSUME_BITS_HIGH(v1, v0);
    TEST_ASSUME_BITS_LOW(0x000055FF, v0);
    TEST_ASSUME_BIT_HIGH(30, v0);
    TEST_ASSUME_BIT_LOW(5, v0);
}

void testNotEqualBitHigh(void)
{
    UNITY_UINT32 v0 = 0x7F55AA00;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_BIT_HIGH(31, v0);
    VERIFY_FAILS_END
}

void testAssumeNotEqualBitHigh(void)
{
    UNITY_UINT32 v0 = 0x7F55AA00;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_BIT_HIGH(31, v0);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualBitLow(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_BIT_LOW(30, v0);
    VERIFY_FAILS_END
}

void testAssumeNotEqualBitLow(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_BIT_LOW(30, v0);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualBitsHigh(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;
    UNITY_UINT32 v1 = 0x55550000;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_BITS_HIGH(v0, v1);
    VERIFY_FAILS_END

}

void testAssumeNotEqualBitsHigh(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;
    UNITY_UINT32 v1 = 0x55550000;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_BITS_HIGH(v0, v1);
    VERIFY_INCONCLUSIVE_END

}

void testNotEqualBitsLow(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;
    UNITY_UINT32 v1 = 0x55550000;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_BITS_LOW(v0, v1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualBitsLow(void)
{
    UNITY_UINT32 v0 = 0xFF55AA00;
    UNITY_UINT32 v1 = 0x55550000;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_BITS_LOW(v0, v1);
    VERIFY_INCONCLUSIVE_END
}


void testEqualShorts(void)
{
    short v0, v1;
    short *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT(1837, 1837);
    TEST_ASSERT_EQUAL_INT(-2987, -2987);
    TEST_ASSERT_EQUAL_INT(v0, v1);
    TEST_ASSERT_EQUAL_INT(19467, v1);
    TEST_ASSERT_EQUAL_INT(v0, 19467);
    TEST_ASSERT_EQUAL_INT(*p0, v1);
    TEST_ASSERT_EQUAL_INT(*p0, *p1);
    TEST_ASSERT_EQUAL_INT(*p0, 19467);
}

void testAssumeEqualShorts(void)
{
    short v0, v1;
    short *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT(1837, 1837);
    TEST_ASSUME_EQUAL_INT(-2987, -2987);
    TEST_ASSUME_EQUAL_INT(v0, v1);
    TEST_ASSUME_EQUAL_INT(19467, v1);
    TEST_ASSUME_EQUAL_INT(v0, 19467);
    TEST_ASSUME_EQUAL_INT(*p0, v1);
    TEST_ASSUME_EQUAL_INT(*p0, *p1);
    TEST_ASSUME_EQUAL_INT(*p0, 19467);
}

void testEqualUShorts(void)
{
    unsigned short v0, v1;
    unsigned short *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_UINT(1837, 1837);
    TEST_ASSERT_EQUAL_UINT(2987, 2987);
    TEST_ASSERT_EQUAL_UINT(v0, v1);
    TEST_ASSERT_EQUAL_UINT(19467, v1);
    TEST_ASSERT_EQUAL_UINT(v0, 19467);
    TEST_ASSERT_EQUAL_UINT(*p0, v1);
    TEST_ASSERT_EQUAL_UINT(*p0, *p1);
    TEST_ASSERT_EQUAL_UINT(*p0, 19467);
}

void testAssumeEqualUShorts(void)
{
    unsigned short v0, v1;
    unsigned short *p0, *p1;

    v0 = 19467;
    v1 = 19467;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_UINT(1837, 1837);
    TEST_ASSUME_EQUAL_UINT(2987, 2987);
    TEST_ASSUME_EQUAL_UINT(v0, v1);
    TEST_ASSUME_EQUAL_UINT(19467, v1);
    TEST_ASSUME_EQUAL_UINT(v0, 19467);
    TEST_ASSUME_EQUAL_UINT(*p0, v1);
    TEST_ASSUME_EQUAL_UINT(*p0, *p1);
    TEST_ASSUME_EQUAL_UINT(*p0, 19467);
}

void testEqualChars(void)
{
    signed char v0, v1;
    signed char *p0, *p1;

    v0 = 109;
    v1 = 109;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT(42, 42);
    TEST_ASSERT_EQUAL_INT(-116, -116);
    TEST_ASSERT_EQUAL_INT(v0, v1);
    TEST_ASSERT_EQUAL_INT(109, v1);
    TEST_ASSERT_EQUAL_INT(v0, 109);
    TEST_ASSERT_EQUAL_INT(*p0, v1);
    TEST_ASSERT_EQUAL_INT(*p0, *p1);
    TEST_ASSERT_EQUAL_INT(*p0, 109);
}

void testAssumeEqualChars(void)
{
    signed char v0, v1;
    signed char *p0, *p1;

    v0 = 109;
    v1 = 109;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT(42, 42);
    TEST_ASSUME_EQUAL_INT(-116, -116);
    TEST_ASSUME_EQUAL_INT(v0, v1);
    TEST_ASSUME_EQUAL_INT(109, v1);
    TEST_ASSUME_EQUAL_INT(v0, 109);
    TEST_ASSUME_EQUAL_INT(*p0, v1);
    TEST_ASSUME_EQUAL_INT(*p0, *p1);
    TEST_ASSUME_EQUAL_INT(*p0, 109);
}

void testEqualUChars(void)
{
    unsigned char v0, v1;
    unsigned char *p0, *p1;

    v0 = 251;
    v1 = 251;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT(42, 42);
    TEST_ASSERT_EQUAL_INT(v0, v1);
    TEST_ASSERT_EQUAL_INT(251, v1);
    TEST_ASSERT_EQUAL_INT(v0, 251);
    TEST_ASSERT_EQUAL_INT(*p0, v1);
    TEST_ASSERT_EQUAL_INT(*p0, *p1);
    TEST_ASSERT_EQUAL_INT(*p0, 251);
}

void testAssumeEqualUChars(void)
{
    unsigned char v0, v1;
    unsigned char *p0, *p1;

    v0 = 251;
    v1 = 251;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT(42, 42);
    TEST_ASSUME_EQUAL_INT(v0, v1);
    TEST_ASSUME_EQUAL_INT(251, v1);
    TEST_ASSUME_EQUAL_INT(v0, 251);
    TEST_ASSUME_EQUAL_INT(*p0, v1);
    TEST_ASSUME_EQUAL_INT(*p0, *p1);
    TEST_ASSUME_EQUAL_INT(*p0, 251);
}

void testEqualPointers(void)
{
    int v0, v1;
    int *p0, *p1, *p2;

    v0 = 19467;
    v1 = 18271;
    p0 = &v0;
    p1 = &v1;
    p2 = &v1;

    TEST_ASSERT_EQUAL_PTR(p0, &v0);
    TEST_ASSERT_EQUAL_PTR(&v1, p1);
    TEST_ASSERT_EQUAL_PTR(p2, p1);
    TEST_ASSERT_EQUAL_PTR(&v0, &v0);
}

void testAssumeEqualPointers(void)
{
    int v0, v1;
    int *p0, *p1, *p2;

    v0 = 19467;
    v1 = 18271;
    p0 = &v0;
    p1 = &v1;
    p2 = &v1;

    TEST_ASSUME_EQUAL_PTR(p0, &v0);
    TEST_ASSUME_EQUAL_PTR(&v1, p1);
    TEST_ASSUME_EQUAL_PTR(p2, p1);
    TEST_ASSUME_EQUAL_PTR(&v0, &v0);
}

void testNotEqualPointers(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_PTR(0x12345678, 0x12345677);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPointers(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_PTR(0x12345678, 0x12345677);
    VERIFY_INCONCLUSIVE_END
}

void testIntsWithinDelta(void)
{
    TEST_ASSERT_INT_WITHIN(1, 5000, 5001);
    TEST_ASSERT_INT_WITHIN(5, 5000, 4996);
    TEST_ASSERT_INT_WITHIN(5, 5000, 5005);
    TEST_ASSERT_INT_WITHIN(500, 50, -440);

    TEST_ASSERT_INT_WITHIN(2, -1, -1);
    TEST_ASSERT_INT_WITHIN(5, 1, -1);
    TEST_ASSERT_INT_WITHIN(5, -1, 1);
}

void testAssumeIntsWithinDelta(void)
{
    TEST_ASSUME_INT_WITHIN(1, 5000, 5001);
    TEST_ASSUME_INT_WITHIN(5, 5000, 4996);
    TEST_ASSUME_INT_WITHIN(5, 5000, 5005);
    TEST_ASSUME_INT_WITHIN(500, 50, -440);

    TEST_ASSUME_INT_WITHIN(2, -1, -1);
    TEST_ASSUME_INT_WITHIN(5, 1, -1);
    TEST_ASSUME_INT_WITHIN(5, -1, 1);
}

void testIntsWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_INT_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSERT_INT_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSERT_INT_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
    TEST_ASSERT_INT_WITHIN_MESSAGE(500, 50, -440, "Custom Message.");

    TEST_ASSERT_INT_WITHIN_MESSAGE(2, -1, -1, "Custom Message.");
    TEST_ASSERT_INT_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    TEST_ASSERT_INT_WITHIN_MESSAGE(5, -1, 1, "Custom Message.");
}

void testAssumeIntsWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_INT_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSUME_INT_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSUME_INT_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
    TEST_ASSUME_INT_WITHIN_MESSAGE(500, 50, -440, "Custom Message.");

    TEST_ASSUME_INT_WITHIN_MESSAGE(2, -1, -1, "Custom Message.");
    TEST_ASSUME_INT_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    TEST_ASSUME_INT_WITHIN_MESSAGE(5, -1, 1, "Custom Message.");
}

void testIntsNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT_WITHIN(5, 5000, 5006);
    VERIFY_FAILS_END
}

void testAssumeIntsNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT_WITHIN(5, 5000, 5006);
    VERIFY_INCONCLUSIVE_END
}

void testIntsNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT_WITHIN_MESSAGE(5, 5000, 5006, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeIntsNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT_WITHIN_MESSAGE(5, 5000, 5006, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testUIntsWithinDelta(void)
{
    TEST_ASSERT_UINT_WITHIN(1, 5000, 5001);
    TEST_ASSERT_UINT_WITHIN(5, 5000, 4996);
    TEST_ASSERT_UINT_WITHIN(5, 5000, 5005);
}

void testAssumeUIntsWithinDelta(void)
{
    TEST_ASSUME_UINT_WITHIN(1, 5000, 5001);
    TEST_ASSUME_UINT_WITHIN(5, 5000, 4996);
    TEST_ASSUME_UINT_WITHIN(5, 5000, 5005);
}

void testUIntsWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_UINT_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSERT_UINT_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSERT_UINT_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testAssumeUIntsWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_UINT_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSUME_UINT_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSUME_UINT_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testUIntsNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT_WITHIN(1, 2147483647u, 2147483649u);
    VERIFY_FAILS_END
}

void testAssumeUIntsNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT_WITHIN(1, 2147483647u, 2147483649u);
    VERIFY_INCONCLUSIVE_END
}

void testUIntsNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT_WITHIN_MESSAGE(1, 2147483647u, 2147483649u, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUIntsNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT_WITHIN_MESSAGE(1, 2147483647u, 2147483649u, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testUIntsNotWithinDeltaEvenThoughASignedIntWouldPassSmallFirst(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT_WITHIN(5, 1, -1);
    VERIFY_FAILS_END
}

void testAssumeUIntsNotWithinDeltaEvenThoughASignedIntWouldPassSmallFirst(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT_WITHIN(5, 1, -1);
    VERIFY_INCONCLUSIVE_END
}

void testUIntsNotWithinDeltaEvenThoughASignedIntWouldPassSmallFirstAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUIntsNotWithinDeltaEvenThoughASignedIntWouldPassSmallFirstAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testUIntsNotWithinDeltaEvenThoughASignedIntWouldPassBigFirst(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT_WITHIN(5, -1, 1);
    VERIFY_FAILS_END
}

void testAssumeUIntsNotWithinDeltaEvenThoughASignedIntWouldPassBigFirst(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT_WITHIN(5, -1, 1);
    VERIFY_INCONCLUSIVE_END
}

void testUIntsNotWithinDeltaEvenThoughASignedIntWouldPassBigFirstAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT_WITHIN_MESSAGE(5, -1, 1, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUIntsNotWithinDeltaEvenThoughASignedIntWouldPassBigFirstAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT_WITHIN_MESSAGE(5, -1, 1, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testHEX32sWithinDelta(void)
{
    TEST_ASSERT_HEX32_WITHIN(1, 5000, 5001);
    TEST_ASSERT_HEX32_WITHIN(5, 5000, 4996);
    TEST_ASSERT_HEX32_WITHIN(5, 5000, 5005);
}

void testAssumeHEX32sWithinDelta(void)
{
    TEST_ASSUME_HEX32_WITHIN(1, 5000, 5001);
    TEST_ASSUME_HEX32_WITHIN(5, 5000, 4996);
    TEST_ASSUME_HEX32_WITHIN(5, 5000, 5005);
}

void testHEX32sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_HEX32_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSERT_HEX32_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSERT_HEX32_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testAssumeHEX32sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_HEX32_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSUME_HEX32_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSUME_HEX32_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testHEX32sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX32_WITHIN(1, 2147483647u, 2147483649u);
    VERIFY_FAILS_END
}

void testAssumeHEX32sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX32_WITHIN(1, 2147483647u, 2147483649u);
    VERIFY_INCONCLUSIVE_END
}

void testHEX32sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX32_WITHIN_MESSAGE(1, 2147483647u, 2147483649u, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeHEX32sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX32_WITHIN_MESSAGE(1, 2147483647u, 2147483649u, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testHEX32sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX32_WITHIN(5, 1, -1);
    VERIFY_FAILS_END
}

void testAssumeHEX32sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX32_WITHIN(5, 1, -1);
    VERIFY_INCONCLUSIVE_END
}

void testHEX32sNotWithinDeltaEvenThoughASignedIntWouldPassAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX32_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeHEX32sNotWithinDeltaEvenThoughASignedIntWouldPassAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX32_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testHEX16sWithinDelta(void)
{
    TEST_ASSERT_HEX16_WITHIN(1, 5000, 5001);
    TEST_ASSERT_HEX16_WITHIN(5, 5000, 4996);
    TEST_ASSERT_HEX16_WITHIN(5, 5000, 5005);
}

void testAssumeHEX16sWithinDelta(void)
{
    TEST_ASSUME_HEX16_WITHIN(1, 5000, 5001);
    TEST_ASSUME_HEX16_WITHIN(5, 5000, 4996);
    TEST_ASSUME_HEX16_WITHIN(5, 5000, 5005);
}

void testHEX16sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_HEX16_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSERT_HEX16_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSERT_HEX16_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testAssumeHEX16sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_HEX16_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSUME_HEX16_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSUME_HEX16_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testHEX16sWithinDeltaWhenThereAreDifferenceOutsideOf16Bits(void)
{
    TEST_ASSERT_HEX16_WITHIN(5, 0x54321, 0x44321);
}

void testAssumeHEX16sWithinDeltaWhenThereAreDifferenceOutsideOf16Bits(void)
{
    TEST_ASSUME_HEX16_WITHIN(5, 0x54321, 0x44321);
}

void testHEX16sWithinDeltaWhenThereAreDifferenceOutsideOf16BitsAndCustomMessage(void)
{
    TEST_ASSERT_HEX16_WITHIN_MESSAGE(5, 0x54321, 0x44321, "Custom Message.");
}

void testAssumeHEX16sWithinDeltaWhenThereAreDifferenceOutsideOf16BitsAndCustomMessage(void)
{
    TEST_ASSUME_HEX16_WITHIN_MESSAGE(5, 0x54321, 0x44321, "Custom Message.");
}

void testHEX16sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX16_WITHIN(2, 65535, 0);
    VERIFY_FAILS_END
}

void testAssumeHEX16sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX16_WITHIN(2, 65535, 0);
    VERIFY_INCONCLUSIVE_END
}

void testHEX16sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX16_WITHIN_MESSAGE(2, 65535, 0, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeHEX16sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX16_WITHIN_MESSAGE(2, 65535, 0, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testHEX8sWithinDelta(void)
{
    TEST_ASSERT_HEX8_WITHIN(1, 254, 255);
    TEST_ASSERT_HEX8_WITHIN(5, 251, 255);
    TEST_ASSERT_HEX8_WITHIN(5, 1, 4);
}

void testAssumeHEX8sWithinDelta(void)
{
    TEST_ASSUME_HEX8_WITHIN(1, 254, 255);
    TEST_ASSUME_HEX8_WITHIN(5, 251, 255);
    TEST_ASSUME_HEX8_WITHIN(5, 1, 4);
}

void testHEX8sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_HEX8_WITHIN_MESSAGE(1, 254, 255, "Custom Message.");
    TEST_ASSERT_HEX8_WITHIN_MESSAGE(5, 251, 255, "Custom Message.");
    TEST_ASSERT_HEX8_WITHIN_MESSAGE(5, 1, 4, "Custom Message.");
}

void testAssumeHEX8sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_HEX8_WITHIN_MESSAGE(1, 254, 255, "Custom Message.");
    TEST_ASSUME_HEX8_WITHIN_MESSAGE(5, 251, 255, "Custom Message.");
    TEST_ASSUME_HEX8_WITHIN_MESSAGE(5, 1, 4, "Custom Message.");
}

void testHEX8sWithinDeltaWhenThereAreDifferenceOutsideOf8Bits(void)
{
    TEST_ASSERT_HEX8_WITHIN(5, 0x123, 0xF23);
}

void testAssumeHEX8sWithinDeltaWhenThereAreDifferenceOutsideOf8Bits(void)
{
    TEST_ASSUME_HEX8_WITHIN(5, 0x123, 0xF23);
}

void testHEX8sWithinDeltaWhenThereAreDifferenceOutsideOf8BitsAndCustomMessage(void)
{
    TEST_ASSERT_HEX8_WITHIN_MESSAGE(5, 0x123, 0xF23, "Custom Message.");
}

void testAssumeHEX8sWithinDeltaWhenThereAreDifferenceOutsideOf8BitsAndCustomMessage(void)
{
    TEST_ASSUME_HEX8_WITHIN_MESSAGE(5, 0x123, 0xF23, "Custom Message.");
}

void testHEX8sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX8_WITHIN(2, 255, 0);
    VERIFY_FAILS_END
}

void testAssumeHEX8sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX8_WITHIN(2, 255, 0);
    VERIFY_INCONCLUSIVE_END
}

void testHEX8sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX8_WITHIN_MESSAGE(2, 255, 0, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeHEX8sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX8_WITHIN_MESSAGE(2, 255, 0, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

//-----------------

void testUINT32sWithinDelta(void)
{
    TEST_ASSERT_UINT32_WITHIN(1, 5000, 5001);
    TEST_ASSERT_UINT32_WITHIN(5, 5000, 4996);
    TEST_ASSERT_UINT32_WITHIN(5, 5000, 5005);
}

void testAssumeUINT32sWithinDelta(void)
{
    TEST_ASSUME_UINT32_WITHIN(1, 5000, 5001);
    TEST_ASSUME_UINT32_WITHIN(5, 5000, 4996);
    TEST_ASSUME_UINT32_WITHIN(5, 5000, 5005);
}

void testUINT32sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_UINT32_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSERT_UINT32_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSERT_UINT32_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testAssumeUINT32sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_UINT32_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSUME_UINT32_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSUME_UINT32_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testUINT32sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT32_WITHIN(1, 2147483647u, 2147483649u);
    VERIFY_FAILS_END
}

void testAssumeUINT32sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT32_WITHIN(1, 2147483647u, 2147483649u);
    VERIFY_INCONCLUSIVE_END
}

void testUINT32sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT32_WITHIN_MESSAGE(1, 2147483647u, 2147483649u, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUINT32sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT32_WITHIN_MESSAGE(1, 2147483647u, 2147483649u, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testUINT32sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT32_WITHIN(5, 1, -1);
    VERIFY_FAILS_END
}

void testAssumeUINT32sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT32_WITHIN(5, 1, -1);
    VERIFY_INCONCLUSIVE_END
}

void testUINT32sNotWithinDeltaEvenThoughASignedIntWouldPassAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT32_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUINT32sNotWithinDeltaEvenThoughASignedIntWouldPassAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT32_WITHIN_MESSAGE(5, 1, -1, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testUINT16sWithinDelta(void)
{
    TEST_ASSERT_UINT16_WITHIN(1, 5000, 5001);
    TEST_ASSERT_UINT16_WITHIN(5, 5000, 4996);
    TEST_ASSERT_UINT16_WITHIN(5, 5000, 5005);
}

void testAssumeUINT16sWithinDelta(void)
{
    TEST_ASSUME_UINT16_WITHIN(1, 5000, 5001);
    TEST_ASSUME_UINT16_WITHIN(5, 5000, 4996);
    TEST_ASSUME_UINT16_WITHIN(5, 5000, 5005);
}

void testUINT16sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_UINT16_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSERT_UINT16_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSERT_UINT16_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testAssumeUINT16sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_UINT16_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
    TEST_ASSUME_UINT16_WITHIN_MESSAGE(5, 5000, 4996, "Custom Message.");
    TEST_ASSUME_UINT16_WITHIN_MESSAGE(5, 5000, 5005, "Custom Message.");
}

void testUINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16Bits(void)
{
    TEST_ASSERT_UINT16_WITHIN(5, 0x54321, 0x44321);
}

void testAssumeUINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16Bits(void)
{
    TEST_ASSUME_UINT16_WITHIN(5, 0x54321, 0x44321);
}

void testUINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16BitsAndCustomMessage(void)
{
    TEST_ASSERT_UINT16_WITHIN_MESSAGE(5, 0x54321, 0x44321, "Custom Message.");
}

void testAssumeUINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16BitsAndCustomMessage(void)
{
    TEST_ASSUME_UINT16_WITHIN_MESSAGE(5, 0x54321, 0x44321, "Custom Message.");
}

void testUINT16sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT16_WITHIN(2, 65535, 0);
    VERIFY_FAILS_END
}

void testAssumeUINT16sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT16_WITHIN(2, 65535, 0);
    VERIFY_INCONCLUSIVE_END
}

void testUINT16sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT16_WITHIN_MESSAGE(2, 65535, 0, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUINT16sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT16_WITHIN_MESSAGE(2, 65535, 0, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testUINT8sWithinDelta(void)
{
    TEST_ASSERT_UINT8_WITHIN(1, 254, 255);
    TEST_ASSERT_UINT8_WITHIN(5, 251, 255);
    TEST_ASSERT_UINT8_WITHIN(5, 1, 4);
}

void testAssumeUINT8sWithinDelta(void)
{
    TEST_ASSUME_UINT8_WITHIN(1, 254, 255);
    TEST_ASSUME_UINT8_WITHIN(5, 251, 255);
    TEST_ASSUME_UINT8_WITHIN(5, 1, 4);
}

void testUINT8sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_UINT8_WITHIN_MESSAGE(1, 254, 255, "Custom Message.");
    TEST_ASSERT_UINT8_WITHIN_MESSAGE(5, 251, 255, "Custom Message.");
    TEST_ASSERT_UINT8_WITHIN_MESSAGE(5, 1, 4, "Custom Message.");
}

void testAssumeUINT8sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_UINT8_WITHIN_MESSAGE(1, 254, 255, "Custom Message.");
    TEST_ASSUME_UINT8_WITHIN_MESSAGE(5, 251, 255, "Custom Message.");
    TEST_ASSUME_UINT8_WITHIN_MESSAGE(5, 1, 4, "Custom Message.");
}

void testUINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8Bits(void)
{
    TEST_ASSERT_UINT8_WITHIN(5, 0x123, 0xF23);
}

void testAssumeUINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8Bits(void)
{
    TEST_ASSUME_UINT8_WITHIN(5, 0x123, 0xF23);
}

void testUINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8BitsAndCustomMessage(void)
{
    TEST_ASSERT_UINT8_WITHIN_MESSAGE(5, 0x123, 0xF23, "Custom Message.");
}

void testAssumeUINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8BitsAndCustomMessage(void)
{
    TEST_ASSUME_UINT8_WITHIN_MESSAGE(5, 0x123, 0xF23, "Custom Message.");
}

void testUINT8sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT8_WITHIN(2, 255, 0);
    VERIFY_FAILS_END
}

void testAssumeUINT8sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT8_WITHIN(2, 255, 0);
    VERIFY_INCONCLUSIVE_END
}

void testUINT8sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT8_WITHIN_MESSAGE(2, 255, 0, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeUINT8sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT8_WITHIN_MESSAGE(2, 255, 0, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testINT32sWithinDelta(void)
{
    TEST_ASSERT_INT32_WITHIN(1, 5000, 5001);
    TEST_ASSERT_INT32_WITHIN(5, 1, -2);
    TEST_ASSERT_INT32_WITHIN(5, -2, 1);
}

void testAssumeINT32sWithinDelta(void)
{
    TEST_ASSUME_INT32_WITHIN(1, 5000, 5001);
    TEST_ASSUME_INT32_WITHIN(5, 1, -2);
    TEST_ASSUME_INT32_WITHIN(5, -2, 1);
}

void testINT32sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_INT32_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
}

void testAssumeINT32sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_INT32_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
}

void testINT32sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT32_WITHIN(1, -3, 1);
    VERIFY_FAILS_END
}

void testAssumeINT32sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT32_WITHIN(1, -3, 1);
    VERIFY_INCONCLUSIVE_END
}

void testINT32sNotWithinDeltaAndDifferenceOverflows(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT32_WITHIN(1, -1, 0x7FFFFFFF);
    VERIFY_FAILS_END
}

void testAssumeINT32sNotWithinDeltaAndDifferenceOverflows(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT32_WITHIN(1, -1, 0x7FFFFFFF);
    VERIFY_INCONCLUSIVE_END
}
void testINT32sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT32_WITHIN_MESSAGE(1, -2, 1, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeINT32sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT32_WITHIN_MESSAGE(1, -2, 1, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testINT16sWithinDelta(void)
{
    TEST_ASSERT_INT16_WITHIN(1, 5000, 5001);
    TEST_ASSERT_INT16_WITHIN(5, 2, -2);
    TEST_ASSERT_INT16_WITHIN(5, -2, 2);
}

void testAssumeINT16sWithinDelta(void)
{
    TEST_ASSUME_INT16_WITHIN(1, 5000, 5001);
    TEST_ASSUME_INT16_WITHIN(5, 2, -2);
    TEST_ASSUME_INT16_WITHIN(5, -2, 2);
}

void testINT16sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_INT16_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
}

void testAssumeINT16sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_INT16_WITHIN_MESSAGE(1, 5000, 5001, "Custom Message.");
}

void testINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16Bits(void)
{
    TEST_ASSERT_INT16_WITHIN(5, 0x54321, 0x44321);
}

void testAssumeINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16Bits(void)
{
    TEST_ASSUME_INT16_WITHIN(5, 0x54321, 0x44321);
}

void testINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16BitsAndCustomMessage(void)
{
    TEST_ASSERT_INT16_WITHIN_MESSAGE(5, 0x54321, 0x44321, "Custom Message.");
}

void testAssumeINT16sWithinDeltaWhenThereAreDifferenceOutsideOf16BitsAndCustomMessage(void)
{
    TEST_ASSUME_INT16_WITHIN_MESSAGE(5, 0x54321, 0x44321, "Custom Message.");
}

void testINT16sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT16_WITHIN(2, 4, -2);
    VERIFY_FAILS_END
}

void testAssumeINT16sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT16_WITHIN(2, 4, -2);
    VERIFY_INCONCLUSIVE_END
}

void testINT16sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT16_WITHIN_MESSAGE(2, 3, 0, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeINT16sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT16_WITHIN_MESSAGE(2, 3, 0, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}

void testINT8sWithinDelta(void)
{
    TEST_ASSERT_INT8_WITHIN(1, 127, 126);
    TEST_ASSERT_INT8_WITHIN(5, -2, 2);
    TEST_ASSERT_INT8_WITHIN(5, 2, -2);
}

void testAssumeINT8sWithinDelta(void)
{
    TEST_ASSUME_INT8_WITHIN(1, 127, 126);
    TEST_ASSUME_INT8_WITHIN(5, -2, 2);
    TEST_ASSUME_INT8_WITHIN(5, 2, -2);
}

void testINT8sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSERT_INT8_WITHIN_MESSAGE(5, 1, 4, "Custom Message.");
}

void testAssumeINT8sWithinDeltaAndCustomMessage(void)
{
    TEST_ASSUME_INT8_WITHIN_MESSAGE(5, 1, 4, "Custom Message.");
}

void testINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8Bits(void)
{
    TEST_ASSERT_INT8_WITHIN(5, 0x123, 0xF23);
}

void testAssumeINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8Bits(void)
{
    TEST_ASSUME_INT8_WITHIN(5, 0x123, 0xF23);
}

void testINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8BitsAndCustomMessage(void)
{
    TEST_ASSERT_INT8_WITHIN_MESSAGE(5, 0x123, 0xF23, "Custom Message.");
}

void testAssumeINT8sWithinDeltaWhenThereAreDifferenceOutsideOf8BitsAndCustomMessage(void)
{
    TEST_ASSUME_INT8_WITHIN_MESSAGE(5, 0x123, 0xF23, "Custom Message.");
}

void testINT8sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT8_WITHIN(2, -3, 0);
    VERIFY_FAILS_END
}

void testAssumeINT8sNotWithinDelta(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT8_WITHIN(2, -3, 0);
    VERIFY_INCONCLUSIVE_END
}

void testINT8sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT8_WITHIN_MESSAGE(2, -4, 0, "Custom Message.");
    VERIFY_FAILS_END
}

void testAssumeINT8sNotWithinDeltaAndCustomMessage(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT8_WITHIN_MESSAGE(2, -4, 0, "Custom Message.");
    VERIFY_INCONCLUSIVE_END
}


//-----------------
void testGreaterThan(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 0;
    v1 = 1;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN(v0, v1);
    TEST_ASSERT_GREATER_THAN(*p0, v1);
    TEST_ASSERT_GREATER_THAN(v0, *p1);
    TEST_ASSERT_GREATER_THAN(*p0, *p1);
}

void testAssumeGreaterThan(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 0;
    v1 = 1;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN(v0, v1);
    TEST_ASSUME_GREATER_THAN(*p0, v1);
    TEST_ASSUME_GREATER_THAN(v0, *p1);
    TEST_ASSUME_GREATER_THAN(*p0, *p1);
}

void testGreaterThanINT(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 302;
    v1 = 3334;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_INT(v0, v1);
    TEST_ASSERT_GREATER_THAN_INT(*p0, v1);
    TEST_ASSERT_GREATER_THAN_INT(v0, *p1);
    TEST_ASSERT_GREATER_THAN_INT(*p0, *p1);
}

void testAssumeGreaterThanINT(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 302;
    v1 = 3334;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_INT(v0, v1);
    TEST_ASSUME_GREATER_THAN_INT(*p0, v1);
    TEST_ASSUME_GREATER_THAN_INT(v0, *p1);
    TEST_ASSUME_GREATER_THAN_INT(*p0, *p1);
}


void testGreaterThanINT8(void)
{
    UNITY_INT8 v0, v1;
    UNITY_INT8 *p0, *p1;

    v0 = -128;
    v1 = 127;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_INT8(v0, v1);
    TEST_ASSERT_GREATER_THAN_INT8(*p0, v1);
    TEST_ASSERT_GREATER_THAN_INT8(v0, *p1);
    TEST_ASSERT_GREATER_THAN_INT8(*p0, *p1);
}

void testAssumeGreaterThanINT8(void)
{
    UNITY_INT8 v0, v1;
    UNITY_INT8 *p0, *p1;

    v0 = -128;
    v1 = 127;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_INT8(v0, v1);
    TEST_ASSUME_GREATER_THAN_INT8(*p0, v1);
    TEST_ASSUME_GREATER_THAN_INT8(v0, *p1);
    TEST_ASSUME_GREATER_THAN_INT8(*p0, *p1);
}

void testGreaterThanINT16(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = -32768;
    v1 = 32767;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_INT16(v0, v1);
    TEST_ASSERT_GREATER_THAN_INT16(*p0, v1);
    TEST_ASSERT_GREATER_THAN_INT16(v0, *p1);
    TEST_ASSERT_GREATER_THAN_INT16(*p0, *p1);
}

void testAssumeGreaterThanINT16(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = -32768;
    v1 = 32767;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_INT16(v0, v1);
    TEST_ASSUME_GREATER_THAN_INT16(*p0, v1);
    TEST_ASSUME_GREATER_THAN_INT16(v0, *p1);
    TEST_ASSUME_GREATER_THAN_INT16(*p0, *p1);
}

void testGreaterThanINT32(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = -214783648;
    v1 = 214783647;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_INT32(v0, v1);
    TEST_ASSERT_GREATER_THAN_INT32(*p0, v1);
    TEST_ASSERT_GREATER_THAN_INT32(v0, *p1);
    TEST_ASSERT_GREATER_THAN_INT32(*p0, *p1);
}

void testAssumeGreaterThanINT32(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = -214783648;
    v1 = 214783647;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_INT32(v0, v1);
    TEST_ASSUME_GREATER_THAN_INT32(*p0, v1);
    TEST_ASSUME_GREATER_THAN_INT32(v0, *p1);
    TEST_ASSUME_GREATER_THAN_INT32(*p0, *p1);
}

void testGreaterThanUINT(void)
{
    UNITY_UINT v0, v1;
    UNITY_UINT *p0, *p1;

    v0 = 0;
    v1 = 1;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_UINT(v0, v1);
    TEST_ASSERT_GREATER_THAN_UINT(*p0, v1);
    TEST_ASSERT_GREATER_THAN_UINT(v0, *p1);
    TEST_ASSERT_GREATER_THAN_UINT(*p0, *p1);
}

void testAssumeGreaterThanUINT(void)
{
    UNITY_UINT v0, v1;
    UNITY_UINT *p0, *p1;

    v0 = 0;
    v1 = 1;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_UINT(v0, v1);
    TEST_ASSUME_GREATER_THAN_UINT(*p0, v1);
    TEST_ASSUME_GREATER_THAN_UINT(v0, *p1);
    TEST_ASSUME_GREATER_THAN_UINT(*p0, *p1);
}


void testGreaterThanUINT8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0;
    v1 = 255;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_UINT8(v0, v1);
    TEST_ASSERT_GREATER_THAN_UINT8(*p0, v1);
    TEST_ASSERT_GREATER_THAN_UINT8(v0, *p1);
    TEST_ASSERT_GREATER_THAN_UINT8(*p0, *p1);
}

void testAssumeGreaterThanUINT8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0;
    v1 = 255;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_UINT8(v0, v1);
    TEST_ASSUME_GREATER_THAN_UINT8(*p0, v1);
    TEST_ASSUME_GREATER_THAN_UINT8(v0, *p1);
    TEST_ASSUME_GREATER_THAN_UINT8(*p0, *p1);
}

void testGreaterThanUINT16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0;
    v1 = 65535;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_UINT16(v0, v1);
    TEST_ASSERT_GREATER_THAN_UINT16(*p0, v1);
    TEST_ASSERT_GREATER_THAN_UINT16(v0, *p1);
    TEST_ASSERT_GREATER_THAN_UINT16(*p0, *p1);
}

void testAssumeGreaterThanUINT16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0;
    v1 = 65535;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_UINT16(v0, v1);
    TEST_ASSUME_GREATER_THAN_UINT16(*p0, v1);
    TEST_ASSUME_GREATER_THAN_UINT16(v0, *p1);
    TEST_ASSUME_GREATER_THAN_UINT16(*p0, *p1);
}

void testGreaterThanUINT32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0;
    v1 = 4294967295;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_UINT32(v0, v1);
    TEST_ASSERT_GREATER_THAN_UINT32(*p0, v1);
    TEST_ASSERT_GREATER_THAN_UINT32(v0, *p1);
    TEST_ASSERT_GREATER_THAN_UINT32(*p0, *p1);
}

void testAssumeGreaterThanUINT32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0;
    v1 = 4294967295;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_UINT32(v0, v1);
    TEST_ASSUME_GREATER_THAN_UINT32(*p0, v1);
    TEST_ASSUME_GREATER_THAN_UINT32(v0, *p1);
    TEST_ASSUME_GREATER_THAN_UINT32(*p0, *p1);
}

void testGreaterThanHEX8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0x00;
    v1 = 0xFF;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_HEX8(v0, v1);
    TEST_ASSERT_GREATER_THAN_HEX8(*p0, v1);
    TEST_ASSERT_GREATER_THAN_HEX8(v0, *p1);
    TEST_ASSERT_GREATER_THAN_HEX8(*p0, *p1);
}

void testAssumeGreaterThanHEX8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0x00;
    v1 = 0xFF;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_HEX8(v0, v1);
    TEST_ASSUME_GREATER_THAN_HEX8(*p0, v1);
    TEST_ASSUME_GREATER_THAN_HEX8(v0, *p1);
    TEST_ASSUME_GREATER_THAN_HEX8(*p0, *p1);
}

void testGreaterThanHEX16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0x0000;
    v1 = 0xFFFF;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_HEX16(v0, v1);
    TEST_ASSERT_GREATER_THAN_HEX16(*p0, v1);
    TEST_ASSERT_GREATER_THAN_HEX16(v0, *p1);
    TEST_ASSERT_GREATER_THAN_HEX16(*p0, *p1);
}

void testAssumeGreaterThanHEX16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0x0000;
    v1 = 0xFFFF;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_HEX16(v0, v1);
    TEST_ASSUME_GREATER_THAN_HEX16(*p0, v1);
    TEST_ASSUME_GREATER_THAN_HEX16(v0, *p1);
    TEST_ASSUME_GREATER_THAN_HEX16(*p0, *p1);
}

void testGreaterThanHEX32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0x00000000;
    v1 = 0xFFFFFFFF;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_GREATER_THAN_HEX32(v0, v1);
    TEST_ASSERT_GREATER_THAN_HEX32(*p0, v1);
    TEST_ASSERT_GREATER_THAN_HEX32(v0, *p1);
    TEST_ASSERT_GREATER_THAN_HEX32(*p0, *p1);
}

void testAssumeGreaterThanHEX32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0x00000000;
    v1 = 0xFFFFFFFF;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_GREATER_THAN_HEX32(v0, v1);
    TEST_ASSUME_GREATER_THAN_HEX32(*p0, v1);
    TEST_ASSUME_GREATER_THAN_HEX32(v0, *p1);
    TEST_ASSUME_GREATER_THAN_HEX32(*p0, *p1);
}


void testNotGreaterThan(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_GREATER_THAN(0, -1);
    VERIFY_FAILS_END
}

void testAssumeNotGreaterThan(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_GREATER_THAN(0, -1);
    VERIFY_INCONCLUSIVE_END
}

void testLessThan(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 0;
    v1 = -1;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN(v0, v1);
    TEST_ASSERT_LESS_THAN(*p0, v1);
    TEST_ASSERT_LESS_THAN(v0, *p1);
    TEST_ASSERT_LESS_THAN(*p0, *p1);
}

void testAssumeLessThan(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 0;
    v1 = -1;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN(v0, v1);
    TEST_ASSUME_LESS_THAN(*p0, v1);
    TEST_ASSUME_LESS_THAN(v0, *p1);
    TEST_ASSUME_LESS_THAN(*p0, *p1);
}

void testLessThanINT(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 3334;
    v1 = 302;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_INT(v0, v1);
    TEST_ASSERT_LESS_THAN_INT(*p0, v1);
    TEST_ASSERT_LESS_THAN_INT(v0, *p1);
    TEST_ASSERT_LESS_THAN_INT(*p0, *p1);
}

void testAssumeLessThanINT(void)
{
    UNITY_INT v0, v1;
    UNITY_INT *p0, *p1;

    v0 = 3334;
    v1 = 302;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_INT(v0, v1);
    TEST_ASSUME_LESS_THAN_INT(*p0, v1);
    TEST_ASSUME_LESS_THAN_INT(v0, *p1);
    TEST_ASSUME_LESS_THAN_INT(*p0, *p1);
}


void testLessThanINT8(void)
{
    UNITY_INT8 v0, v1;
    UNITY_INT8 *p0, *p1;

    v0 = 127;
    v1 = -128;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_INT8(v0, v1);
    TEST_ASSERT_LESS_THAN_INT8(*p0, v1);
    TEST_ASSERT_LESS_THAN_INT8(v0, *p1);
    TEST_ASSERT_LESS_THAN_INT8(*p0, *p1);
}

void testAssumeLessThanINT8(void)
{
    UNITY_INT8 v0, v1;
    UNITY_INT8 *p0, *p1;

    v0 = 127;
    v1 = -128;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_INT8(v0, v1);
    TEST_ASSUME_LESS_THAN_INT8(*p0, v1);
    TEST_ASSUME_LESS_THAN_INT8(v0, *p1);
    TEST_ASSUME_LESS_THAN_INT8(*p0, *p1);
}

void testLessThanINT16(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = 32767;
    v1 = -32768;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_INT16(v0, v1);
    TEST_ASSERT_LESS_THAN_INT16(*p0, v1);
    TEST_ASSERT_LESS_THAN_INT16(v0, *p1);
    TEST_ASSERT_LESS_THAN_INT16(*p0, *p1);
}

void testAssumeLessThanINT16(void)
{
    UNITY_INT16 v0, v1;
    UNITY_INT16 *p0, *p1;

    v0 = 32767;
    v1 = -32768;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_INT16(v0, v1);
    TEST_ASSUME_LESS_THAN_INT16(*p0, v1);
    TEST_ASSUME_LESS_THAN_INT16(v0, *p1);
    TEST_ASSUME_LESS_THAN_INT16(*p0, *p1);
}

void testLessThanINT32(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = 214783647;
    v1 = -214783648;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_INT32(v0, v1);
    TEST_ASSERT_LESS_THAN_INT32(*p0, v1);
    TEST_ASSERT_LESS_THAN_INT32(v0, *p1);
    TEST_ASSERT_LESS_THAN_INT32(*p0, *p1);
}

void testAssumeLessThanINT32(void)
{
    UNITY_INT32 v0, v1;
    UNITY_INT32 *p0, *p1;

    v0 = 214783647;
    v1 = -214783648;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_INT32(v0, v1);
    TEST_ASSUME_LESS_THAN_INT32(*p0, v1);
    TEST_ASSUME_LESS_THAN_INT32(v0, *p1);
    TEST_ASSUME_LESS_THAN_INT32(*p0, *p1);
}

void testLessThanUINT(void)
{
    UNITY_UINT v0, v1;
    UNITY_UINT *p0, *p1;

    v0 = 1;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_UINT(v0, v1);
    TEST_ASSERT_LESS_THAN_UINT(*p0, v1);
    TEST_ASSERT_LESS_THAN_UINT(v0, *p1);
    TEST_ASSERT_LESS_THAN_UINT(*p0, *p1);
}

void testAssumeLessThanUINT(void)
{
    UNITY_UINT v0, v1;
    UNITY_UINT *p0, *p1;

    v0 = 1;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_UINT(v0, v1);
    TEST_ASSUME_LESS_THAN_UINT(*p0, v1);
    TEST_ASSUME_LESS_THAN_UINT(v0, *p1);
    TEST_ASSUME_LESS_THAN_UINT(*p0, *p1);
}


void testLessThanUINT8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 255;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_UINT8(v0, v1);
    TEST_ASSERT_LESS_THAN_UINT8(*p0, v1);
    TEST_ASSERT_LESS_THAN_UINT8(v0, *p1);
    TEST_ASSERT_LESS_THAN_UINT8(*p0, *p1);
}

void testAssumeLessThanUINT8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 255;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_UINT8(v0, v1);
    TEST_ASSUME_LESS_THAN_UINT8(*p0, v1);
    TEST_ASSUME_LESS_THAN_UINT8(v0, *p1);
    TEST_ASSUME_LESS_THAN_UINT8(*p0, *p1);
}

void testLessThanUINT16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 65535;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_UINT16(v0, v1);
    TEST_ASSERT_LESS_THAN_UINT16(*p0, v1);
    TEST_ASSERT_LESS_THAN_UINT16(v0, *p1);
    TEST_ASSERT_LESS_THAN_UINT16(*p0, *p1);
}

void testAssumeLessThanUINT16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 65535;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_UINT16(v0, v1);
    TEST_ASSUME_LESS_THAN_UINT16(*p0, v1);
    TEST_ASSUME_LESS_THAN_UINT16(v0, *p1);
    TEST_ASSUME_LESS_THAN_UINT16(*p0, *p1);
}

void testLessThanUINT32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 4294967295;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_UINT32(v0, v1);
    TEST_ASSERT_LESS_THAN_UINT32(*p0, v1);
    TEST_ASSERT_LESS_THAN_UINT32(v0, *p1);
    TEST_ASSERT_LESS_THAN_UINT32(*p0, *p1);
}

void testAssumeLessThanUINT32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 4294967295;
    v1 = 0;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_UINT32(v0, v1);
    TEST_ASSUME_LESS_THAN_UINT32(*p0, v1);
    TEST_ASSUME_LESS_THAN_UINT32(v0, *p1);
    TEST_ASSUME_LESS_THAN_UINT32(*p0, *p1);
}

void testLessThanHEX8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0xFF;
    v1 = 0x00;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_HEX8(v0, v1);
    TEST_ASSERT_LESS_THAN_HEX8(*p0, v1);
    TEST_ASSERT_LESS_THAN_HEX8(v0, *p1);
    TEST_ASSERT_LESS_THAN_HEX8(*p0, *p1);
}

void testAssumeLessThanHEX8(void)
{
    UNITY_UINT8 v0, v1;
    UNITY_UINT8 *p0, *p1;

    v0 = 0xFF;
    v1 = 0x00;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_HEX8(v0, v1);
    TEST_ASSUME_LESS_THAN_HEX8(*p0, v1);
    TEST_ASSUME_LESS_THAN_HEX8(v0, *p1);
    TEST_ASSUME_LESS_THAN_HEX8(*p0, *p1);
}

void testLessThanHEX16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0xFFFF;
    v1 = 0x0000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_HEX16(v0, v1);
    TEST_ASSERT_LESS_THAN_HEX16(*p0, v1);
    TEST_ASSERT_LESS_THAN_HEX16(v0, *p1);
    TEST_ASSERT_LESS_THAN_HEX16(*p0, *p1);
}

void testAssumeLessThanHEX16(void)
{
    UNITY_UINT16 v0, v1;
    UNITY_UINT16 *p0, *p1;

    v0 = 0xFFFF;
    v1 = 0x0000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_HEX16(v0, v1);
    TEST_ASSUME_LESS_THAN_HEX16(*p0, v1);
    TEST_ASSUME_LESS_THAN_HEX16(v0, *p1);
    TEST_ASSUME_LESS_THAN_HEX16(*p0, *p1);
}

void testLessThanHEX32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0xFFFFFFFF;
    v1 = 0x00000000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_LESS_THAN_HEX32(v0, v1);
    TEST_ASSERT_LESS_THAN_HEX32(*p0, v1);
    TEST_ASSERT_LESS_THAN_HEX32(v0, *p1);
    TEST_ASSERT_LESS_THAN_HEX32(*p0, *p1);
}

void testAssumeLessThanHEX32(void)
{
    UNITY_UINT32 v0, v1;
    UNITY_UINT32 *p0, *p1;

    v0 = 0xFFFFFFFF;
    v1 = 0x00000000;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_LESS_THAN_HEX32(v0, v1);
    TEST_ASSUME_LESS_THAN_HEX32(*p0, v1);
    TEST_ASSUME_LESS_THAN_HEX32(v0, *p1);
    TEST_ASSUME_LESS_THAN_HEX32(*p0, *p1);
}


void testNotLessThan(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_LESS_THAN(0, 1);
    VERIFY_FAILS_END
}

void testAssumeNotLessThan(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_LESS_THAN(0, 1);
    VERIFY_INCONCLUSIVE_END
}



//-----------------
void testEqualStrings(void)
{
    const char *testString = "foo";

    TEST_ASSERT_EQUAL_STRING(testString, testString);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("foo", "foo", "foo isn't foo");
    TEST_ASSERT_EQUAL_STRING("foo", testString);
    TEST_ASSERT_EQUAL_STRING(testString, "foo");
    TEST_ASSERT_EQUAL_STRING("", "");
}

void testAssumeEqualStrings(void)
{
    const char *testString = "foo";

    TEST_ASSUME_EQUAL_STRING(testString, testString);
    TEST_ASSUME_EQUAL_STRING_MESSAGE("foo", "foo", "foo isn't foo");
    TEST_ASSUME_EQUAL_STRING("foo", testString);
    TEST_ASSUME_EQUAL_STRING(testString, "foo");
    TEST_ASSUME_EQUAL_STRING("", "");
}

void testEqualStringsLen(void)
{
    const char *testString = "foobar";
    TEST_ASSERT_EQUAL_STRING_LEN(testString, testString, strlen(testString));
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE("foobar", "foobaz", 5, "fooba isn't fooba");
    TEST_ASSERT_EQUAL_STRING_LEN("foo", testString, 3);
    TEST_ASSERT_EQUAL_STRING_LEN(testString, "foo", 3);
    TEST_ASSERT_EQUAL_STRING_LEN("", "", 3);
}

void testAssumeEqualStringsLen(void)
{
    const char *testString = "foobar";
    TEST_ASSUME_EQUAL_STRING_LEN(testString, testString, strlen(testString));
    TEST_ASSUME_EQUAL_STRING_LEN_MESSAGE("foobar", "foobaz", 5, "fooba isn't fooba");
    TEST_ASSUME_EQUAL_STRING_LEN("foo", testString, 3);
    TEST_ASSUME_EQUAL_STRING_LEN(testString, "foo", 3);
    TEST_ASSUME_EQUAL_STRING_LEN("", "", 3);
}

void testEqualStringsWithCarriageReturnsAndLineFeeds(void)
{
    const char *testString = "foo\r\nbar";

    TEST_ASSERT_EQUAL_STRING(testString, testString);
    TEST_ASSERT_EQUAL_STRING("foo\r\nbar", "foo\r\nbar");
    TEST_ASSERT_EQUAL_STRING("foo\r\nbar", testString);
    TEST_ASSERT_EQUAL_STRING(testString, "foo\r\nbar");
    TEST_ASSERT_EQUAL_STRING("", "");
}

void testAssumeEqualStringsWithCarriageReturnsAndLineFeeds(void)
{
    const char *testString = "foo\r\nbar";

    TEST_ASSUME_EQUAL_STRING(testString, testString);
    TEST_ASSUME_EQUAL_STRING("foo\r\nbar", "foo\r\nbar");
    TEST_ASSUME_EQUAL_STRING("foo\r\nbar", testString);
    TEST_ASSUME_EQUAL_STRING(testString, "foo\r\nbar");
    TEST_ASSUME_EQUAL_STRING("", "");
}

void testNotEqualString1(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING("foo", "bar");
    VERIFY_FAILS_END
}

void testAssumeNotEqualString1(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING("foo", "bar");
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringLen1(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_LEN("foobar", "foobaz", 6);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringLen1(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_LEN("foobar", "foobaz", 6);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualString2(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING("foo", "");
    VERIFY_FAILS_END
}

void testAssumeNotEqualString2(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING("foo", "");
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringLen2(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_LEN("foo", "", 3);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringLen2(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_LEN("foo", "", 3);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualString3(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING("", "bar");
    VERIFY_FAILS_END
}

void testAssumeNotEqualString3(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING("", "bar");
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringLen3(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_LEN("", "bar", 3);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringLen3(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_LEN("", "bar", 3);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualString4(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING("bar\r", "bar\n");
    VERIFY_FAILS_END
}

void testAssumeNotEqualString4(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING("bar\r", "bar\n");
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringLen4(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_LEN("ba\r\x16", "ba\r\n", 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringLen4(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_LEN("ba\r\x16", "ba\r\n", 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualString5(void)
{
    const char str1[] = { 0x41, 0x42, 0x03, 0x00 };
    const char str2[] = { 0x41, 0x42, 0x04, 0x00 };
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING(str1, str2);
    VERIFY_FAILS_END
}

void testAssumeNotEqualString5(void)
{
    const char str1[] = { 0x41, 0x42, 0x03, 0x00 };
    const char str2[] = { 0x41, 0x42, 0x04, 0x00 };
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING(str1, str2);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualString_ExpectedStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING(NULL, "bar");
    VERIFY_FAILS_END
}

void testAssumeNotEqualString_ExpectedStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING(NULL, "bar");
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringLen_ExpectedStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_LEN(NULL, "bar", 1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringLen_ExpectedStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_LEN(NULL, "bar", 1);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualString_ActualStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING("foo", NULL);
    VERIFY_FAILS_END
}

void testAssumeNotEqualString_ActualStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING("foo", NULL);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringLen_ActualStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_LEN("foo", NULL, 1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringLen_ActualStringIsNull(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_LEN("foo", NULL, 1);
    VERIFY_INCONCLUSIVE_END
}

void testEqualStringArrays(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, expStrings, 3);
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 3);
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 2);
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 1);
}

void testAssumeEqualStringArrays(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, expStrings, 3);
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 3);
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 2);
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 1);
}

void testNotEqualStringArray1(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArray1(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringArray2(void)
{
    const char *testStrings[] = { "zoo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", "boo", "woo", "moo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArray2(void)
{
    const char *testStrings[] = { "zoo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", "boo", "woo", "moo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringArray3(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", NULL };
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArray3(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", NULL };
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringArray4(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", NULL, "woo", "moo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArray4(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "moo" };
    const char *expStrings[] = { "foo", NULL, "woo", "moo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringArray5(void)
{
    const char **testStrings = NULL;
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArray5(void)
{
    const char **testStrings = NULL;
    const char *expStrings[] = { "foo", "boo", "woo", "zoo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringArray6(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "zoo" };
    const char **expStrings = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArray6(void)
{
    const char *testStrings[] = { "foo", "boo", "woo", "zoo" };
    const char **expStrings = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualStringArrayIfBothNulls(void)
{
    const char **testStrings = NULL;
    const char **expStrings = NULL;

    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
}

void testAssumeEqualStringArrayIfBothNulls(void)
{
    const char **testStrings = NULL;
    const char **expStrings = NULL;

    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 4);
}

void testNotEqualStringArrayLengthZero(void)
{
    const char *testStrings[] = {NULL};
    const char **expStrings = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_ARRAY(expStrings, testStrings, 0);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringArrayLengthZero(void)
{
    const char *testStrings[] = {NULL};
    const char **expStrings = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_ARRAY(expStrings, testStrings, 0);
    VERIFY_INCONCLUSIVE_END
}

void testEqualStringEachEqual(void)
{
    const char *testStrings1[] = { "foo", "foo", "foo", "foo" };
    const char *testStrings2[] = { "boo", "boo", "boo", "zoo" };
    const char *testStrings3[] = { "", "", "", "" };

    TEST_ASSERT_EACH_EQUAL_STRING("foo", testStrings1, 4);
    TEST_ASSERT_EACH_EQUAL_STRING("foo", testStrings1, 1);
    TEST_ASSERT_EACH_EQUAL_STRING("boo", testStrings2, 3);
    TEST_ASSERT_EACH_EQUAL_STRING("", testStrings3, 4);
}

void testAssumeEqualStringEachEqual(void)
{
    const char *testStrings1[] = { "foo", "foo", "foo", "foo" };
    const char *testStrings2[] = { "boo", "boo", "boo", "zoo" };
    const char *testStrings3[] = { "", "", "", "" };

    TEST_ASSUME_EACH_EQUAL_STRING("foo", testStrings1, 4);
    TEST_ASSUME_EACH_EQUAL_STRING("foo", testStrings1, 1);
    TEST_ASSUME_EACH_EQUAL_STRING("boo", testStrings2, 3);
    TEST_ASSUME_EACH_EQUAL_STRING("", testStrings3, 4);
}

void testNotEqualStringEachEqual1(void)
{
    const char *testStrings[] = { "foo", "foo", "foo", "moo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringEachEqual1(void)
{
    const char *testStrings[] = { "foo", "foo", "foo", "moo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringEachEqual2(void)
{
    const char *testStrings[] = { "boo", "foo", "foo", "foo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringEachEqual2(void)
{
    const char *testStrings[] = { "boo", "foo", "foo", "foo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringEachEqual3(void)
{
    const char *testStrings[] = { "foo", "foo", "foo", NULL };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringEachEqual3(void)
{
    const char *testStrings[] = { "foo", "foo", "foo", NULL };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringEachEqual4(void)
{
    const char *testStrings[] = { "foo", "foo", "woo", "foo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringEachEqual4(void)
{
    const char *testStrings[] = { "foo", "foo", "woo", "foo" };

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_STRING("foo", testStrings, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualStringEachEqual5(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_STRING("foo", NULL, 1);
    VERIFY_FAILS_END
}

void testAssumeNotEqualStringEachEqual5(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_STRING("foo", NULL, 1);
    VERIFY_INCONCLUSIVE_END
}

void testEqualMemory(void)
{
    const char *testString = "whatever";

    TEST_ASSERT_EQUAL_MEMORY(testString, testString, 8);
    TEST_ASSERT_EQUAL_MEMORY("whatever", "whatever", 8);
    TEST_ASSERT_EQUAL_MEMORY("whatever", testString, 8);
    TEST_ASSERT_EQUAL_MEMORY(testString, "whatever", 8);
    TEST_ASSERT_EQUAL_MEMORY(testString, "whatever", 2);
    TEST_ASSERT_EQUAL_MEMORY(NULL, NULL, 1);
}

void testAssumeEqualMemory(void)
{
    const char *testString = "whatever";

    TEST_ASSUME_EQUAL_MEMORY(testString, testString, 8);
    TEST_ASSUME_EQUAL_MEMORY("whatever", "whatever", 8);
    TEST_ASSUME_EQUAL_MEMORY("whatever", testString, 8);
    TEST_ASSUME_EQUAL_MEMORY(testString, "whatever", 8);
    TEST_ASSUME_EQUAL_MEMORY(testString, "whatever", 2);
    TEST_ASSUME_EQUAL_MEMORY(NULL, NULL, 1);
}

void testNotEqualMemory1(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY("foo", "bar", 3);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemory1(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY("foo", "bar", 3);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemory2(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY("fool", "food", 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemory2(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY("fool", "food", 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemory3(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY(NULL, "food", 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemory3(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY(NULL, "food", 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemory4(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY("fool", NULL, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemory4(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY("fool", NULL, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryLengthZero(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY(NULL, NULL, 0);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryLengthZero(void)
{
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY(NULL, NULL, 0);
    VERIFY_INCONCLUSIVE_END
}

void testEqualIntArrays(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2};
    int p2[] = {1, 8, 987, 2};
    int p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p3, 1);
    TEST_ASSERT_EQUAL_INT_ARRAY(NULL, NULL, 1);
}

void testAssumeEqualIntArrays(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2};
    int p2[] = {1, 8, 987, 2};
    int p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p3, 1);
    TEST_ASSUME_EQUAL_INT_ARRAY(NULL, NULL, 1);
}

void testNotEqualIntArraysNullExpected(void)
{
    int* p0 = NULL;
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntArraysNullExpected(void)
{
    int* p0 = NULL;
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntArraysNullActual(void)
{
    int* p1 = NULL;
    int p0[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntArraysNullActual(void)
{
    int* p1 = NULL;
    int p0[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntArrays1(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntArrays1(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntArrays2(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {2, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntArrays2(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {2, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntArrays3(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 986, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntArrays3(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 986, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntArraysLengthZero(void)
{
    UNITY_UINT32 p0[1] = {1};
    UNITY_UINT32 p1[1] = {1};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_ARRAY(p0, p1, 0);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntArraysLengthZero(void)
{
    UNITY_UINT32 p0[1] = {1};
    UNITY_UINT32 p1[1] = {1};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_ARRAY(p0, p1, 0);
    VERIFY_INCONCLUSIVE_END
}

void testEqualIntEachEqual(void)
{
    int p0[] = {1, 1, 1, 1};
    int p1[] = {987, 987, 987, 987};
    int p2[] = {-2, -2, -2, -3};
    int p3[] = {1, 5, 600, 700};

    TEST_ASSERT_EACH_EQUAL_INT(1, p0, 1);
    TEST_ASSERT_EACH_EQUAL_INT(1, p0, 4);
    TEST_ASSERT_EACH_EQUAL_INT(987, p1, 4);
    TEST_ASSERT_EACH_EQUAL_INT(-2, p2, 3);
    TEST_ASSERT_EACH_EQUAL_INT(1, p3, 1);
}

void testAssumeEqualIntEachEqual(void)
{
    int p0[] = {1, 1, 1, 1};
    int p1[] = {987, 987, 987, 987};
    int p2[] = {-2, -2, -2, -3};
    int p3[] = {1, 5, 600, 700};

    TEST_ASSUME_EACH_EQUAL_INT(1, p0, 1);
    TEST_ASSUME_EACH_EQUAL_INT(1, p0, 4);
    TEST_ASSUME_EACH_EQUAL_INT(987, p1, 4);
    TEST_ASSUME_EACH_EQUAL_INT(-2, p2, 3);
    TEST_ASSUME_EACH_EQUAL_INT(1, p3, 1);
}

void testNotEqualIntEachEqualNullActual(void)
{
    int* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT(1, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntEachEqualNullActual(void)
{
    int* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT(1, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntEachEqual1(void)
{
    int p0[] = {1, 1, 1, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT(1, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntEachEqual1(void)
{
    int p0[] = {1, 1, 1, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT(1, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntEachEqual2(void)
{
    int p0[] = {-5, -5, -1, -5};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT(-5, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntEachEqual2(void)
{
    int p0[] = {-5, -5, -1, -5};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT(-5, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualIntEachEqual3(void)
{
    int p0[] = {1, 88, 88, 88};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT(88, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualIntEachEqual3(void)
{
    int p0[] = {1, 88, 88, 88};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT(88, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualEachEqualLengthZero(void)
{
    UNITY_UINT32 p0[1] = {1};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT(0, p0, 0);
    VERIFY_FAILS_END
}

void testAssumeNotEqualEachEqualLengthZero(void)
{
    UNITY_UINT32 p0[1] = {1};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT(0, p0, 0);
    VERIFY_INCONCLUSIVE_END
}

void testEqualPtrArrays(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &B, &C};
    char* p1[] = {&A, &B, &C, &A};
    char* p2[] = {&A, &B};
    char* p3[] = {&A};

    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p0, 3);
    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p1, 3);
    TEST_ASSERT_EQUAL_PTR_ARRAY(p1, p2, 2);
    TEST_ASSERT_EQUAL_PTR_ARRAY(p3, p0, 1);
}

void testAssumeEqualPtrArrays(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &B, &C};
    char* p1[] = {&A, &B, &C, &A};
    char* p2[] = {&A, &B};
    char* p3[] = {&A};

    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p0, 3);
    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p1, 3);
    TEST_ASSUME_EQUAL_PTR_ARRAY(p1, p2, 2);
    TEST_ASSUME_EQUAL_PTR_ARRAY(p3, p0, 1);
}

void testNotEqualPtrArraysNullExpected(void)
{
    char A = 1;
    char B = 2;
    char** p0 = NULL;
    char* p1[] = {&A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p1, 2);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrArraysNullExpected(void)
{
    char A = 1;
    char B = 2;
    char** p0 = NULL;
    char* p1[] = {&A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p1, 2);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrArraysNullActual(void)
{
    char A = 1;
    char B = 2;
    char** p0 = NULL;
    char* p1[] = {&A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_PTR_ARRAY(p1, p0, 2);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrArraysNullActual(void)
{
    char A = 1;
    char B = 2;
    char** p0 = NULL;
    char* p1[] = {&A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_PTR_ARRAY(p1, p0, 2);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrArrays1(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &B, &C, &B};
    char* p1[] = {&A, &B, &C, &A};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrArrays1(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &B, &C, &B};
    char* p1[] = {&A, &B, &C, &A};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrArrays2(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&B, &B, &C, &A};
    char* p1[] = {&A, &B, &C, &A};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrArrays2(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&B, &B, &C, &A};
    char* p1[] = {&A, &B, &C, &A};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrArrays3(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &B, &B, &A};
    char* p1[] = {&A, &B, &C, &A};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_PTR_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrArrays3(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &B, &B, &A};
    char* p1[] = {&A, &B, &C, &A};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_PTR_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualPtrEachEqual(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &A, &A};
    char* p1[] = {&A, &B, &C, &A};
    char* p2[] = {&B, &B};
    char* p3[] = {&C};

    TEST_ASSERT_EACH_EQUAL_PTR(&A, p0, 1);
    TEST_ASSERT_EACH_EQUAL_PTR(&A, p0, 3);
    TEST_ASSERT_EACH_EQUAL_PTR(&A, p1, 1);
    TEST_ASSERT_EACH_EQUAL_PTR(&B, p2, 2);
    TEST_ASSERT_EACH_EQUAL_PTR(&C, p3, 1);
}

void testAssumeEqualPtrEachEqual(void)
{
    char A = 1;
    char B = 2;
    char C = 3;
    char* p0[] = {&A, &A, &A};
    char* p1[] = {&A, &B, &C, &A};
    char* p2[] = {&B, &B};
    char* p3[] = {&C};

    TEST_ASSUME_EACH_EQUAL_PTR(&A, p0, 1);
    TEST_ASSUME_EACH_EQUAL_PTR(&A, p0, 3);
    TEST_ASSUME_EACH_EQUAL_PTR(&A, p1, 1);
    TEST_ASSUME_EACH_EQUAL_PTR(&B, p2, 2);
    TEST_ASSUME_EACH_EQUAL_PTR(&C, p3, 1);
}

void testNotEqualPtrEachEqualNullExpected(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_PTR(&A, p0, 2);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrEachEqualNullExpected(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_PTR(&A, p0, 2);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrEachEqualNullActual(void)
{
    char A = 1;
    char** p0 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_PTR(&A, p0, 2);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrEachEqualNullActual(void)
{
    char A = 1;
    char** p0 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_PTR(&A, p0, 2);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrEachEqual1(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&A, &A, &A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_PTR(&A, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrEachEqual1(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&A, &A, &A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_PTR(&A, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrEachEqual2(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&B, &B, &A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_PTR(&B, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrEachEqual2(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&B, &B, &A, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_PTR(&B, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualPtrEachEqual3(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&A, &B, &B, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_PTR(&B, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualPtrEachEqual3(void)
{
    char A = 1;
    char B = 1;
    char* p0[] = {&A, &B, &B, &B};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_PTR(&B, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInt8Arrays(void)
{
    UNITY_INT8 p0[] = {1, 8, 117, -2};
    UNITY_INT8 p1[] = {1, 8, 117, -2};
    UNITY_INT8 p2[] = {1, 8, 117, 2};
    UNITY_INT8 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EQUAL_INT8_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_INT8_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_INT8_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_INT8_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_INT8_ARRAY(p0, p3, 1);
}

void testAssumeEqualInt8Arrays(void)
{
    UNITY_INT8 p0[] = {1, 8, 117, -2};
    UNITY_INT8 p1[] = {1, 8, 117, -2};
    UNITY_INT8 p2[] = {1, 8, 117, 2};
    UNITY_INT8 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EQUAL_INT8_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_INT8_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_INT8_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_INT8_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_INT8_ARRAY(p0, p3, 1);
}

void testNotEqualInt8Arrays(void)
{
    UNITY_INT8 p0[] = {1, 8, 36, -2};
    UNITY_INT8 p1[] = {1, 8, 36, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt8Arrays(void)
{
    UNITY_INT8 p0[] = {1, 8, 36, -2};
    UNITY_INT8 p1[] = {1, 8, 36, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInt8EachEqual(void)
{
    UNITY_INT8 p0[] = {1, 1, 1, 1};
    UNITY_INT8 p1[] = {117, 117, 117, -2};
    UNITY_INT8 p2[] = {-1, -1, 117, 2};
    UNITY_INT8 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EACH_EQUAL_INT8(1, p0, 1);
    TEST_ASSERT_EACH_EQUAL_INT8(1, p0, 4);
    TEST_ASSERT_EACH_EQUAL_INT8(117, p1, 3);
    TEST_ASSERT_EACH_EQUAL_INT8(-1, p2, 2);
    TEST_ASSERT_EACH_EQUAL_INT8(1, p3, 1);
}

void testAssumeEqualInt8EachEqual(void)
{
    UNITY_INT8 p0[] = {1, 1, 1, 1};
    UNITY_INT8 p1[] = {117, 117, 117, -2};
    UNITY_INT8 p2[] = {-1, -1, 117, 2};
    UNITY_INT8 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EACH_EQUAL_INT8(1, p0, 1);
    TEST_ASSUME_EACH_EQUAL_INT8(1, p0, 4);
    TEST_ASSUME_EACH_EQUAL_INT8(117, p1, 3);
    TEST_ASSUME_EACH_EQUAL_INT8(-1, p2, 2);
    TEST_ASSUME_EACH_EQUAL_INT8(1, p3, 1);
}

void testNotEqualInt8EachEqual(void)
{
    UNITY_INT8 p0[] = {1, 8, 36, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT8(1, p0, 2);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt8EachEqual(void)
{
    UNITY_INT8 p0[] = {1, 8, 36, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT8(1, p0, 2);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUIntArrays(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {1, 8, 987, 65132u};
    unsigned int p2[] = {1, 8, 987, 2};
    unsigned int p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p3, 1);
}

void testAssumeEqualUIntArrays(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {1, 8, 987, 65132u};
    unsigned int p2[] = {1, 8, 987, 2};
    unsigned int p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p3, 1);
}

void testNotEqualUIntArrays1(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUIntArrays1(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUIntArrays2(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUIntArrays2(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUIntArrays3(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUIntArrays3(void)
{
    unsigned int p0[] = {1, 8, 987, 65132u};
    unsigned int p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUIntEachEqual(void)
{
    unsigned int p0[] = {1, 1, 1, 1};
    unsigned int p1[] = {65132u, 65132u, 65132u, 65132u};
    unsigned int p2[] = {8, 8, 987, 2};
    unsigned int p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EACH_EQUAL_UINT(1, p0, 1);
    TEST_ASSERT_EACH_EQUAL_UINT(1, p0, 4);
    TEST_ASSERT_EACH_EQUAL_UINT(65132u, p1, 4);
    TEST_ASSERT_EACH_EQUAL_UINT(8, p2, 2);
    TEST_ASSERT_EACH_EQUAL_UINT(1, p3, 1);
}

void testAssumeEqualUIntEachEqual(void)
{
    unsigned int p0[] = {1, 1, 1, 1};
    unsigned int p1[] = {65132u, 65132u, 65132u, 65132u};
    unsigned int p2[] = {8, 8, 987, 2};
    unsigned int p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EACH_EQUAL_UINT(1, p0, 1);
    TEST_ASSUME_EACH_EQUAL_UINT(1, p0, 4);
    TEST_ASSUME_EACH_EQUAL_UINT(65132u, p1, 4);
    TEST_ASSUME_EACH_EQUAL_UINT(8, p2, 2);
    TEST_ASSUME_EACH_EQUAL_UINT(1, p3, 1);
}

void testNotEqualUIntEachEqual1(void)
{
    unsigned int p0[] = {1, 65132u, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUIntEachEqual1(void)
{
    unsigned int p0[] = {1, 65132u, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUIntEachEqual2(void)
{
    unsigned int p0[] = {987, 8, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT(987, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUIntEachEqual2(void)
{
    unsigned int p0[] = {987, 8, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT(987, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUIntEachEqual3(void)
{
    unsigned int p0[] = {1, 1, 1, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT(1, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUIntEachEqual3(void)
{
    unsigned int p0[] = {1, 1, 1, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT(1, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInt16Arrays(void)
{
    UNITY_INT16 p0[] = {1, 8, 117, 3};
    UNITY_INT16 p1[] = {1, 8, 117, 3};
    UNITY_INT16 p2[] = {1, 8, 117, 2};
    UNITY_INT16 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EQUAL_INT16_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_INT16_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_INT16_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_INT16_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_INT16_ARRAY(p0, p3, 1);
}

void testAssumeEqualInt16Arrays(void)
{
    UNITY_INT16 p0[] = {1, 8, 117, 3};
    UNITY_INT16 p1[] = {1, 8, 117, 3};
    UNITY_INT16 p2[] = {1, 8, 117, 2};
    UNITY_INT16 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EQUAL_INT16_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_INT16_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_INT16_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_INT16_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_INT16_ARRAY(p0, p3, 1);
}

void testNotEqualInt16Arrays(void)
{
    UNITY_INT16 p0[] = {1, 8, 127, 3};
    UNITY_INT16 p1[] = {1, 8, 127, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt16Arrays(void)
{
    UNITY_INT16 p0[] = {1, 8, 127, 3};
    UNITY_INT16 p1[] = {1, 8, 127, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInt16EachEqual(void)
{
    UNITY_INT16 p0[] = {1, 1, 1, 1};
    UNITY_INT16 p1[] = {32111, 32111, 32111, 3};
    UNITY_INT16 p2[] = {-1, -1, -1, 2};
    UNITY_INT16 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EACH_EQUAL_INT16(1, p0, 1);
    TEST_ASSERT_EACH_EQUAL_INT16(1, p0, 4);
    TEST_ASSERT_EACH_EQUAL_INT16(32111, p1, 3);
    TEST_ASSERT_EACH_EQUAL_INT16(-1, p2, 3);
    TEST_ASSERT_EACH_EQUAL_INT16(1, p3, 1);
}

void testAssumeEqualInt16EachEqual(void)
{
    UNITY_INT16 p0[] = {1, 1, 1, 1};
    UNITY_INT16 p1[] = {32111, 32111, 32111, 3};
    UNITY_INT16 p2[] = {-1, -1, -1, 2};
    UNITY_INT16 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EACH_EQUAL_INT16(1, p0, 1);
    TEST_ASSUME_EACH_EQUAL_INT16(1, p0, 4);
    TEST_ASSUME_EACH_EQUAL_INT16(32111, p1, 3);
    TEST_ASSUME_EACH_EQUAL_INT16(-1, p2, 3);
    TEST_ASSUME_EACH_EQUAL_INT16(1, p3, 1);
}

void testNotEqualInt16EachEqual(void)
{
    UNITY_INT16 p0[] = {127, 127, 127, 3};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT16(127, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt16EachEqual(void)
{
    UNITY_INT16 p0[] = {127, 127, 127, 3};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT16(127, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInt32Arrays(void)
{
    UNITY_INT32 p0[] = {1, 8, 117, 3};
    UNITY_INT32 p1[] = {1, 8, 117, 3};
    UNITY_INT32 p2[] = {1, 8, 117, 2};
    UNITY_INT32 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EQUAL_INT32_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_INT32_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_INT32_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_INT32_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_INT32_ARRAY(p0, p3, 1);
}

void testAssumeEqualInt32Arrays(void)
{
    UNITY_INT32 p0[] = {1, 8, 117, 3};
    UNITY_INT32 p1[] = {1, 8, 117, 3};
    UNITY_INT32 p2[] = {1, 8, 117, 2};
    UNITY_INT32 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EQUAL_INT32_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_INT32_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_INT32_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_INT32_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_INT32_ARRAY(p0, p3, 1);
}

void testNotEqualInt32Arrays(void)
{
    UNITY_INT32 p0[] = {1, 8, 127, 3};
    UNITY_INT32 p1[] = {1, 8, 127, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt32Arrays(void)
{
    UNITY_INT32 p0[] = {1, 8, 127, 3};
    UNITY_INT32 p1[] = {1, 8, 127, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualInt32EachEqual(void)
{
    UNITY_INT32 p0[] = {8, 8, 8, 8};
    UNITY_INT32 p1[] = {65537, 65537, 65537, 65537};
    UNITY_INT32 p2[] = {-3, -3, -3, 2};
    UNITY_INT32 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EACH_EQUAL_INT32(8, p0, 1);
    TEST_ASSERT_EACH_EQUAL_INT32(8, p0, 4);
    TEST_ASSERT_EACH_EQUAL_INT32(65537, p1, 4);
    TEST_ASSERT_EACH_EQUAL_INT32(-3, p2, 3);
    TEST_ASSERT_EACH_EQUAL_INT32(1, p3, 1);
}

void testAssumeEqualInt32EachEqual(void)
{
    UNITY_INT32 p0[] = {8, 8, 8, 8};
    UNITY_INT32 p1[] = {65537, 65537, 65537, 65537};
    UNITY_INT32 p2[] = {-3, -3, -3, 2};
    UNITY_INT32 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EACH_EQUAL_INT32(8, p0, 1);
    TEST_ASSUME_EACH_EQUAL_INT32(8, p0, 4);
    TEST_ASSUME_EACH_EQUAL_INT32(65537, p1, 4);
    TEST_ASSUME_EACH_EQUAL_INT32(-3, p2, 3);
    TEST_ASSUME_EACH_EQUAL_INT32(1, p3, 1);
}

void testNotEqualInt32EachEqual(void)
{
    UNITY_INT32 p0[] = {127, 8, 127, 127};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_INT32(127, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualInt32EachEqual(void)
{
    UNITY_INT32 p0[] = {127, 8, 127, 127};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_INT32(127, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUINT8Arrays(void)
{
    UNITY_UINT8 p0[] = {1, 8, 100, 127};
    UNITY_UINT8 p1[] = {1, 8, 100, 127};
    UNITY_UINT8 p2[] = {1, 8, 100, 2};
    UNITY_UINT8 p3[] = {1, 50, 60, 70};

    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p3, 1);
}

void testAssumeEqualUINT8Arrays(void)
{
    UNITY_UINT8 p0[] = {1, 8, 100, 127};
    UNITY_UINT8 p1[] = {1, 8, 100, 127};
    UNITY_UINT8 p2[] = {1, 8, 100, 2};
    UNITY_UINT8 p3[] = {1, 50, 60, 70};

    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p3, 1);
}

void testNotEqualUINT8Arrays1(void)
{
    unsigned char p0[] = {1, 8, 100, 127u};
    unsigned char p1[] = {1, 8, 100, 255u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT8Arrays1(void)
{
    unsigned char p0[] = {1, 8, 100, 127u};
    unsigned char p1[] = {1, 8, 100, 255u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT8Arrays2(void)
{
    unsigned char p0[] = {1, 8, 100, 127u};
    unsigned char p1[] = {1, 8, 100, 255u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT8Arrays2(void)
{
    unsigned char p0[] = {1, 8, 100, 127u};
    unsigned char p1[] = {1, 8, 100, 255u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT8Arrays3(void)
{
    unsigned char p0[] = {1, 8, 100, 127u};
    unsigned char p1[] = {1, 8, 100, 255u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT8Arrays3(void)
{
    unsigned char p0[] = {1, 8, 100, 127u};
    unsigned char p1[] = {1, 8, 100, 255u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}


void testEqualUINT16Arrays(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65132u};
    unsigned short p2[] = {1, 8, 987, 2};
    unsigned short p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p3, 1);
}

void testAssumeEqualUINT16Arrays(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65132u};
    unsigned short p2[] = {1, 8, 987, 2};
    unsigned short p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p3, 1);
}

void testNotEqualUINT16Arrays1(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT16Arrays1(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT16Arrays2(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT16Arrays2(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT16Arrays3(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT16Arrays3(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUINT32Arrays(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p2[] = {1, 8, 987, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p3, 1);
}

void testAssumeEqualUINT32Arrays(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p2[] = {1, 8, 987, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p3, 1);
}

void testNotEqualUINT32Arrays1(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT32Arrays1(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT32Arrays2(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT32Arrays2(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT32Arrays3(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT32Arrays3(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEXArrays(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p2[] = {1, 8, 987, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_HEX_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_HEX_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_HEX_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p3, 1);
}

void testAssumeEqualHEXArrays(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p2[] = {1, 8, 987, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_HEX_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_HEX_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_HEX_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p3, 1);
}

void testNotEqualHEXArrays1(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEXArrays1(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEXArrays2(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEXArrays2(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEXArrays3(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEXArrays3(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEX32Arrays(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p2[] = {1, 8, 987, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p3, 1);
}

void testAssumeEqualHEX32Arrays(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p2[] = {1, 8, 987, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p3, 1);
}

void testNotEqualHEX32Arrays1(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX32Arrays1(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX32Arrays2(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX32Arrays2(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX32Arrays3(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX32Arrays3(void)
{
    UNITY_UINT32 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT32 p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX32_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEX16Arrays(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65132u};
    unsigned short p2[] = {1, 8, 987, 2};
    unsigned short p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p3, 1);
}

void testAssumeEqualHEX16Arrays(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65132u};
    unsigned short p2[] = {1, 8, 987, 2};
    unsigned short p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p3, 1);
}

void testNotEqualHEX16Arrays1(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX16Arrays1(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX16Arrays2(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX16Arrays2(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX16Arrays3(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX16_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX16Arrays3(void)
{
    unsigned short p0[] = {1, 8, 987, 65132u};
    unsigned short p1[] = {1, 8, 986, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX16_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEX8Arrays(void)
{
    unsigned char p0[] = {1, 8, 254u, 123};
    unsigned char p1[] = {1, 8, 254u, 123};
    unsigned char p2[] = {1, 8, 254u, 2};
    unsigned char p3[] = {1, 23, 25, 26};

    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p3, 1);
}

void testAssumeEqualHEX8Arrays(void)
{
    unsigned char p0[] = {1, 8, 254u, 123};
    unsigned char p1[] = {1, 8, 254u, 123};
    unsigned char p2[] = {1, 8, 254u, 2};
    unsigned char p3[] = {1, 23, 25, 26};

    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p3, 1);
}

void testNotEqualHEX8Arrays1(void)
{
    unsigned char p0[] = {1, 8, 254u, 253u};
    unsigned char p1[] = {1, 8, 254u, 252u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX8Arrays1(void)
{
    unsigned char p0[] = {1, 8, 254u, 253u};
    unsigned char p1[] = {1, 8, 254u, 252u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX8Arrays2(void)
{
    unsigned char p0[] = {1, 8, 254u, 253u};
    unsigned char p1[] = {2, 8, 254u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX8Arrays2(void)
{
    unsigned char p0[] = {1, 8, 254u, 253u};
    unsigned char p1[] = {2, 8, 254u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX8Arrays3(void)
{
    unsigned char p0[] = {1, 8, 254u, 253u};
    unsigned char p1[] = {1, 8, 255u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX8Arrays3(void)
{
    unsigned char p0[] = {1, 8, 254u, 253u};
    unsigned char p1[] = {1, 8, 255u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX8_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUINT8EachEqual(void)
{
    UNITY_UINT8 p0[] = {127u, 127u, 127u, 127u};
    UNITY_UINT8 p1[] = {1u, 1u, 1u, 1u};
    UNITY_UINT8 p2[] = {128u, 128u, 128u, 2u};
    UNITY_UINT8 p3[] = {1u, 50u, 60u, 70u};

    TEST_ASSERT_EACH_EQUAL_UINT8(127u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_UINT8(127u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_UINT8(1u, p1, 4);
    TEST_ASSERT_EACH_EQUAL_UINT8(128u, p2, 3);
    TEST_ASSERT_EACH_EQUAL_UINT8(1u, p3, 1);
}

void testAssumeEqualUINT8EachEqual(void)
{
    UNITY_UINT8 p0[] = {127u, 127u, 127u, 127u};
    UNITY_UINT8 p1[] = {1u, 1u, 1u, 1u};
    UNITY_UINT8 p2[] = {128u, 128u, 128u, 2u};
    UNITY_UINT8 p3[] = {1u, 50u, 60u, 70u};

    TEST_ASSUME_EACH_EQUAL_UINT8(127u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_UINT8(127u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_UINT8(1u, p1, 4);
    TEST_ASSUME_EACH_EQUAL_UINT8(128u, p2, 3);
    TEST_ASSUME_EACH_EQUAL_UINT8(1u, p3, 1);
}

void testNotEqualUINT8EachEqual1(void)
{
    unsigned char p0[] = {127u, 127u, 128u, 127u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT8(127u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT8EachEqual1(void)
{
    unsigned char p0[] = {127u, 127u, 128u, 127u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT8(127u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT8EachEqual2(void)
{
    unsigned char p0[] = {1, 1, 1, 127u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT8(1, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT8EachEqual2(void)
{
    unsigned char p0[] = {1, 1, 1, 127u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT8(1, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT8EachEqual3(void)
{
    unsigned char p0[] = {54u, 55u, 55u, 55u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT8(55u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT8EachEqual3(void)
{
    unsigned char p0[] = {54u, 55u, 55u, 55u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT8(55u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUINT16EachEqual(void)
{
    unsigned short p0[] = {65132u, 65132u, 65132u, 65132u};
    unsigned short p1[] = {987, 987, 987, 987};
    unsigned short p2[] = {1, 1, 1, 2};
    unsigned short p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EACH_EQUAL_UINT16(65132u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_UINT16(65132u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_UINT16(987, p1, 4);
    TEST_ASSERT_EACH_EQUAL_UINT16(1, p2, 3);
    TEST_ASSERT_EACH_EQUAL_UINT16(1, p3, 1);
}

void testAssumeEqualUINT16EachEqual(void)
{
    unsigned short p0[] = {65132u, 65132u, 65132u, 65132u};
    unsigned short p1[] = {987, 987, 987, 987};
    unsigned short p2[] = {1, 1, 1, 2};
    unsigned short p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EACH_EQUAL_UINT16(65132u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_UINT16(65132u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_UINT16(987, p1, 4);
    TEST_ASSUME_EACH_EQUAL_UINT16(1, p2, 3);
    TEST_ASSUME_EACH_EQUAL_UINT16(1, p3, 1);
}

void testNotEqualUINT16EachEqual1(void)
{
    unsigned short p0[] = {1, 65132u, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT16(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT16EachEqual1(void)
{
    unsigned short p0[] = {1, 65132u, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT16(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT16EachEqual2(void)
{
    unsigned short p0[] = {65132u, 65132u, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT16(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT16EachEqual2(void)
{
    unsigned short p0[] = {65132u, 65132u, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT16(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT16EachEqual3(void)
{
    unsigned short p0[] = {65132u, 65132u, 65132u, 65133u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT16(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT16EachEqual3(void)
{
    unsigned short p0[] = {65132u, 65132u, 65132u, 65133u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT16(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualUINT32EachEqual(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT32 p1[] = {987, 987, 987, 987};
    UNITY_UINT32 p2[] = {8, 8, 8, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EACH_EQUAL_UINT32(65132u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_UINT32(65132u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_UINT32(987, p1, 4);
    TEST_ASSERT_EACH_EQUAL_UINT32(8, p2, 3);
    TEST_ASSERT_EACH_EQUAL_UINT32(1, p3, 1);
}

void testAssumeEqualUINT32EachEqual(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT32 p1[] = {987, 987, 987, 987};
    UNITY_UINT32 p2[] = {8, 8, 8, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EACH_EQUAL_UINT32(65132u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_UINT32(65132u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_UINT32(987, p1, 4);
    TEST_ASSUME_EACH_EQUAL_UINT32(8, p2, 3);
    TEST_ASSUME_EACH_EQUAL_UINT32(1, p3, 1);
}

void testNotEqualUINT32EachEqual1(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT32(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT32EachEqual1(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT32(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT32EachEqual2(void)
{
    UNITY_UINT32 p0[] = {1, 987, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT32(987, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT32EachEqual2(void)
{
    UNITY_UINT32 p0[] = {1, 987, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT32(987, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualUINT32EachEqual3(void)
{
    UNITY_UINT32 p0[] = {1, 1, 1, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_UINT32(1, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualUINT32EachEqual3(void)
{
    UNITY_UINT32 p0[] = {1, 1, 1, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_UINT32(1, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEXEachEqual(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT32 p1[] = {987, 987, 987, 987};
    UNITY_UINT32 p2[] = {8, 8, 8, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EACH_EQUAL_HEX(65132u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_HEX(65132u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_HEX(987, p1, 4);
    TEST_ASSERT_EACH_EQUAL_HEX(8, p2, 3);
    TEST_ASSERT_EACH_EQUAL_HEX(1, p3, 1);
}

void testAssumeEqualHEXEachEqual(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT32 p1[] = {987, 987, 987, 987};
    UNITY_UINT32 p2[] = {8, 8, 8, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EACH_EQUAL_HEX(65132u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_HEX(65132u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_HEX(987, p1, 4);
    TEST_ASSUME_EACH_EQUAL_HEX(8, p2, 3);
    TEST_ASSUME_EACH_EQUAL_HEX(1, p3, 1);
}

void testNotEqualHEXEachEqual1(void)
{
    UNITY_UINT32 p0[] = {1, 65132u, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX32(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEXEachEqual1(void)
{
    UNITY_UINT32 p0[] = {1, 65132u, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX32(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEXEachEqual2(void)
{
    UNITY_UINT32 p0[] = {987, 987, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX32(987, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEXEachEqual2(void)
{
    UNITY_UINT32 p0[] = {987, 987, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX32(987, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEXEachEqual3(void)
{
    UNITY_UINT32 p0[] = {8, 8, 987, 8};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX(8, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEXEachEqual3(void)
{
    UNITY_UINT32 p0[] = {8, 8, 987, 8};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX(8, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEX32EachEqual(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT32 p1[] = {987, 987, 987, 987};
    UNITY_UINT32 p2[] = {8, 8, 8, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EACH_EQUAL_HEX32(65132u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_HEX32(65132u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_HEX32(987, p1, 4);
    TEST_ASSERT_EACH_EQUAL_HEX32(8, p2, 3);
    TEST_ASSERT_EACH_EQUAL_HEX32(1, p3, 1);
}

void testAssumeEqualHEX32EachEqual(void)
{
    UNITY_UINT32 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT32 p1[] = {987, 987, 987, 987};
    UNITY_UINT32 p2[] = {8, 8, 8, 2};
    UNITY_UINT32 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EACH_EQUAL_HEX32(65132u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_HEX32(65132u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_HEX32(987, p1, 4);
    TEST_ASSUME_EACH_EQUAL_HEX32(8, p2, 3);
    TEST_ASSUME_EACH_EQUAL_HEX32(1, p3, 1);
}

void testNotEqualHEX32EachEqual1(void)
{
    UNITY_UINT32 p0[] = {65132u, 8, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX32(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX32EachEqual1(void)
{
    UNITY_UINT32 p0[] = {65132u, 8, 65132u, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX32(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX32EachEqual2(void)
{
    UNITY_UINT32 p0[] = {1, 987, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX32(987, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX32EachEqual2(void)
{
    UNITY_UINT32 p0[] = {1, 987, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX32(987, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX32EachEqual3(void)
{
    UNITY_UINT32 p0[] = {8, 8, 8, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX32(8, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX32EachEqual3(void)
{
    UNITY_UINT32 p0[] = {8, 8, 8, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX32(8, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEX16EachEqual(void)
{
    UNITY_UINT16 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT16 p1[] = {987, 987, 987, 987};
    UNITY_UINT16 p2[] = {8, 8, 8, 2};
    UNITY_UINT16 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EACH_EQUAL_HEX16(65132u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_HEX16(65132u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_HEX16(987, p1, 4);
    TEST_ASSERT_EACH_EQUAL_HEX16(8, p2, 3);
    TEST_ASSERT_EACH_EQUAL_HEX16(1, p3, 1);
}

void testAssumeEqualHEX16EachEqual(void)
{
    UNITY_UINT16 p0[] = {65132u, 65132u, 65132u, 65132u};
    UNITY_UINT16 p1[] = {987, 987, 987, 987};
    UNITY_UINT16 p2[] = {8, 8, 8, 2};
    UNITY_UINT16 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EACH_EQUAL_HEX16(65132u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_HEX16(65132u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_HEX16(987, p1, 4);
    TEST_ASSUME_EACH_EQUAL_HEX16(8, p2, 3);
    TEST_ASSUME_EACH_EQUAL_HEX16(1, p3, 1);
}

void testNotEqualHEX16EachEqual1(void)
{
    unsigned short p0[] = {65132u, 65132u, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX16(65132u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX16EachEqual1(void)
{
    unsigned short p0[] = {65132u, 65132u, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX16(65132u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX16EachEqual2(void)
{
    unsigned short p0[] = {1, 987, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX16(987, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX16EachEqual2(void)
{
    unsigned short p0[] = {1, 987, 987, 987};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX16(987, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX16EachEqual3(void)
{
    unsigned short p0[] = {8, 8, 8, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX16(8, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX16EachEqual3(void)
{
    unsigned short p0[] = {8, 8, 8, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX16(8, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualHEX8EachEqual(void)
{
    unsigned char p0[] = {254u, 254u, 254u, 254u};
    unsigned char p1[] = {123, 123, 123, 123};
    unsigned char p2[] = {8, 8, 8, 2};
    unsigned char p3[] = {1, 23, 25, 26};

    TEST_ASSERT_EACH_EQUAL_HEX8(254u, p0, 1);
    TEST_ASSERT_EACH_EQUAL_HEX8(254u, p0, 4);
    TEST_ASSERT_EACH_EQUAL_HEX8(123, p1, 4);
    TEST_ASSERT_EACH_EQUAL_HEX8(8, p2, 3);
    TEST_ASSERT_EACH_EQUAL_HEX8(1, p3, 1);
}

void testAssumeEqualHEX8EachEqual(void)
{
    unsigned char p0[] = {254u, 254u, 254u, 254u};
    unsigned char p1[] = {123, 123, 123, 123};
    unsigned char p2[] = {8, 8, 8, 2};
    unsigned char p3[] = {1, 23, 25, 26};

    TEST_ASSUME_EACH_EQUAL_HEX8(254u, p0, 1);
    TEST_ASSUME_EACH_EQUAL_HEX8(254u, p0, 4);
    TEST_ASSUME_EACH_EQUAL_HEX8(123, p1, 4);
    TEST_ASSUME_EACH_EQUAL_HEX8(8, p2, 3);
    TEST_ASSUME_EACH_EQUAL_HEX8(1, p3, 1);
}

void testNotEqualHEX8EachEqual1(void)
{
    unsigned char p0[] = {253u, 253u, 254u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX8(253u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX8EachEqual1(void)
{
    unsigned char p0[] = {253u, 253u, 254u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX8(253u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX8EachEqual2(void)
{
    unsigned char p0[] = {254u, 254u, 254u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX8(254u, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX8EachEqual2(void)
{
    unsigned char p0[] = {254u, 254u, 254u, 253u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX8(254u, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualHEX8EachEqual3(void)
{
    unsigned char p0[] = {1, 8, 8, 8};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_HEX8(8, p0, 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualHEX8EachEqual3(void)
{
    unsigned char p0[] = {1, 8, 8, 8};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_HEX8(8, p0, 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualMemoryArrays(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2};
    int p2[] = {1, 8, 987, 2};
    int p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p0, sizeof(int), 1);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p0, sizeof(int), 4);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p2, sizeof(int), 3);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p3, sizeof(int), 1);
}

void testAssumeEqualMemoryArrays(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2};
    int p2[] = {1, 8, 987, 2};
    int p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p0, sizeof(int), 1);
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p0, sizeof(int), 4);
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p2, sizeof(int), 3);
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p3, sizeof(int), 1);
}

void testNotEqualMemoryArraysExpectedNull(void)
{
    int* p0 = NULL;
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryArraysExpectedNull(void)
{
    int* p0 = NULL;
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryArraysActualNull(void)
{
    int p0[] = {1, 8, 987, -2};
    int* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryArraysActualNull(void)
{
    int p0[] = {1, 8, 987, -2};
    int* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryArrays1(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryArrays1(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryArrays2(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {2, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryArrays2(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {2, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryArrays3(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 986, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryArrays3(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 986, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_MEMORY_ARRAY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testEqualMemoryEachEqual(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2, 1, 8, 987, -2};
    int p2[] = {8, 8, 8, 2};
    int p3[] = {8, 500, 600, 700};
    int v = 8;

    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p0, sizeof(int)*4, 1);
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 2);
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 1);
    TEST_ASSERT_EACH_EQUAL_MEMORY(&v, p2, sizeof(int), 3);
    TEST_ASSERT_EACH_EQUAL_MEMORY(&v, p3, sizeof(int), 1);
}

void testAssumeEqualMemoryEachEqual(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2, 1, 8, 987, -2};
    int p2[] = {8, 8, 8, 2};
    int p3[] = {8, 500, 600, 700};
    int v = 8;

    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p0, sizeof(int)*4, 1);
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 2);
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 1);
    TEST_ASSUME_EACH_EQUAL_MEMORY(&v, p2, sizeof(int), 3);
    TEST_ASSUME_EACH_EQUAL_MEMORY(&v, p3, sizeof(int), 1);
}

void testNotEqualMemoryEachEqualExpectedNull(void)
{
    int* p0 = NULL;
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryEachEqualExpectedNull(void)
{
    int* p0 = NULL;
    int p1[] = {1, 8, 987, 2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryEachEqualActualNull(void)
{
    int p0[] = {1, 8, 987, -2};
    int* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int), 4);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryEachEqualActualNull(void)
{
    int p0[] = {1, 8, 987, -2};
    int* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int), 4);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryEachEqual1(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {9, 8, 987, -2, 1, 8, 987, -2, 1, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 3);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryEachEqual1(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {9, 8, 987, -2, 1, 8, 987, -2, 1, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 3);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryEachEqual2(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2, 1, 8, 987, -2, 1, 8, 987, 9};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 3);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryEachEqual2(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2, 1, 8, 987, -2, 1, 8, 987, 9};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 3);
    VERIFY_INCONCLUSIVE_END
}

void testNotEqualMemoryEachEqual3(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2, 1, 9, 987, -2, 1, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 3);
    VERIFY_FAILS_END
}

void testAssumeNotEqualMemoryEachEqual3(void)
{
    int p0[] = {1, 8, 987, -2};
    int p1[] = {1, 8, 987, -2, 1, 9, 987, -2, 1, 8, 987, -2};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_MEMORY(p0, p1, sizeof(int)*4, 3);
    VERIFY_INCONCLUSIVE_END
}

void testProtection(void)
{
    volatile int mask = 0;

    if (TEST_PROTECT())
    {
        mask |= 1;
        TEST_ABORT();
    }
    else
    {
        Unity.CurrentTestFailed = 0;
        mask |= 2;
    }

    TEST_ASSERT_EQUAL(3, mask);
}

void testAssumeProtection(void)
{
    volatile int mask = 0;

    if (TEST_PROTECT())
    {
        mask |= 1;
        TEST_ABORT();
    }
    else
    {
        Unity.CurrentTestFailed = 0;
        mask |= 2;
    }

    TEST_ASSUME_EQUAL(3, mask);
}

void testIgnoredAndThenFailInTearDown(void)
{
    SetToOneToFailInTearDown = 1;
    TEST_IGNORE();
}

void testAssumeIgnoredAndThenFailInTearDown(void)
{
    SetToOneToFailInTearDown = 1;
    TEST_IGNORE();
}

// Tricky series of macros to set USING_OUTPUT_SPY
#define USING_SPY_AS(a)           EXPAND_AND_USE_2ND(ASSIGN_VALUE(a), 0)
#define ASSIGN_VALUE(a)           VAL_##a
#define VAL_putcharSpy            0, 1
#define EXPAND_AND_USE_2ND(a, b)  SECOND_PARAM(a, b, throwaway)
#define SECOND_PARAM(a, b, ...)   b
#if USING_SPY_AS(UNITY_OUTPUT_CHAR)
  #define USING_OUTPUT_SPY // true only if UNITY_OUTPUT_CHAR = putcharSpy
#endif

#ifdef USING_OUTPUT_SPY
#include <stdio.h>
#define SPY_BUFFER_MAX 40
static char putcharSpyBuffer[SPY_BUFFER_MAX];
#endif
static int indexSpyBuffer;
static int putcharSpyEnabled;

void startPutcharSpy(void) {indexSpyBuffer = 0; putcharSpyEnabled = 1;}

void endPutcharSpy(void) {putcharSpyEnabled = 0;}

char* getBufferPutcharSpy(void)
{
#ifdef USING_OUTPUT_SPY
    putcharSpyBuffer[indexSpyBuffer] = '\0';
    return putcharSpyBuffer;
#else
    return NULL;
#endif
}

void putcharSpy(int c)
{
#ifdef USING_OUTPUT_SPY
    if (putcharSpyEnabled)
    {
        if (indexSpyBuffer < SPY_BUFFER_MAX - 1)
            putcharSpyBuffer[indexSpyBuffer++] = (char)c;
    } else
        putchar((char)c);
#endif
}

/* This is for counting the calls to the flushSpy */
static int flushSpyEnabled;
static int flushSpyCalls = 0;

void startFlushSpy(void) { flushSpyCalls = 0; flushSpyEnabled = 1; }
void endFlushSpy(void) { flushSpyCalls = 0; flushSpyEnabled = 0; }
int getFlushSpyCalls(void) { return flushSpyCalls; }

void flushSpy(void)
{
    if (flushSpyEnabled){ flushSpyCalls++; }
}

void testFailureCountIncrementsAndIsReturnedAtEnd(void)
{
    UNITY_UINT savedFailures = Unity.TestFailures;
    Unity.CurrentTestFailed = 1;
    startPutcharSpy(); // Suppress output
    startFlushSpy();
    TEST_ASSERT_EQUAL(0, getFlushSpyCalls());
    UnityConcludeTest();
    endPutcharSpy();
    TEST_ASSERT_EQUAL(savedFailures + 1, Unity.TestFailures);
#if defined(UNITY_OUTPUT_FLUSH) && defined(UNITY_OUTPUT_FLUSH_HEADER_DECLARATION)
    TEST_ASSERT_EQUAL(1, getFlushSpyCalls());
#else
    TEST_ASSERT_EQUAL(0, getFlushSpyCalls());
#endif
    endFlushSpy();

    startPutcharSpy(); // Suppress output
    int failures = UnityEnd();
    Unity.TestFailures--;
    endPutcharSpy();
    TEST_ASSERT_EQUAL(savedFailures + 1, failures);
}

void testAssumeFailureCountIncrementsAndIsReturnedAtEnd(void)
{
    UNITY_UINT savedFailures = Unity.TestFailures;
    Unity.CurrentTestFailed = 1;
    startPutcharSpy(); // Suppress output
    startFlushSpy();
    TEST_ASSUME_EQUAL(0, getFlushSpyCalls());
    UnityConcludeTest();
    endPutcharSpy();
    TEST_ASSUME_EQUAL(savedFailures + 1, Unity.TestFailures);
#if defined(UNITY_OUTPUT_FLUSH) && defined(UNITY_OUTPUT_FLUSH_HEADER_DECLARATION)
    TEST_ASSUME_EQUAL(1, getFlushSpyCalls());
#else
    TEST_ASSUME_EQUAL(0, getFlushSpyCalls());
#endif
    endFlushSpy();

    startPutcharSpy(); // Suppress output
    int failures = UnityEnd();
    Unity.TestFailures--;
    endPutcharSpy();
    TEST_ASSUME_EQUAL(savedFailures + 1, failures);
}

void testCstringsEscapeSequence(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    startPutcharSpy();
    UnityPrint("\x16\x10");
    endPutcharSpy();
    TEST_ASSERT_EQUAL_STRING("\\x16\\x10", getBufferPutcharSpy());
#endif
}

void testAssumeCstringsEscapeSequence(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    startPutcharSpy();
    UnityPrint("\x16\x10");
    endPutcharSpy();
    TEST_ASSUME_EQUAL_STRING("\\x16\\x10", getBufferPutcharSpy());
#endif
}

void testHexPrintsUpToMaxNumberOfNibbles(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    startPutcharSpy();
    UnityPrintNumberHex(0xBEE, 21);
    endPutcharSpy();
    TEST_ASSERT_EQUAL_INT(sizeof(UNITY_INT)*2, strlen(getBufferPutcharSpy()));
#endif
}

void testAssumeHexPrintsUpToMaxNumberOfNibbles(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    startPutcharSpy();
    UnityPrintNumberHex(0xBEE, 21);
    endPutcharSpy();
    TEST_ASSUME_EQUAL_INT(sizeof(UNITY_INT)*2, strlen(getBufferPutcharSpy()));
#endif
}

#define TEST_ASSERT_EQUAL_PRINT_NUMBERS(expected, actual) {             \
        startPutcharSpy(); UnityPrintNumber((actual)); endPutcharSpy(); \
        TEST_ASSERT_EQUAL_STRING((expected), getBufferPutcharSpy());    \
        }

#define TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS(expected, actual) {            \
        startPutcharSpy(); UnityPrintNumberUnsigned((actual)); endPutcharSpy(); \
        TEST_ASSERT_EQUAL_STRING((expected), getBufferPutcharSpy());            \
        }

#define TEST_ASSUME_EQUAL_PRINT_NUMBERS(expected, actual) {             \
        startPutcharSpy(); UnityPrintNumber((actual)); endPutcharSpy(); \
        TEST_ASSUME_EQUAL_STRING((expected), getBufferPutcharSpy());    \
        }

#define TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS(expected, actual) {            \
        startPutcharSpy(); UnityPrintNumberUnsigned((actual)); endPutcharSpy(); \
        TEST_ASSUME_EQUAL_STRING((expected), getBufferPutcharSpy());            \
        }

void testPrintNumbers32(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE_MESSAGE("Compile with '-D UNITY_OUTPUT_CHAR=putcharSpy' to enable print testing");
#else
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("0", 0);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("1", 1);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("-1", -1);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("2000000000", 2000000000);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("-2147483648", (UNITY_INT32)0x80000000);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("-1",          (UNITY_INT32)0xFFFFFFFF);
#endif
}

void testAssumePrintNumbers32(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE_MESSAGE("Compile with '-D UNITY_OUTPUT_CHAR=putcharSpy' to enable print testing");
#else
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("0", 0);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("1", 1);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("-1", -1);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("2000000000", 2000000000);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("-2147483648", (UNITY_INT32)0x80000000);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("-1",          (UNITY_INT32)0xFFFFFFFF);
#endif
}

void testPrintNumbersUnsigned32(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("0", 0);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("1", 1);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("1500000000", 1500000000);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("2147483648", (UNITY_UINT32)0x80000000);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("4294967295", (UNITY_UINT32)0xFFFFFFFF);
#endif
}

void testAssumePrintNumbersUnsigned32(void)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("0", 0);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("1", 1);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("1500000000", 1500000000);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("2147483648", (UNITY_UINT32)0x80000000);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("4294967295", (UNITY_UINT32)0xFFFFFFFF);
#endif
}


// ===================== THESE TEST WILL RUN IF YOUR CONFIG INCLUDES 64 BIT SUPPORT ==================

void testPrintNumbersInt64(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
  #ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
  #else
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("0", 0);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("10000000000", 10000000000);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("-9223372036854775808", (UNITY_INT)0x8000000000000000);
    TEST_ASSERT_EQUAL_PRINT_NUMBERS("-1", (UNITY_INT)0xFFFFFFFFFFFFFFFF);
  #endif
#endif
}

void testAssumePrintNumbersInt64(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
  #ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
  #else
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("0", 0);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("10000000000", 10000000000);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("-9223372036854775808", (UNITY_INT)0x8000000000000000);
    TEST_ASSUME_EQUAL_PRINT_NUMBERS("-1", (UNITY_INT)0xFFFFFFFFFFFFFFFF);
  #endif
#endif
}

void testPrintNumbersUInt64(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
  #ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
  #else
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("0", 0);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("70000000000", 70000000000);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("9223372036854775808",  (UNITY_UINT)0x8000000000000000);
    TEST_ASSERT_EQUAL_PRINT_UNSIGNED_NUMBERS("18446744073709551615", (UNITY_UINT)0xFFFFFFFFFFFFFFFF);
  #endif
#endif
}

void testAssumePrintNumbersUInt64(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
  #ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
  #else
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("0", 0);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("70000000000", 70000000000);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("9223372036854775808",  (UNITY_UINT)0x8000000000000000);
    TEST_ASSUME_EQUAL_PRINT_UNSIGNED_NUMBERS("18446744073709551615", (UNITY_UINT)0xFFFFFFFFFFFFFFFF);
  #endif
#endif
}

void testEqualHex64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;
    UNITY_UINT64 *p0, *p1;

    v0 = 0x9876543201234567;
    v1 = 0x9876543201234567;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_HEX64(0x9876543201234567, 0x9876543201234567);
    TEST_ASSERT_EQUAL_HEX64(v0, v1);
    TEST_ASSERT_EQUAL_HEX64(0x9876543201234567, v1);
    TEST_ASSERT_EQUAL_HEX64(v0, 0x9876543201234567);
    TEST_ASSERT_EQUAL_HEX64(*p0, v1);
    TEST_ASSERT_EQUAL_HEX64(*p0, *p1);
    TEST_ASSERT_EQUAL_HEX64(*p0, 0x9876543201234567);
#endif
}

void testAssumeEqualHex64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;
    UNITY_UINT64 *p0, *p1;

    v0 = 0x9876543201234567;
    v1 = 0x9876543201234567;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_HEX64(0x9876543201234567, 0x9876543201234567);
    TEST_ASSUME_EQUAL_HEX64(v0, v1);
    TEST_ASSUME_EQUAL_HEX64(0x9876543201234567, v1);
    TEST_ASSUME_EQUAL_HEX64(v0, 0x9876543201234567);
    TEST_ASSUME_EQUAL_HEX64(*p0, v1);
    TEST_ASSUME_EQUAL_HEX64(*p0, *p1);
    TEST_ASSUME_EQUAL_HEX64(*p0, 0x9876543201234567);
#endif
}

void testEqualUint64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;
    UNITY_UINT64 *p0, *p1;

    v0 = 0x9876543201234567;
    v1 = 0x9876543201234567;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_UINT64(0x9876543201234567, 0x9876543201234567);
    TEST_ASSERT_EQUAL_UINT64(v0, v1);
    TEST_ASSERT_EQUAL_UINT64(0x9876543201234567, v1);
    TEST_ASSERT_EQUAL_UINT64(v0, 0x9876543201234567);
    TEST_ASSERT_EQUAL_UINT64(*p0, v1);
    TEST_ASSERT_EQUAL_UINT64(*p0, *p1);
    TEST_ASSERT_EQUAL_UINT64(*p0, 0x9876543201234567);
#endif
}

void testAssumeEqualUint64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;
    UNITY_UINT64 *p0, *p1;

    v0 = 0x9876543201234567;
    v1 = 0x9876543201234567;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_UINT64(0x9876543201234567, 0x9876543201234567);
    TEST_ASSUME_EQUAL_UINT64(v0, v1);
    TEST_ASSUME_EQUAL_UINT64(0x9876543201234567, v1);
    TEST_ASSUME_EQUAL_UINT64(v0, 0x9876543201234567);
    TEST_ASSUME_EQUAL_UINT64(*p0, v1);
    TEST_ASSUME_EQUAL_UINT64(*p0, *p1);
    TEST_ASSUME_EQUAL_UINT64(*p0, 0x9876543201234567);
#endif
}

void testEqualInt64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 v0, v1;
    UNITY_INT64 *p0, *p1;

    v0 = (UNITY_INT64)0x9876543201234567;
    v1 = (UNITY_INT64)0x9876543201234567;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSERT_EQUAL_INT64(0x9876543201234567, 0x9876543201234567);
    TEST_ASSERT_EQUAL_INT64(v0, v1);
    TEST_ASSERT_EQUAL_INT64(0x9876543201234567, v1);
    TEST_ASSERT_EQUAL_INT64(v0, 0x9876543201234567);
    TEST_ASSERT_EQUAL_INT64(*p0, v1);
    TEST_ASSERT_EQUAL_INT64(*p0, *p1);
    TEST_ASSERT_EQUAL_INT64(*p0, 0x9876543201234567);
#endif
}

void testAssumeEqualInt64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 v0, v1;
    UNITY_INT64 *p0, *p1;

    v0 = (UNITY_INT64)0x9876543201234567;
    v1 = (UNITY_INT64)0x9876543201234567;
    p0 = &v0;
    p1 = &v1;

    TEST_ASSUME_EQUAL_INT64(0x9876543201234567, 0x9876543201234567);
    TEST_ASSUME_EQUAL_INT64(v0, v1);
    TEST_ASSUME_EQUAL_INT64(0x9876543201234567, v1);
    TEST_ASSUME_EQUAL_INT64(v0, 0x9876543201234567);
    TEST_ASSUME_EQUAL_INT64(*p0, v1);
    TEST_ASSUME_EQUAL_INT64(*p0, *p1);
    TEST_ASSUME_EQUAL_INT64(*p0, 0x9876543201234567);
#endif
}


void testNotEqualHex64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;

    v0 = 9000000000;
    v1 = 9100000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX64(v0, v1);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualHex64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;

    v0 = 9000000000;
    v1 = 9100000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX64(v0, v1);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualUint64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;

    v0 = 9000000000;
    v1 = 9100000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT64(v0, v1);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualUint64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 v0, v1;

    v0 = 9000000000;
    v1 = 9100000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT64(v0, v1);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualInt64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 v0, v1;

    v0 = -9000000000;
    v1 = 9100000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT64(v0, v1);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualInt64s(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 v0, v1;

    v0 = -9000000000;
    v1 = 9100000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT64(v0, v1);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualHex64sIfSigned(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 v0, v1;

    v0 = -9000000000;
    v1 = 9000000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX64(v0, v1);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualHex64sIfSigned(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 v0, v1;

    v0 = -9000000000;
    v1 = 9000000000;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX64(v0, v1);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testHEX64sWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    TEST_ASSERT_HEX64_WITHIN(1, 0x7FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFE);
    TEST_ASSERT_HEX64_WITHIN(5, 5000, 4996);
    TEST_ASSERT_HEX64_WITHIN(5, 5000, 5005);
#endif
}

void testAssumeHEX64sWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    TEST_ASSUME_HEX64_WITHIN(1, 0x7FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFE);
    TEST_ASSUME_HEX64_WITHIN(5, 5000, 4996);
    TEST_ASSUME_HEX64_WITHIN(5, 5000, 5005);
#endif
}

void testHEX64sNotWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX64_WITHIN(1, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFC);
    VERIFY_FAILS_END
#endif
}

void testAssumeHEX64sNotWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX64_WITHIN(1, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFC);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testHEX64sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_HEX64_WITHIN(5, 1, -1);
    VERIFY_FAILS_END
#endif
}

void testAssumeHEX64sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_HEX64_WITHIN(5, 1, -1);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testUINT64sWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    TEST_ASSERT_UINT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFE);
    TEST_ASSERT_UINT64_WITHIN(5, 5000, 4996);
    TEST_ASSERT_UINT64_WITHIN(5, 5000, 5005);
#endif
}

void testAssumeUINT64sWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    TEST_ASSUME_UINT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFE);
    TEST_ASSUME_UINT64_WITHIN(5, 5000, 4996);
    TEST_ASSUME_UINT64_WITHIN(5, 5000, 5005);
#endif
}

void testUINT64sNotWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFC);
    VERIFY_FAILS_END
#endif
}

void testAssumeUINT64sNotWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFC);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testUINT64sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_UINT64_WITHIN(5, 1, -1);
    VERIFY_FAILS_END
#endif
}

void testAssumeUINT64sNotWithinDeltaEvenThoughASignedIntWouldPass(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_UINT64_WITHIN(5, 1, -1);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testINT64sWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    TEST_ASSERT_INT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFE);
    TEST_ASSERT_INT64_WITHIN(5, 5000, 4996);
    TEST_ASSERT_INT64_WITHIN(5, 5000, 5005);
#endif
}

void testAssumeINT64sWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    TEST_ASSUME_INT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFE);
    TEST_ASSUME_INT64_WITHIN(5, 5000, 4996);
    TEST_ASSUME_INT64_WITHIN(5, 5000, 5005);
#endif
}

void testINT64sNotWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFC);
    VERIFY_FAILS_END
#endif
}

void testAssumeINT64sNotWithinDelta(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT64_WITHIN(1, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFC);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testINT64sNotWithinDeltaAndDifferenceOverflows(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_INT64_WITHIN(1, 0x8000000000000000, 0x7FFFFFFFFFFFFFFF);
    VERIFY_FAILS_END
#endif
}

void testAssumeINT64sNotWithinDeltaAndDifferenceOverflows(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_INT64_WITHIN(1, 0x8000000000000000, 0x7FFFFFFFFFFFFFFF);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualHEX64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p2[] = {1, 8, 987, 2};
    UNITY_UINT64 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p3, 1);
#endif
}

void testAssumeEqualHEX64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p2[] = {1, 8, 987, 2};
    UNITY_UINT64 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p3, 1);
#endif
}

void testEqualUint64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p2[] = {1, 8, 987, 2};
    UNITY_UINT64 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_UINT64_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_UINT64_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_UINT64_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_UINT64_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_UINT64_ARRAY(p0, p3, 1);
#endif
}

void testAssumeEqualUint64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p2[] = {1, 8, 987, 2};
    UNITY_UINT64 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_UINT64_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_UINT64_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_UINT64_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_UINT64_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_UINT64_ARRAY(p0, p3, 1);
#endif
}

void testEqualInt64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 p0[] = {1, 8, 987, -65132};
    UNITY_INT64 p1[] = {1, 8, 987, -65132};
    UNITY_INT64 p2[] = {1, 8, 987, -2};
    UNITY_INT64 p3[] = {1, 500, 600, 700};

    TEST_ASSERT_EQUAL_INT64_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_INT64_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_INT64_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_INT64_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_INT64_ARRAY(p0, p3, 1);
#endif
}

void testAssumeEqualInt64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 p0[] = {1, 8, 987, -65132};
    UNITY_INT64 p1[] = {1, 8, 987, -65132};
    UNITY_INT64 p2[] = {1, 8, 987, -2};
    UNITY_INT64 p3[] = {1, 500, 600, 700};

    TEST_ASSUME_EQUAL_INT64_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_INT64_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_INT64_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_INT64_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_INT64_ARRAY(p0, p3, 1);
#endif
}


void testNotEqualHEX64Arrays1(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualHEX64Arrays1(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualHEX64Arrays2(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX64_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualHEX64Arrays2(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {2, 8, 987, 65132u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX64_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualUint64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_UINT64_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualUint64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_UINT64 p0[] = {1, 8, 987, 65132u};
    UNITY_UINT64 p1[] = {1, 8, 987, 65131u};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_UINT64_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualInt64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 p0[] = {1, 8, 987, -65132};
    UNITY_INT64 p1[] = {1, 8, 987, -65131};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT64_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualInt64Arrays(void)
{
#ifndef UNITY_SUPPORT_64
    TEST_IGNORE();
#else
    UNITY_INT64 p0[] = {1, 8, 987, -65132};
    UNITY_INT64 p1[] = {1, 8, 987, -65131};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT64_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}
// ===================== THESE TEST WILL RUN IF YOUR CONFIG INCLUDES FLOAT SUPPORT ==================

void testFloatsWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_WITHIN(0.00003f, 187245.03485f, 187245.03488f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 187245.0f, 187246.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 9273.2549f, 9273.2049f);
    TEST_ASSERT_FLOAT_WITHIN(0.007f, -726.93724f, -726.94424f);
#endif
}

void testAssumeFloatsWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_WITHIN(0.00003f, 187245.03485f, 187245.03488f);
    TEST_ASSUME_FLOAT_WITHIN(1.0f, 187245.0f, 187246.0f);
    TEST_ASSUME_FLOAT_WITHIN(0.05f, 9273.2549f, 9273.2049f);
    TEST_ASSUME_FLOAT_WITHIN(0.007f, -726.93724f, -726.94424f);
#endif
}

void testFloatsNotWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 9273.2649f, 9273.2049f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_WITHIN(0.05f, 9273.2649f, 9273.2049f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsEqual(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_FLOAT(187245.0f, 187246.0f);
    TEST_ASSERT_EQUAL_FLOAT(18724.5f, 18724.6f);
    TEST_ASSERT_EQUAL_FLOAT(9273.2549f, 9273.2599f);
    TEST_ASSERT_EQUAL_FLOAT(-726.93724f, -726.9374f);
#endif
}

void testAssumeFloatsEqual(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_FLOAT(187245.0f, 187246.0f);
    TEST_ASSUME_EQUAL_FLOAT(18724.5f, 18724.6f);
    TEST_ASSUME_EQUAL_FLOAT(9273.2549f, 9273.2599f);
    TEST_ASSUME_EQUAL_FLOAT(-726.93724f, -726.9374f);
#endif
}

void testFloatsNotEqual(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(9273.9649f, 9273.0049f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqual(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(9273.9649f, 9273.0049f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(-9273.9649f, -9273.0049f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(-9273.9649f, -9273.0049f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(-9273.0049f, -9273.9649f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(-9273.0049f, -9273.9649f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualActualNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(85.963f, 0.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualActualNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(85.963f, 0.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualExpectedNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(0.0f / f_zero, 85.963f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualExpectedNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(0.0f / f_zero, 85.963f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsEqualBothNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_FLOAT(0.0f / f_zero, 0.0f / f_zero);
#endif
}

void testAssumeFloatsEqualBothNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_FLOAT(0.0f / f_zero, 0.0f / f_zero);
#endif
}

void testFloatsNotEqualInfNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(1.0f / f_zero, 0.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualInfNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(1.0f / f_zero, 0.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualNaNInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(0.0f / f_zero, 1.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualNaNInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(0.0f / f_zero, 1.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualActualInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(321.642f, 1.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualActualInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(321.642f, 1.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsNotEqualExpectedInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(1.0f / f_zero, 321.642f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualExpectedInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(1.0f / f_zero, 321.642f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatsEqualBothInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_FLOAT(1.0f / f_zero, 1.0f / f_zero);
#endif
}

void testAssumeFloatsEqualBothInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_FLOAT(1.0f / f_zero, 1.0f / f_zero);
#endif
}

void testFloatsNotEqualPlusMinusInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT(1.0f / f_zero, -1.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatsNotEqualPlusMinusInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT(1.0f / f_zero, -1.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsPosInf1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_INF(2.0f / f_zero);
#endif
}

void testAssumeFloatIsPosInf1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_INF(2.0f / f_zero);
#endif
}

void testFloatIsPosInf2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NOT_INF(2.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsPosInf2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NOT_INF(2.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNegInf1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_NEG_INF(-3.0f / f_zero);
#endif
}

void testAssumeFloatIsNegInf1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_NEG_INF(-3.0f / f_zero);
#endif
}

void testFloatIsNegInf2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NOT_NEG_INF(-3.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsNegInf2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NOT_NEG_INF(-3.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNotPosInf1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_INF(2.0f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsNotPosInf1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_INF(2.0f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNotPosInf2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_NOT_INF(2.0f);
#endif
}

void testAssumeFloatIsNotPosInf2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_NOT_INF(2.0f);
#endif
}

void testFloatIsNotNegInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NEG_INF(-999.876f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsNotNegInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NEG_INF(-999.876f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNan1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_NAN(0.0f / f_zero);
#endif
}

void testAssumeFloatIsNan1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_NAN(0.0f / f_zero);
#endif
}

void testFloatIsNan2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NOT_NAN(0.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsNan2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NOT_NAN(0.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNotNan1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NAN(234.9f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsNotNan1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NAN(234.9f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNotNan2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_NOT_NAN(234.9f);
#endif
}

void testAssumeFloatIsNotNan2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_NOT_NAN(234.9f);
#endif
}

void testFloatInfIsNotNan(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NAN(1.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatInfIsNotNan(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NAN(1.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatNanIsNotInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_INF(0.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatNanIsNotInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_INF(0.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_DETERMINATE(0.0f);
    TEST_ASSERT_FLOAT_IS_DETERMINATE(123.3f);
    TEST_ASSERT_FLOAT_IS_DETERMINATE(-88.3f);
#endif
}

void testAssumeFloatIsDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_DETERMINATE(0.0f);
    TEST_ASSUME_FLOAT_IS_DETERMINATE(123.3f);
    TEST_ASSUME_FLOAT_IS_DETERMINATE(-88.3f);
#endif
}

void testFloatIsDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(-88.3f);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_NOT_DETERMINATE(-88.3f);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatIsNotDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(1.0f / f_zero);
    TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(-1.0f / f_zero);
    TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(0.0f / f_zero);
#endif
}

void testAssumeFloatIsNotDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    TEST_ASSUME_FLOAT_IS_NOT_DETERMINATE(1.0f / f_zero);
    TEST_ASSUME_FLOAT_IS_NOT_DETERMINATE(-1.0f / f_zero);
    TEST_ASSUME_FLOAT_IS_NOT_DETERMINATE(0.0f / f_zero);
#endif
}

void testFloatIsNotDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_FLOAT_IS_DETERMINATE(-1.0f / f_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatIsNotDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_FLOAT_IS_DETERMINATE(-1.0f / f_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testFloatTraitFailsOnInvalidTrait(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    UnityTestFloatSpecial(1.0f, NULL, __LINE__, UNITY_FLOAT_INVALID_TRAIT, UNITY_TEST_TYPE_ASSERT);
    VERIFY_FAILS_END
#endif
}

void testAssumeFloatTraitFailsOnInvalidTrait(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    UnityTestFloatSpecial(1.0f, NULL, __LINE__, UNITY_FLOAT_INVALID_TRAIT, UNITY_TEST_TYPE_ASSUME);
    VERIFY_INCONCLUSIVE_END
#endif
}


void testEqualFloatArrays(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, -8.0f,  25.4f, -0.123f};
    float p1[] = {1.0f, -8.0f,  25.4f, -0.123f};
    float p2[] = {1.0f, -8.0f,  25.4f, -0.2f};
    float p3[] = {1.0f, -23.0f, 25.0f, -0.26f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p3, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(NULL, NULL, 1);
#endif
}

void testAssumeEqualFloatArrays(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, -8.0f,  25.4f, -0.123f};
    float p1[] = {1.0f, -8.0f,  25.4f, -0.123f};
    float p2[] = {1.0f, -8.0f,  25.4f, -0.2f};
    float p3[] = {1.0f, -23.0f, 25.0f, -0.26f};

    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p3, 1);
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(NULL, NULL, 1);
#endif
}

void testNotEqualFloatArraysExpectedNull(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float* p0 = NULL;
    float p1[] = {1.0f, 8.0f, 25.4f, 0.252f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArraysExpectedNull(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float* p0 = NULL;
    float p1[] = {1.0f, 8.0f, 25.4f, 0.252f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArraysActualNull(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArraysActualNull(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArrays1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float p1[] = {1.0f, 8.0f, 25.4f, 0.252f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArrays1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float p1[] = {1.0f, 8.0f, 25.4f, 0.252f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArrays2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float p1[] = {2.0f, 8.0f, 25.4f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArrays2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float p1[] = {2.0f, 8.0f, 25.4f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArrays3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float p1[] = {1.0f, 8.0f, 25.5f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArrays3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 8.0f, 25.4f, 0.253f};
    float p1[] = {1.0f, 8.0f, 25.5f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArraysNegative1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -8.0f, -25.4f, -0.253f};
    float p1[] = {-1.0f, -8.0f, -25.4f, -0.252f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArraysNegative1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -8.0f, -25.4f, -0.253f};
    float p1[] = {-1.0f, -8.0f, -25.4f, -0.252f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArraysNegative2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -8.0f, -25.4f, -0.253f};
    float p1[] = {-2.0f, -8.0f, -25.4f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArraysNegative2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -8.0f, -25.4f, -0.253f};
    float p1[] = {-2.0f, -8.0f, -25.4f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatArraysNegative3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -8.0f, -25.4f, -0.253f};
    float p1[] = {-1.0f, -8.0f, -25.5f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArraysNegative3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -8.0f, -25.4f, -0.253f};
    float p1[] = {-1.0f, -8.0f, -25.5f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualFloatArraysNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 0.0f / f_zero, 25.4f, 0.253f};
    float p1[] = {1.0f, 0.0f / f_zero, 25.4f, 0.253f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
#endif
}

void testAssumeEqualFloatArraysNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 0.0f / f_zero, 25.4f, 0.253f};
    float p1[] = {1.0f, 0.0f / f_zero, 25.4f, 0.253f};

    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
#endif
}

void testEqualFloatArraysInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 1.0f / f_zero, 25.4f, 0.253f};
    float p1[] = {1.0f, 1.0f / f_zero, 25.4f, 0.253f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 4);
#endif
}

void testAssumeEqualFloatArraysInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 1.0f / f_zero, 25.4f, 0.253f};
    float p1[] = {1.0f, 1.0f / f_zero, 25.4f, 0.253f};

    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 4);
#endif
}

void testNotEqualFloatArraysLengthZero(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[1] = {0.0f};
    float p1[1] = {0.0f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(p0, p1, 0);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatArraysLengthZero(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[1] = {0.0f};
    float p1[1] = {0.0f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_FLOAT_ARRAY(p0, p1, 0);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualFloatEachEqual(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float p1[] = {-0.123f, -0.123f, -0.123f, -0.123f};
    float p2[] = {25.4f, 25.4f, 25.4f, -0.2f};
    float p3[] = {1.0f, -23.0f, 25.0f, -0.26f};

    TEST_ASSERT_EACH_EQUAL_FLOAT(1.0f, p0, 1);
    TEST_ASSERT_EACH_EQUAL_FLOAT(1.0f, p0, 4);
    TEST_ASSERT_EACH_EQUAL_FLOAT(-0.123f, p1, 4);
    TEST_ASSERT_EACH_EQUAL_FLOAT(25.4f, p2, 3);
    TEST_ASSERT_EACH_EQUAL_FLOAT(1.0f, p3, 1);
#endif
}

void testAssumeEqualFloatEachEqual(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float p1[] = {-0.123f, -0.123f, -0.123f, -0.123f};
    float p2[] = {25.4f, 25.4f, 25.4f, -0.2f};
    float p3[] = {1.0f, -23.0f, 25.0f, -0.26f};

    TEST_ASSUME_EACH_EQUAL_FLOAT(1.0f, p0, 1);
    TEST_ASSUME_EACH_EQUAL_FLOAT(1.0f, p0, 4);
    TEST_ASSUME_EACH_EQUAL_FLOAT(-0.123f, p1, 4);
    TEST_ASSUME_EACH_EQUAL_FLOAT(25.4f, p2, 3);
    TEST_ASSUME_EACH_EQUAL_FLOAT(1.0f, p3, 1);
#endif
}

void testNotEqualFloatEachEqualActualNull(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float* p0 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(5, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqualActualNull(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float* p0 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(5, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatEachEqual1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {0.253f, 8.0f, 0.253f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(0.253f, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqual1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {0.253f, 8.0f, 0.253f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(0.253f, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatEachEqual2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {8.0f, 8.0f, 8.0f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(8.0f, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqual2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {8.0f, 8.0f, 8.0f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(8.0f, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatEachEqual3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 1.0f, 1.0f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(1.0f, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqual3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f, 1.0f, 1.0f, 0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(1.0f, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatEachEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -0.253f, -0.253f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(-0.253f, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-1.0f, -0.253f, -0.253f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(-0.253f, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatEachEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-25.4f, -8.0f, -25.4f, -25.4f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(-25.4f, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-25.4f, -8.0f, -25.4f, -25.4f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(-25.4f, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualFloatEachEqualNegative3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-8.0f, -8.0f, -8.0f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(-8.0f, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqualNegative3(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {-8.0f, -8.0f, -8.0f, -0.253f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(-8.0f, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualFloatEachEqualNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {0.0f / f_zero, 0.0f / f_zero, 0.0f / f_zero, 0.0f / f_zero};

    TEST_ASSERT_EACH_EQUAL_FLOAT(0.0f / f_zero, p0, 4);
#endif
}

void testAssumeEqualFloatEachEqualNaN(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {0.0f / f_zero, 0.0f / f_zero, 0.0f / f_zero, 0.0f / f_zero};

    TEST_ASSUME_EACH_EQUAL_FLOAT(0.0f / f_zero, p0, 4);
#endif
}

void testEqualFloatEachEqualInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f / f_zero, 1.0f / f_zero, 25.4f, 0.253f};

    TEST_ASSERT_EACH_EQUAL_FLOAT(1.0f / f_zero, p0, 2);
#endif
}

void testAssumeEqualFloatEachEqualInf(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[] = {1.0f / f_zero, 1.0f / f_zero, 25.4f, 0.253f};

    TEST_ASSUME_EACH_EQUAL_FLOAT(1.0f / f_zero, p0, 2);
#endif
}

void testNotEqualFloatEachEqualLengthZero(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[1] = {0.0f};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_FLOAT(0.0f, p0, 0);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualFloatEachEqualLengthZero(void)
{
#ifdef UNITY_EXCLUDE_FLOAT
    TEST_IGNORE();
#else
    float p0[1] = {0.0f};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_FLOAT(0.0f, p0, 0);
    VERIFY_INCONCLUSIVE_END
#endif
}

#define TEST_ASSERT_EQUAL_PRINT_FLOATING(expected, actual) {            \
        startPutcharSpy(); UnityPrintFloat((actual)); endPutcharSpy();  \
        TEST_ASSERT_EQUAL_STRING((expected), getBufferPutcharSpy());    \
        }

#define TEST_ASSUME_EQUAL_PRINT_FLOATING(expected, actual) {            \
        startPutcharSpy(); UnityPrintFloat((actual)); endPutcharSpy();  \
        TEST_ASSUME_EQUAL_STRING((expected), getBufferPutcharSpy());    \
        }

void testFloatPrinting(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_INCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_PRINT_FLOATING("0",            0.0f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("4.99e-07",     0.000000499f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("0.1004695",    0.100469499f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("2",            1.9999995f); /*Rounding to int place*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1",            1.0f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1.25",         1.25f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("7.999999",     7.999999f); /*Not rounding*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("16.00002",     16.00002f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("16.00004",     16.00004f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("16.00006",     16.00006f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("9999999",      9999999.0f); /*Last full print integer*/

    TEST_ASSERT_EQUAL_PRINT_FLOATING("-0",            -0.0f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-4.99e-07",     -0.000000499f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-0.1004695",    -0.100469499f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-2",            -1.9999995f); /*Rounding to int place*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-1",            -1.0f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-1.25",         -1.25f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-7.999999",     -7.999999f); /*Not rounding*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-16.00002",     -16.00002f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-16.00004",     -16.00004f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-16.00006",     -16.00006f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-9999999",      -9999999.0f); /*Last full print integer*/

    /* Fails, prints "4.294968e+09" due to FP math imprecision
     * TEST_ASSERT_EQUAL_PRINT_FLOATING("4.294967e+09",  4294967296.0f); */
    TEST_ASSERT_EQUAL_PRINT_FLOATING("5e+09",        5000000000.0f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("8e+09",        8.0e+09f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("8.309999e+09", 8309999104.0f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1e+10",        1.0e+10f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1e+10",        10000000000.0f);
    /* Some compilers have trouble with inexact float constants, a float cast works generally */
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1.000055e+10", (float)1.000055e+10f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1.1e+38",      (float)1.10000005e+38f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1.635299e+10", 1.63529943e+10f);
    /* Fails, prints "3.402824e+38" due to FP math imprecision
     * TEST_ASSERT_EQUAL_PRINT_FLOATING("3.402823e+38", 3.40282346638e38f); */

    TEST_ASSERT_EQUAL_PRINT_FLOATING("-1e+10",        -1.0e+10f);
    /* Fails, prints "-3.402824e+38" due to FP math imprecision
     * TEST_ASSERT_EQUAL_PRINT_FLOATING("-3.402823e+38", -3.40282346638e38f); */
#endif
}

void testAssumeFloatPrinting(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_INCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_PRINT_FLOATING("0",            0.0f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("4.99e-07",     0.000000499f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("0.1004695",    0.100469499f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("2",            1.9999995f); /*Rounding to int place*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1",            1.0f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1.25",         1.25f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("7.999999",     7.999999f); /*Not rounding*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("16.00002",     16.00002f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("16.00004",     16.00004f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("16.00006",     16.00006f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("9999999",      9999999.0f); /*Last full print integer*/

    TEST_ASSUME_EQUAL_PRINT_FLOATING("-0",            -0.0f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-4.99e-07",     -0.000000499f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-0.1004695",    -0.100469499f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-2",            -1.9999995f); /*Rounding to int place*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-1",            -1.0f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-1.25",         -1.25f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-7.999999",     -7.999999f); /*Not rounding*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-16.00002",     -16.00002f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-16.00004",     -16.00004f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-16.00006",     -16.00006f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-9999999",      -9999999.0f); /*Last full print integer*/

    /* Fails, prints "4.294968e+09" due to FP math imprecision
     * TEST_ASSUME_EQUAL_PRINT_FLOATING("4.294967e+09",  4294967296.0f); */
    TEST_ASSUME_EQUAL_PRINT_FLOATING("5e+09",        5000000000.0f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("8e+09",        8.0e+09f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("8.309999e+09", 8309999104.0f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1e+10",        1.0e+10f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1e+10",        10000000000.0f);
    /* Some compilers have trouble with inexact float constants, a float cast works generally */
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1.000055e+10", (float)1.000055e+10f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1.1e+38",      (float)1.10000005e+38f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1.635299e+10", 1.63529943e+10f);
    /* Fails, prints "3.402824e+38" due to FP math imprecision
     * TEST_ASSUME_EQUAL_PRINT_FLOATING("3.402823e+38", 3.40282346638e38f); */

    TEST_ASSUME_EQUAL_PRINT_FLOATING("-1e+10",        -1.0e+10f);
    /* Fails, prints "-3.402824e+38" due to FP math imprecision
     * TEST_ASSUME_EQUAL_PRINT_FLOATING("-3.402823e+38", -3.40282346638e38f); */
#endif
}

void testFloatPrintingRoundTiesToEven(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_INCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
  #ifdef UNITY_ROUND_TIES_AWAY_FROM_ZERO
    TEST_ASSERT_EQUAL_PRINT_FLOATING("0.0004882813",  0.00048828125f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("488281.3",      488281.25f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("5.000001e-07",  0.00000050000005f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-5.000001e-07", -0.00000050000005f);
  #else /* Default to Round ties to even */
    TEST_ASSERT_EQUAL_PRINT_FLOATING("0.0004882812",  0.00048828125f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("488281.2",      488281.25f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("5e-07",         0.00000050000005f);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-5e-07",        -0.00000050000005f);
  #endif
#endif
}

void testAssumeFloatPrintingRoundTiesToEven(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_INCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
  #ifdef UNITY_ROUND_TIES_AWAY_FROM_ZERO
    TEST_ASSUME_EQUAL_PRINT_FLOATING("0.0004882813",  0.00048828125f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("488281.3",      488281.25f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("5.000001e-07",  0.00000050000005f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-5.000001e-07", -0.00000050000005f);
  #else /* Default to Round ties to even */
    TEST_ASSUME_EQUAL_PRINT_FLOATING("0.0004882812",  0.00048828125f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("488281.2",      488281.25f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("5e-07",         0.00000050000005f);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-5e-07",        -0.00000050000005f);
  #endif
#endif
}

void testFloatPrintingInfinityAndNaN(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_PRINT_FLOATING("inf",   1.0f / f_zero);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-inf", -1.0f / f_zero);

    TEST_ASSERT_EQUAL_PRINT_FLOATING("nan",   0.0f / f_zero);
#endif
}

void testAssumeFloatPrintingInfinityAndNaN(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_PRINT_FLOATING("inf",   1.0f / f_zero);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-inf", -1.0f / f_zero);

    TEST_ASSUME_EQUAL_PRINT_FLOATING("nan",   0.0f / f_zero);
#endif
}

#if defined(UNITY_TEST_ALL_FLOATS_PRINT_OK) && defined(USING_OUTPUT_SPY)
#ifdef UNITY_INCLUDE_DOUBLE
static void printFloatValue(float f)
{
    char expected[18];

    startPutcharSpy();
    UnityPrintFloat(f);

    sprintf(expected, "%.9g", f);
    /* We print all NaN's as "nan", not "-nan" */
    if (strcmp(expected, "-nan") == 0) strcpy(expected, "nan");

    if (strcmp(expected, getBufferPutcharSpy()))
    {
        /* Fail with diagnostic printing */
        TEST_ASSERT_EQUAL_PRINT_FLOATING(expected, f);
    }
}
#else
static void printFloatValue(float f)
{
    char expected[18];
    char expected_lower[18];
    char expected_lower2[18];
    char expected_lower3[18];
    char expected_higher[18];
    char expected_higher2[18];
    char expected_higher3[18];

    startPutcharSpy();
    UnityPrintFloat(f);

    sprintf(expected, "%.7g", f);
    /* We print all NaN's as "nan", not "-nan" */
    if (strcmp(expected, "-nan") == 0) strcpy(expected, "nan");

    strcpy(expected_lower, expected);
    strcpy(expected_lower2, expected);
    strcpy(expected_lower3, expected);
    strcpy(expected_higher, expected);
    strcpy(expected_higher2, expected);
    strcpy(expected_higher3, expected);

    /* Allow for rounding differences in the last digit */
    double lower = (double)f * 0.99999995;
    double higher = (double)f * 1.00000005;

    if(isfinite(lower)) sprintf(expected_lower, "%.7g", lower);
    if(isfinite(higher)) sprintf(expected_higher, "%.7g", higher);

    /* Outside [1,10000000] allow for relative error of +/-2.5e-7 */
    if (f < 1.0 || f > 10000000)
    {
        double lower2 = (double)f * 0.99999985;
        double lower3 = (double)f * 0.99999975;
        double higher2 = (double)f * 1.00000015;
        double higher3 = (double)f * 1.00000025;

        if (isfinite(lower2)) sprintf(expected_lower2, "%.7g", lower2);
        if (isfinite(lower3)) sprintf(expected_lower3, "%.7g", lower3);
        if (isfinite(higher2)) sprintf(expected_higher2, "%.7g", higher2);
        if (isfinite(higher3)) sprintf(expected_higher3, "%.7g", higher3);
    }

    if (strcmp(expected, getBufferPutcharSpy()) != 0 &&
        strcmp(expected_lower, getBufferPutcharSpy()) != 0 &&
        strcmp(expected_lower2, getBufferPutcharSpy()) != 0 &&
        strcmp(expected_lower3, getBufferPutcharSpy()) != 0 &&
        strcmp(expected_higher, getBufferPutcharSpy()) != 0 &&
        strcmp(expected_higher2, getBufferPutcharSpy()) != 0 &&
        strcmp(expected_higher3, getBufferPutcharSpy()) != 0)
    {
        /* Fail with diagnostic printing */
        TEST_ASSERT_EQUAL_PRINT_FLOATING(expected, f);
    }
}
#endif
#endif

void testFloatPrintingRandomSamples(void)
{
#if !defined(UNITY_TEST_ALL_FLOATS_PRINT_OK) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    union { float f_value; uint32_t int_value; } u;

    /* These values are not covered by the MINSTD generator */
    u.int_value = 0x00000000; printFloatValue(u.f_value);
    u.int_value = 0x80000000; printFloatValue(u.f_value);
    u.int_value = 0x7fffffff; printFloatValue(u.f_value);
    u.int_value = 0xffffffff; printFloatValue(u.f_value);

    uint32_t a = 1;
    for(int num_tested = 0; num_tested < 1000000; num_tested++)
    {
        /* MINSTD pseudo-random number generator */
        a = (uint32_t)(((uint64_t)a * 48271u) % 2147483647u);

        /* MINSTD does not set the highest bit; test both possibilities */
        u.int_value = a;              printFloatValue(u.f_value);
        u.int_value = a | 0x80000000; printFloatValue(u.f_value);
    }
#endif
}

void testAssumeFloatPrintingRandomSamples(void)
{
#if !defined(UNITY_TEST_ALL_FLOATS_PRINT_OK) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    union { float f_value; uint32_t int_value; } u;

    /* These values are not covered by the MINSTD generator */
    u.int_value = 0x00000000; printFloatValue(u.f_value);
    u.int_value = 0x80000000; printFloatValue(u.f_value);
    u.int_value = 0x7fffffff; printFloatValue(u.f_value);
    u.int_value = 0xffffffff; printFloatValue(u.f_value);

    uint32_t a = 1;
    for(int num_tested = 0; num_tested < 1000000; num_tested++)
    {
        /* MINSTD pseudo-random number generator */
        a = (uint32_t)(((uint64_t)a * 48271u) % 2147483647u);

        /* MINSTD does not set the highest bit; test both possibilities */
        u.int_value = a;              printFloatValue(u.f_value);
        u.int_value = a | 0x80000000; printFloatValue(u.f_value);
    }
#endif
}

// ===================== THESE TEST WILL RUN IF YOUR CONFIG INCLUDES DOUBLE SUPPORT ==================

void testDoublesWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_WITHIN(0.00003, 187245.03485, 187245.03488);
    TEST_ASSERT_DOUBLE_WITHIN(1.0, 187245.0, 187246.0);
    TEST_ASSERT_DOUBLE_WITHIN(0.05, 9273.2549, 9273.2049);
    TEST_ASSERT_DOUBLE_WITHIN(0.007, -726.93725, -726.94424);
#endif
}

void testAssumeDoublesWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_WITHIN(0.00003, 187245.03485, 187245.03488);
    TEST_ASSUME_DOUBLE_WITHIN(1.0, 187245.0, 187246.0);
    TEST_ASSUME_DOUBLE_WITHIN(0.05, 9273.2549, 9273.2049);
    TEST_ASSUME_DOUBLE_WITHIN(0.007, -726.93725, -726.94424);
#endif
}

void testDoublesNotWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_WITHIN(0.05, 9273.2649, 9273.2049);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotWithinDelta(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_WITHIN(0.05, 9273.2649, 9273.2049);
    VERIFY_INCONCLUSIVE_END
#endif
}


void testDoublesEqual(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_DOUBLE(187245123456.0, 187245123456.0);
    TEST_ASSERT_EQUAL_DOUBLE(187241234567.5, 187241234567.6);
    TEST_ASSERT_EQUAL_DOUBLE(9273.2512345649, 9273.25123455699);
    TEST_ASSERT_EQUAL_DOUBLE(-726.12345693724, -726.1234569374);
#endif
}

void testAssumeDoublesEqual(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_DOUBLE(187245123456.0, 187245123456.0);
    TEST_ASSUME_EQUAL_DOUBLE(187241234567.5, 187241234567.6);
    TEST_ASSUME_EQUAL_DOUBLE(9273.2512345649, 9273.25123455699);
    TEST_ASSUME_EQUAL_DOUBLE(-726.12345693724, -726.1234569374);
#endif
}

void testDoublesNotEqual(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(9273.9649, 9273.0049);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqual(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(9273.9649, 9273.0049);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(-9273.9649, -9273.0049);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(-9273.9649, -9273.0049);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(-9273.0049, -9273.9649);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(-9273.0049, -9273.9649);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualActualNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(85.963, 0.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualActualNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(85.963, 0.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualExpectedNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(0.0 / d_zero, 85.963);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualExpectedNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(0.0 / d_zero, 85.963);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesEqualBothNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_DOUBLE(0.0 / d_zero, 0.0 / d_zero);
#endif
}

void testAssumeDoublesEqualBothNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_DOUBLE(0.0 / d_zero, 0.0 / d_zero);
#endif
}

void testDoublesNotEqualInfNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(1.0 / d_zero, 0.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualInfNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(1.0 / d_zero, 0.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualNaNInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(0.0 / d_zero, 1.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualNaNInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(0.0 / d_zero, 1.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualActualInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(321.642, 1.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualActualInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(321.642, 1.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesNotEqualExpectedInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(1.0 / d_zero, 321.642);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualExpectedInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(1.0 / d_zero, 321.642);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublesEqualBothInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_DOUBLE(1.0 / d_zero, 1.0 / d_zero);
#endif
}

void testAssumeDoublesEqualBothInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_DOUBLE(1.0 / d_zero, 1.0 / d_zero);
#endif
}

void testDoublesNotEqualPlusMinusInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE(1.0 / d_zero, -1.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoublesNotEqualPlusMinusInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE(1.0 / d_zero, -1.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsPosInf1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_INF(2.0 / d_zero);
#endif
}

void testAssumeDoubleIsPosInf1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_INF(2.0 / d_zero);
#endif
}

void testDoubleIsPosInf2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NOT_INF(2.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsPosInf2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NOT_INF(2.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNegInf1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_NEG_INF(-3.0 / d_zero);
#endif
}

void testAssumeDoubleIsNegInf1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_NEG_INF(-3.0 / d_zero);
#endif
}

void testDoubleIsNegInf2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF(-3.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsNegInf2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NOT_NEG_INF(-3.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNotPosInf1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_INF(2.0);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsNotPosInf1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_INF(2.0);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNotPosInf2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_NOT_INF(2.0);
#endif
}

void testAssumeDoubleIsNotPosInf2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_NOT_INF(2.0);
#endif
}

void testDoubleIsNotNegInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NEG_INF(-999.876);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsNotNegInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NEG_INF(-999.876);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNan1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_NAN(0.0 / d_zero);
#endif
}

void testAssumeDoubleIsNan1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_NAN(0.0 / d_zero);
#endif
}

void testDoubleIsNan2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NOT_NAN(0.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsNan2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NOT_NAN(0.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNotNan1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NAN(234.9);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsNotNan1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NAN(234.9);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNotNan2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_NOT_NAN(234.9);
#endif
}

void testAssumeDoubleIsNotNan2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_NOT_NAN(234.9);
#endif
}

void testDoubleInfIsNotNan(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NAN(1.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleInfIsNotNan(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NAN(1.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleNanIsNotInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_INF(0.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleNanIsNotInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_INF(0.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_DETERMINATE(0.0);
    TEST_ASSERT_DOUBLE_IS_DETERMINATE(123.3);
    TEST_ASSERT_DOUBLE_IS_DETERMINATE(-88.3);
#endif
}

void testAssumeDoubleIsDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_DETERMINATE(0.0);
    TEST_ASSUME_DOUBLE_IS_DETERMINATE(123.3);
    TEST_ASSUME_DOUBLE_IS_DETERMINATE(-88.3);
#endif
}

void testDoubleIsDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(-88.3);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_NOT_DETERMINATE(-88.3);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleIsNotDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(1.0 / d_zero);
    TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(-1.0 / d_zero);
    TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(0.0 / d_zero);
#endif
}

void testAssumeDoubleIsNotDeterminate1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    TEST_ASSUME_DOUBLE_IS_NOT_DETERMINATE(1.0 / d_zero);
    TEST_ASSUME_DOUBLE_IS_NOT_DETERMINATE(-1.0 / d_zero);
    TEST_ASSUME_DOUBLE_IS_NOT_DETERMINATE(0.0 / d_zero);
#endif
}

void testDoubleIsNotDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSERT_DOUBLE_IS_DETERMINATE(-1.0 / d_zero);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleIsNotDeterminate2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    TEST_ASSUME_DOUBLE_IS_DETERMINATE(-1.0 / d_zero);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoubleTraitFailsOnInvalidTrait(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    UnityTestDoubleSpecial(1.0, NULL, __LINE__, UNITY_FLOAT_INVALID_TRAIT, UNITY_TEST_TYPE_ASSERT);
    VERIFY_FAILS_END
#endif
}

void testAssumeDoubleTraitFailsOnInvalidTrait(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    EXPECT_ABORT_BEGIN
    UnityTestDoubleSpecial(1.0, NULL, __LINE__, UNITY_FLOAT_INVALID_TRAIT, UNITY_TEST_TYPE_ASSUME);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualDoubleArrays(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, -8.0,  25.4, -0.123};
    double p1[] = {1.0, -8.0,  25.4, -0.123};
    double p2[] = {1.0, -8.0,  25.4, -0.2};
    double p3[] = {1.0, -23.0, 25.0, -0.26};

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p0, 1);
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p0, 4);
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p2, 3);
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p3, 1);
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(NULL, NULL, 1);
#endif
}

void testAssumeEqualDoubleArrays(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, -8.0,  25.4, -0.123};
    double p1[] = {1.0, -8.0,  25.4, -0.123};
    double p2[] = {1.0, -8.0,  25.4, -0.2};
    double p3[] = {1.0, -23.0, 25.0, -0.26};

    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p0, 1);
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p0, 4);
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p2, 3);
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p3, 1);
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(NULL, NULL, 1);
#endif
}

void testNotEqualDoubleArraysExpectedNull(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double* p0 = NULL;
    double p1[] = {1.0, 8.0, 25.4, 0.252};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArraysExpectedNull(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double* p0 = NULL;
    double p1[] = {1.0, 8.0, 25.4, 0.252};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArraysActualNull(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.253};
    double* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArraysActualNull(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.253};
    double* p1 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArrays1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.25666666667};
    double p1[] = {1.0, 8.0, 25.4, 0.25666666666};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArrays1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.25666666667};
    double p1[] = {1.0, 8.0, 25.4, 0.25666666666};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArrays2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.253};
    double p1[] = {2.0, 8.0, 25.4, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArrays2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.253};
    double p1[] = {2.0, 8.0, 25.4, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArrays3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.253};
    double p1[] = {1.0, 8.0, 25.5, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArrays3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 8.0, 25.4, 0.253};
    double p1[] = {1.0, 8.0, 25.5, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArraysNegative1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -8.0, -25.4, -0.2566666667};
    double p1[] = {-1.0, -8.0, -25.4, -0.2566666666};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArraysNegative1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -8.0, -25.4, -0.2566666667};
    double p1[] = {-1.0, -8.0, -25.4, -0.2566666666};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArraysNegative2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -8.0, -25.4, -0.253};
    double p1[] = {-2.0, -8.0, -25.4, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArraysNegative2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -8.0, -25.4, -0.253};
    double p1[] = {-2.0, -8.0, -25.4, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleArraysNegative3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -8.0, -25.4, -0.253};
    double p1[] = {-1.0, -8.0, -25.5, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArraysNegative3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -8.0, -25.4, -0.253};
    double p1[] = {-1.0, -8.0, -25.5, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualDoubleArraysNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 0.0 / d_zero, 25.4, 0.253};
    double p1[] = {1.0, 0.0 / d_zero, 25.4, 0.253};

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
#endif
}

void testAssumeEqualDoubleArraysNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 0.0 / d_zero, 25.4, 0.253};
    double p1[] = {1.0, 0.0 / d_zero, 25.4, 0.253};

    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
#endif
}

void testEqualDoubleArraysInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 1.0 / d_zero, 25.4, 0.253};
    double p1[] = {1.0, 1.0 / d_zero, 25.4, 0.253};

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
#endif
}

void testAssumeEqualDoubleArraysInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 1.0 / d_zero, 25.4, 0.253};
    double p1[] = {1.0, 1.0 / d_zero, 25.4, 0.253};

    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 4);
#endif
}

void testNotEqualDoubleArraysLengthZero(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[1] = {0.0};
    double p1[1] = {0.0};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(p0, p1, 0);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleArraysLengthZero(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[1] = {0.0};
    double p1[1] = {0.0};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_DOUBLE_ARRAY(p0, p1, 0);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualDoubleEachEqual(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 1.0, 1.0, 1.0};
    double p1[] = {-0.123, -0.123, -0.123, -0.123};
    double p2[] = {25.4, 25.4, 25.4, -0.2};
    double p3[] = {1.0, -23.0, 25.0, -0.26};

    TEST_ASSERT_EACH_EQUAL_DOUBLE(1.0, p0, 1);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(1.0, p0, 4);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(-0.123, p1, 4);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(25.4, p2, 3);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(1.0, p3, 1);
#endif
}

void testAssumeEqualDoubleEachEqual(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 1.0, 1.0, 1.0};
    double p1[] = {-0.123, -0.123, -0.123, -0.123};
    double p2[] = {25.4, 25.4, 25.4, -0.2};
    double p3[] = {1.0, -23.0, 25.0, -0.26};

    TEST_ASSUME_EACH_EQUAL_DOUBLE(1.0, p0, 1);
    TEST_ASSUME_EACH_EQUAL_DOUBLE(1.0, p0, 4);
    TEST_ASSUME_EACH_EQUAL_DOUBLE(-0.123, p1, 4);
    TEST_ASSUME_EACH_EQUAL_DOUBLE(25.4, p2, 3);
    TEST_ASSUME_EACH_EQUAL_DOUBLE(1.0, p3, 1);
#endif
}

void testNotEqualDoubleEachEqualActualNull(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double* p0 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(5, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqualActualNull(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double* p0 = NULL;

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(5, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleEachEqual1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {0.253, 8.0, 0.253, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.253, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqual1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {0.253, 8.0, 0.253, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(0.253, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleEachEqual2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {8.0, 8.0, 8.0, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(8.0, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqual2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {8.0, 8.0, 8.0, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(8.0, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleEachEqual3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 1.0, 1.0, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(1.0, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqual3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0, 1.0, 1.0, 0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(1.0, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleEachEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -0.253, -0.253, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(-0.253, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqualNegative1(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-1.0, -0.253, -0.253, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(-0.253, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleEachEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-25.4, -8.0, -25.4, -25.4};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(-25.4, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqualNegative2(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-25.4, -8.0, -25.4, -25.4};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(-25.4, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testNotEqualDoubleEachEqualNegative3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-8.0, -8.0, -8.0, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(-8.0, p0, 4);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqualNegative3(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {-8.0, -8.0, -8.0, -0.253};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(-8.0, p0, 4);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testEqualDoubleEachEqualNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {0.0 / d_zero, 0.0 / d_zero, 0.0 / d_zero, 0.0 / d_zero};

    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0 / d_zero, p0, 4);
#endif
}

void testAssumeEqualDoubleEachEqualNaN(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {0.0 / d_zero, 0.0 / d_zero, 0.0 / d_zero, 0.0 / d_zero};

    TEST_ASSUME_EACH_EQUAL_DOUBLE(0.0 / d_zero, p0, 4);
#endif
}

void testEqualDoubleEachEqualInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0 / d_zero, 1.0 / d_zero, 25.4, 0.253};

    TEST_ASSERT_EACH_EQUAL_DOUBLE(1.0 / d_zero, p0, 2);
#endif
}

void testAssumeEqualDoubleEachEqualInf(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[] = {1.0 / d_zero, 1.0 / d_zero, 25.4, 0.253};

    TEST_ASSUME_EACH_EQUAL_DOUBLE(1.0 / d_zero, p0, 2);
#endif
}

void testNotEqualDoubleEachEqualLengthZero(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[1] = {0.0};

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, p0, 0);
    VERIFY_FAILS_END
#endif
}

void testAssumeNotEqualDoubleEachEqualLengthZero(void)
{
#ifdef UNITY_EXCLUDE_DOUBLE
    TEST_IGNORE();
#else
    double p0[1] = {0.0};

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EACH_EQUAL_DOUBLE(0.0, p0, 0);
    VERIFY_INCONCLUSIVE_END
#endif
}

void testDoublePrinting(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_EXCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_PRINT_FLOATING("0",             0.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("4.99e-07",      0.000000499);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("5.0000005e-07", 0.00000050000005);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("0.100469499",   0.100469499);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1",             0.9999999995); /*Rounding to int place*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1",             1.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1.25",          1.25);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("7.99999999",    7.99999999); /*Not rounding*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("16.0000002",    16.0000002);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("16.0000004",    16.0000004);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("16.0000006",    16.0000006);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("999999999",     999999999.0); /*Last full print integer*/

    TEST_ASSERT_EQUAL_PRINT_FLOATING("-0",             -0.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-4.99e-07",      -0.000000499);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-5.0000005e-07", -0.00000050000005);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-0.100469499",   -0.100469499);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-1",             -0.9999999995); /*Rounding to int place*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-1",             -1.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-1.25",          -1.25);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-7.99999999",    -7.99999999); /*Not rounding*/
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-16.0000002",    -16.0000002);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-16.0000004",    -16.0000004);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-16.0000006",    -16.0000006);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-999999999",     -999999999.0); /*Last full print integer*/

    TEST_ASSERT_EQUAL_PRINT_FLOATING("0.1004695",       0.10046949999999999);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("4.2949673e+09",   4294967295.9);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("4.2949673e+09",   4294967296.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1e+10",           9999999995.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("9.00719925e+15",  9007199254740990.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("7e+100",          7.0e+100);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("3e+200",          3.0e+200);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("9.23456789e+300", 9.23456789e+300);

    TEST_ASSERT_EQUAL_PRINT_FLOATING("-0.1004695",     -0.10046949999999999);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-4.2949673e+09", -4294967295.9);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-4.2949673e+09", -4294967296.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-7e+100",        -7.0e+100);
#endif
}

void testAssumeDoublePrinting(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_EXCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_PRINT_FLOATING("0",             0.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("4.99e-07",      0.000000499);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("5.0000005e-07", 0.00000050000005);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("0.100469499",   0.100469499);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1",             0.9999999995); /*Rounding to int place*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1",             1.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1.25",          1.25);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("7.99999999",    7.99999999); /*Not rounding*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("16.0000002",    16.0000002);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("16.0000004",    16.0000004);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("16.0000006",    16.0000006);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("999999999",     999999999.0); /*Last full print integer*/

    TEST_ASSUME_EQUAL_PRINT_FLOATING("-0",             -0.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-4.99e-07",      -0.000000499);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-5.0000005e-07", -0.00000050000005);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-0.100469499",   -0.100469499);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-1",             -0.9999999995); /*Rounding to int place*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-1",             -1.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-1.25",          -1.25);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-7.99999999",    -7.99999999); /*Not rounding*/
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-16.0000002",    -16.0000002);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-16.0000004",    -16.0000004);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-16.0000006",    -16.0000006);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-999999999",     -999999999.0); /*Last full print integer*/

    TEST_ASSUME_EQUAL_PRINT_FLOATING("0.1004695",       0.10046949999999999);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("4.2949673e+09",   4294967295.9);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("4.2949673e+09",   4294967296.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1e+10",           9999999995.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("9.00719925e+15",  9007199254740990.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("7e+100",          7.0e+100);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("3e+200",          3.0e+200);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("9.23456789e+300", 9.23456789e+300);

    TEST_ASSUME_EQUAL_PRINT_FLOATING("-0.1004695",     -0.10046949999999999);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-4.2949673e+09", -4294967295.9);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-4.2949673e+09", -4294967296.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-7e+100",        -7.0e+100);
#endif
}

void testDoublePrintingRoundTiesToEven(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_EXCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
  #ifdef UNITY_ROUND_TIES_AWAY_FROM_ZERO
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1.00000001e+10", 10000000050.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("9.00719925e+15", 9007199245000000.0);
  #else /* Default to Round ties to even */
    TEST_ASSERT_EQUAL_PRINT_FLOATING("1e+10",          10000000050.0);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("9.00719924e+15", 9007199245000000.0);
  #endif
#endif
}

void testAssumeDoublePrintingRoundTiesToEven(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_EXCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
  #ifdef UNITY_ROUND_TIES_AWAY_FROM_ZERO
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1.00000001e+10", 10000000050.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("9.00719925e+15", 9007199245000000.0);
  #else /* Default to Round ties to even */
    TEST_ASSUME_EQUAL_PRINT_FLOATING("1e+10",          10000000050.0);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("9.00719924e+15", 9007199245000000.0);
  #endif
#endif
}

void testDoublePrintingInfinityAndNaN(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_EXCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSERT_EQUAL_PRINT_FLOATING("inf",   1.0 / d_zero);
    TEST_ASSERT_EQUAL_PRINT_FLOATING("-inf", -1.0 / d_zero);

    TEST_ASSERT_EQUAL_PRINT_FLOATING("nan",   0.0 / d_zero);
#endif
}

void testAssumeDoublePrintingInfinityAndNaN(void)
{
#if defined(UNITY_EXCLUDE_FLOAT_PRINT) || defined(UNITY_EXCLUDE_DOUBLE) || !defined(USING_OUTPUT_SPY)
    TEST_IGNORE();
#else
    TEST_ASSUME_EQUAL_PRINT_FLOATING("inf",   1.0 / d_zero);
    TEST_ASSUME_EQUAL_PRINT_FLOATING("-inf", -1.0 / d_zero);

    TEST_ASSUME_EQUAL_PRINT_FLOATING("nan",   0.0 / d_zero);
#endif
}

// ===================== THESE TEST WILL RUN IF YOUR CONFIG INCLUDES DETAIL SUPPORT ==================

void testThatDetailsCanBeHandleOneDetail(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAIL("Detail1");

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_INT_MESSAGE(5, 6, "Should Fail And Say Detail1");
    VERIFY_FAILS_END
#endif
}

void testAssumeThatDetailsCanBeHandleOneDetail(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAIL("Detail1");

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_INT_MESSAGE(5, 6, "Should Fail And Say Detail1");
    VERIFY_INCONCLUSIVE_END
#endif
}

void testThatDetailsCanHandleTestFail(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAILS("Detail1","Detail2");

    EXPECT_ABORT_BEGIN
    TEST_FAIL_MESSAGE("Should Fail And Say Detail1 and Detail2");
    VERIFY_FAILS_END
#endif
}

void testThatDetailsCanHandleTestInconclusive(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAILS("Detail1","Detail2");

    EXPECT_ABORT_BEGIN
    TEST_INCONCLUSIVE_MESSAGE("Should Fail And Say Detail1 and Detail2");
    VERIFY_INCONCLUSIVE_END
#endif
}

void testThatDetailsCanBeHandleTwoDetails(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAILS("Detail1","Detail2");

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(7, 8, "Should Fail And Say Detail1 and Detail2");
    VERIFY_FAILS_END
#endif
}

void testAssumeThatDetailsCanBeHandleTwoDetails(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAILS("Detail1","Detail2");

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_HEX8_MESSAGE(7, 8, "Should Fail And Say Detail1 and Detail2");
    VERIFY_INCONCLUSIVE_END
#endif
}

void testThatDetailsCanBeHandleSingleDetailClearingTwoDetails(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAILS("Detail1","Detail2");
    UNITY_SET_DETAIL("DetailNew");

    EXPECT_ABORT_BEGIN
    TEST_ASSERT_EQUAL_STRING_MESSAGE("MEH", "GUH", "Should Fail And Say DetailNew");
    VERIFY_FAILS_END
#endif
}

void testAssumeThatDetailsCanBeHandleSingleDetailClearingTwoDetails(void)
{
#ifdef UNITY_EXCLUDE_DETAILS
    TEST_IGNORE();
#else
    UNITY_SET_DETAILS("Detail1","Detail2");
    UNITY_SET_DETAIL("DetailNew");

    EXPECT_ABORT_BEGIN
    TEST_ASSUME_EQUAL_STRING_MESSAGE("MEH", "GUH", "Should Fail And Say DetailNew");
    VERIFY_INCONCLUSIVE_END
#endif
}

