
#include "ProductionCode.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_Widget_Create_WhichIsBroken(void)
{
    TEST_ASSERT_NOT_NULL(Widget_Create_WhichIsBroken());
}

void test_Widget_FunctionA_WhichReliesOnCreateWorking(void)
{
    struct Widget * widget = Widget_Create_WhichIsBroken();
    TEST_ASSUME_NOT_NULL(widget);
    TEST_ASSERT_EQUAL(0, Widget_FunctionA_WhichReliesOnCreateWorking(widget));
}

void test_Widget_FunctionB_WhichReliesOnCreateWorking(void)
{
    struct Widget * widget = Widget_Create_WhichIsBroken();
    TEST_ASSUME_NOT_NULL(widget);
    TEST_ASSERT_EQUAL(0, Widget_FunctionB_WhichReliesOnCreateWorking(widget));
}

