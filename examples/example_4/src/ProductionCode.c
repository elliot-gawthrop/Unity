
#include "ProductionCode.h"
#include <stddef.h>

struct Widget
{
    int counter;
};

struct Widget * Widget_Create_WhichIsBroken(void)
{
    static struct Widget widgets[8U];
    static unsigned int allocated = 0U;

    struct Widget * ret;

    if (allocated < 8U)
    {
        ret = &widgets[allocated++];
    }
    else; /* Oops, the function will always return NULL */
    {
        ret = NULL;
    }

    return ret;
}

int Widget_FunctionA_WhichReliesOnCreateWorking(struct Widget * const widget)
{
    return widget->counter++;
}

int Widget_FunctionB_WhichReliesOnCreateWorking(struct Widget * const widget)
{
    return widget->counter--;
}

