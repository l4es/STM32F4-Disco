#ifndef _COUNTER_H
#define _COUNTER_H

#include "global_inc.h"

#define ID_FRAMEWIN_0  (GUI_ID_USER + 0x00)
#define ID_TEXT_0  (GUI_ID_USER + 0x01)
#define ID_TEXT_1  (GUI_ID_USER + 0x02)
#define ID_TEXT_2  (GUI_ID_USER + 0x03)
#define ID_GRAPH_0  (GUI_ID_USER + 0x05)
#define ID_TEXT_3  (GUI_ID_USER + 0x06)
#define ID_TEXT_4  (GUI_ID_USER + 0x07)
#define ID_TEXT_5  (GUI_ID_USER + 0x08)

static void _cbDialog(WM_MESSAGE * pMsg);
WM_HWIN Createcounter(void);
void Counter( void * pvParameters);

#endif
