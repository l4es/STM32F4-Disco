#ifndef _ALARM_H
#define _ALARM_H

#include "global_inc.h"

#define ID_WINDOW_0 (GUI_ID_1 + 0x17)
#define ID_BUTTON_0 (GUI_ID_1 + 0x18)
#define ID_BUTTON_1 (GUI_ID_1 + 0x19)
#define ID_BUTTON_2 (GUI_ID_1 + 0x1A)
#define ID_BUTTON_3 (GUI_ID_1 + 0x1B)
#define ID_CHECKBOX_0 (GUI_ID_1 + 0x1C)
#define ID_TEXT_0 (GUI_ID_1 + 0x1D)
#define ID_TEXT_1 (GUI_ID_1 + 0x1E)
#define ID_TEXT_2 (GUI_ID_1 + 0x1F)
#define ID_TEXT_3 (GUI_ID_1 + 0x20)

static void _cbDialog(WM_MESSAGE * pMsg);
WM_HWIN CreateAlarm(void);
void Alarm( void * pvParameters);

#endif
