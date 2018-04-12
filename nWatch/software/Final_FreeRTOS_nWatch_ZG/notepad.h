#ifndef _NOTEPAD_H
#define _NOTEPAD_H

#include "global_inc.h"

#define ID_WINDOW_0    (GUI_ID_USER + 0x02)
#define ID_MULTIEDIT_0    (GUI_ID_USER + 0x03)
#define ID_TEXT_0    (GUI_ID_USER + 0x04)
#define ID_TEXT_1    (GUI_ID_USER + 0x05)

WM_HWIN Createnotepad(void);
static void _cbDialog(WM_MESSAGE * pMsg);
void Notepad( void * pvParameters);

#endif
