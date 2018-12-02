
struct Widget;

struct Widget * Widget_Create_WhichIsBroken(void);
int Widget_FunctionA_WhichReliesOnCreateWorking(struct Widget * const widget);
int Widget_FunctionB_WhichReliesOnCreateWorking(struct Widget * const widget);
