#include "DIALOG.h"
#include "counter.h"
#include "clock.h"
#include "menu.h"
#include "heading.h"
#include "settings.h"

WM_HWIN hSettings;

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Settings", ID_WINDOW_0, 0, 0, 320, 240, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_0, 53, 186, 202, 30, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

static void _cbDialog(WM_MESSAGE * pMsg) {
  int NCode;
  int Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_SLIDER_0: // Notifications sent by 'Slider'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
    case WM_PAINT:
    {
  	  taskENTER_CRITICAL();
  	  LCD_BMP("0:dzien.bmp");
  	  taskEXIT_CRITICAL();
    	break;
    }
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}


WM_HWIN CreateSettings(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

void Settings( void * pvParameters)
{

	hSettings  = CreateSettings();
	if(Menu_Handle!=NULL)vTaskDelete(Menu_Handle);
	Menu_Handle=NULL;
	WM_HWIN hSlider = WM_GetDialogItem(hSettings, ID_SLIDER_0);
	SLIDER_SetValue(hSlider,50);

	while(1)
	{
		vTaskDelay(200);
		hSlider = WM_GetDialogItem(hSettings, ID_SLIDER_0);
		backlight(SLIDER_GetValue(hSlider));
	}
}
