#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "global_inc.h"

#define ID_WINDOW_0 (GUI_ID_USER + 0x00)
#define ID_SLIDER_0 (GUI_ID_USER + 0x01)

static void _cbDialog(WM_MESSAGE * pMsg);
WM_HWIN CreateSettings(void);
void Settings( void * pvParameters);

#endif
