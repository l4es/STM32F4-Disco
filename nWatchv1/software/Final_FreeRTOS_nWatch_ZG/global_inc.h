#ifndef _GLOBAL_INC
#define _GLOBAL_INC

#include <stm32f4xx.h>
#include <system_stm32f4xx.h>
#include <core_cm4.h>
#include "itoa.h"
#include "LCD_6300.h"
#include "string.h"
#include <misc.h>
#include <stm32f4xx_fsmc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_rtc.h>
#include <stm32f4xx_adc.h>
//#include <stm32f4xx_flash.h>
#include "jpeglib.h"
#include "decode.h"
#include "ff.h"
#include "tjpgd.h"
static TCHAR lfname[_MAX_LFN];
//////////USB//////////////////
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
//////////touch///////////////
#include "stmpe811.h"
#include "button.h"
#include "list1.h"
#include "textbox.h"
////////LE////////////////////
#include "stm32f4xx_pwr.h"
////////EXPLORER///////////
#include "explorer.h"
/////////////sram//////////////////
#include "sram.h"
////////graphic////////////
#include "bsp.h"
#include "GUIDRV_Template.h"
////////////////////////////////////SENSOR//////////////////////////////////////
#include <MPU5060.h>
////////////////////////////////////MUSIC////////////////////////////////////////////
#include "vs1003.h"
////////////////////////////////////FreeRTOS///////////////////////////////////////////
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "portmacro.h"
#include "queue.h"
#include "trcUser.h"
/////////////////////////////////////APPS//////////////////////////////////////////////
#include "mp3.h"
#include "manager.h"
#include "notepad.h"
#include "calc.h"
//#include "bar.h"
#include "settings.h"
#include "menuv2.h"
////////////////////////////////////RNG///////////////////////////////////////////
#include "stm32f4xx_rng.h"

#define CPU_ON GPIOD->BSRRL|=GPIO_BSRR_BS_3
#define CPU_OFF cpu_off()

#define LED_ON GPIOD->BSRRL|=GPIO_BSRR_BS_6
#define LED_OFF GPIOD->BSRRH|=GPIO_BSRR_BS_6
////////////////////////////////////BUTTONS///////////////////////////////////////////////////////
#define vol_up  (!((GPIOE->IDR) & GPIO_IDR_IDR_6))
#define vol_down  (!((GPIOA->IDR) & GPIO_IDR_IDR_1))
#define wake    (((GPIOB->IDR) & GPIO_IDR_IDR_12))

xTaskHandle                   Task_Handle;
xTaskHandle        			  Calc_Handle;
xTaskHandle                   Heading_Handle;
xTaskHandle                   Counter_Handle;
xTaskHandle                   Clock_Handle;
xTaskHandle                   MP3_Handle;
xTaskHandle                   Menu_Handle;
xTaskHandle                   Manager_Handle;
xTaskHandle        			  BSP_Handle;
xTaskHandle        			  Notepad_Handle;
xTaskHandle        			  Settings_Handle;
xTaskHandle        			  Alarm_Handle;
xTimerHandle                  TouchScreenTimer;

xQueueHandle                  xQueue_men;

char t[10];
int mem[101];

void cpu_off(void);

#endif
