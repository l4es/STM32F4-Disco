#ifndef _CALC_H
#define _CALC_H

#include "global_inc.h"

#define GUI_ID_USER1  0x900

#define ID_WINDOW_0    (GUI_ID_USER1 + 0x00)
#define ID_TEXT_0    (GUI_ID_USER1 + 0x02)
#define ID_BUTTON_0    (GUI_ID_USER1 + 0x03)
#define ID_BUTTON_1    (GUI_ID_USER1 + 0x04)
#define ID_BUTTON_2    (GUI_ID_USER1 + 0x05)
#define ID_BUTTON_3    (GUI_ID_USER1 + 0x06)
#define ID_BUTTON_4    (GUI_ID_USER1 + 0x07)
#define ID_BUTTON_5    (GUI_ID_USER1 + 0x08)
#define ID_BUTTON_6    (GUI_ID_USER1 + 0x09)
#define ID_BUTTON_7    (GUI_ID_USER1 + 0x0A)
#define ID_BUTTON_8    (GUI_ID_USER1 + 0x0B)
#define ID_BUTTON_9    (GUI_ID_USER1 + 0x0C)
#define ID_BUTTON_10    (GUI_ID_USER1 + 0x0D)
#define ID_BUTTON_11    (GUI_ID_USER1 + 0x0E)
#define ID_BUTTON_12    (GUI_ID_USER1 + 0x0F)
#define ID_BUTTON_13    (GUI_ID_USER1 + 0x10)
#define ID_BUTTON_14    (GUI_ID_USER1 + 0x11)

WM_HWIN CreateCalc(void);
void Calc( void * pvParameters);


#endif
