#ifndef UI_H
#define UI_H

#include "manager.h"

void ui_init(void);
void ui_clean(void);
void ui_refresh(manager_t *m);
int ui_get_input(void);

#endif
