#include "main.h"
#include <stm32f4xx.h>
#include <system_stm32f4xx.h>
#include <core_cm4.h>
#include "itoa.h"
#include "LCD_6300.h"
#include "ili9320.h"
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
#include <stm32f4xx_tim.h>
#include <stm32f4xx_iwdg.h>
#include <stm32f4xx_usart.h>
#include "jpeglib.h"
#include "decode.h"
#include "ff.h"
#include "tjpgd.h"
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
#include "clock.h"
#include "AB0805.h"
#include "iconview.h"
/////////////sram//////////////////
#include "sram.h"
////////graphic////////////
#include "bsp.h"
#include "GUIDRV_Template.h"
extern __IO int32_t OS_TimeMS;
#define CHECBOX_BUTTON_SIZE 20
////////////////////////////////////SENSOR//////////////////////////////////////
#include <MPU5060.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
////////////////////////////////////MUSIC////////////////////////////////////////////
#include "vs1003.h"
////////////////////////////////////FreeRTOS///////////////////////////////////////////
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "portmacro.h"
#include "trcUser.h"
/////////////////////////////////////APPS//////////////////////////////////////////////
#include "mp3.h"
#include "heading.h"
#include "menu.h"
////////////////////////////////////RNG///////////////////////////////////////////
#include "stm32f4xx_rng.h"
#include "global_inc.h"
//////////////////////////////////////BOOTLOADER//////////////////////////////////////////////////
#define SYSMEM_RESET_VECTOR            0x1fffC804
#define RESET_TO_BOOTLOADER_MAGIC_CODE 0xDEADBEEF
#define BOOTLOADER_STACK_POINTER       0x20002250

uint32_t dfu_reset_to_bootloader_magic;
/////////////////////////////////////USB///////////////////////////////////////////////////////
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_Core __ALIGN_END;
volatile uint8_t usb_host_connected_flag = 0;
volatile uint8_t usb_host_disconnected_flag = 0;

#define MAX_STRLEN 12 // this is the maximum string length of our string in characters
volatile char received_string[12]; // this will hold the recieved string
///////////////////////////////////////RCC//////////////////////////////////////////////////////////
#define PLL_Q	7			/* Dzielnik dla USB, SDIO, RND - 48MHz */
#define PLL_M	16
#define PLL_N	336 	    /* Glowny mnoznik - 336 MHz */
//#define PLL_N	400
#define PLL_P	2			/* Dzielnik przed VCO - 2 MHz */
/////////////////////////////////////LED's/////////////////////////////////////////////////////////
#define LED_ON GPIOD->BSRRL|=GPIO_BSRR_BS_6
#define LED_OFF GPIOD->BSRRH|=GPIO_BSRR_BS_6

//#define CPU_ON GPIOB->BSRRL|=GPIO_BSRR_BS_8
//#define CPU_OFF GPIOB->BSRRH|=GPIO_BSRR_BS_8
////////////////////////////////////BUTTONS///////////////////////////////////////////////////////
#define vol_up  (!((GPIOE->IDR) & GPIO_IDR_IDR_6))
#define vol_down  (!((GPIOA->IDR) & GPIO_IDR_IDR_1))
#define wake  	(((GPIOB->IDR) & GPIO_IDR_IDR_12))
///////////////////////////////////FUNCTIONS////////////////////////////////////////////////////
void EXTI9_5_IRQHandler(void)__attribute__((interrupt));
void TIM3_IRQHandler(void)__attribute__((interrupt));
void TIM2_IRQHandler(void)__attribute__((interrupt));
void RTC_Alarm_IRQHandler(void)__attribute__((interrupt));
void USART6_IRQHandler(void)__attribute__((interrupt));
void RCC_cfg (void);
void RCC_Config (void);
void GPIO_cfg(void);
void NVIC_cfg(void);
void delay(unsigned int ms);
void FSMC_init(void);
void GPIO_Config(void);
void SPI_Config(void);
void DMA_Config(int ele, int* buf);
void exti_init(void);
void delay_init(void);
int get_random(int form,int to);
void ADC1_Configuration(void);
void I2C1_init(void);
void RTC_Config(void);
void IWDG_Config(void);
void cb(u8 i, u8 l);
void dfu_run_bootloader(void);
void USART_puts( volatile char *s);

//////////////////////////////////////TASKs//////////////////////////////////////////////////////
static void Background_Task(void * pvParameters);
static void vTimerCallback( xTimerHandle pxTimer );
static void BSP_Task(void * pvParameters);
void bitmap_RGB(char *sc , u16 x, u16 y, u16 lx, u16 ly);
uint8_t pagebuff[10]__attribute((section(".ExRam")));
uint8_t buffe[23]__attribute((section(".ExRam")));
uint8_t aucLine[2048]__attribute((section(".ExRam")));
FRESULT f=0;
FIL fsrc;
DIR dir;
FATFS fs;
FILINFO fno;
volatile uint8_t update=0;
volatile int sec,sec2,sec3,cpu=1;
int16_t gyro[3], accel[3], sensors;
int32_t quat[4];
uint8_t more;
int s2=0;
touch ts;


int main(void)
{
	  SystemInit();
	  RCC_cfg();
	  GPIO_cfg();
	  SRAM_Init();
	  FSMC_NAND_Init();
	  delay_init();
	  init_USART(115200);
	  RNG_Cmd(ENABLE);
	  NVIC_Config();
//	  dfu_run_bootloader();
	  while(wake){};
	  LED_ON;
	  GPIOG->BSRRH|=GPIO_BSRR_BS_6;
	  GPIOG->BSRRH|=GPIO_BSRR_BS_8;
	  backlight(50);
	  IWDG_Config();
	  WM_SetCreateFlags(WM_CF_MEMDEV);
	  I2C1_init();
	  MPU6050_WriteBit(MPU6050_RA_INT_PIN_CFG , 7, 0);
	  stmpe811_init();
	  AB0805_initialize();
//	  AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL_WRTC, 0);
	  AB0805_WriteBit(devAddr, 0x0f,2,0);/////////////
	  AB0805_writeByte(devAddr, AB0805_RA_CONTROL1, 0x75);////////
//	  AB0805_setDateTime24(2016,10,12,23,35,0);
//	  AB0805_setDayOfWeek(7);
//	  USART_puts("TTM:REN-nWATCH");
//	  delay(50);
//	  USART_puts("TTM:ADP-(10)");
//	  delay(50);
	  USART_puts("TTM:TPL-(-6)");
	  f=f_mount(&fs,"",0);
	  LED_OFF;
//	  while(1)
//	  {
//		  IWDG_ReloadCounter();
//
//	  };
//	  VS1003_Config();
//	  Mp3Reset();
//	  VS1053_Start();
//	  VsSineTest();
//	  	f=f_open(&fsrc,"0:log.txt",FA_WRITE|FA_OPEN_EXISTING);
//	  	f=f_lseek(&fsrc,f_size(&fsrc));
//
//		f=f_write(&fsrc,itoa(time.RTC_Hours,t,10),2,&s2);
//		f=f_write(&fsrc,":",1,&s2);
//		f=f_write(&fsrc,itoa(time.RTC_Minutes,t,10),2,&s2);
//		f=f_write(&fsrc,":",1,&s2);
//		f=f_write(&fsrc,itoa(time.RTC_Seconds,t,10),2,&s2);
//		f_puts("\n", &fsrc);
//		f=f_close(&fsrc);

	  if(vol_up)
	  {
		  f_mount(&fs,"",0);
		  USBD_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
		  while(!wake)
		  {
			  sec=0;
			  CPU_ON;
			  IWDG_ReloadCounter();
		  };
	  }
//	  vTraceInitTraceData();
	  GUI_Init();
	  GUI_SetBkColor(GUI_WHITE);
	  WM_MOTION_Enable(1);
	  BUTTON_SetReactOnLevel();
//	  GUI_SetColor(GUI_RED);
//	  GUI_DispStringAt("dziala",20,20);
//	   {
//		   IWDG_ReloadCounter();
//		   if(touch_reg(&ts))
//		   {
//		   GUI_DispDecAt(ts.x,10,10,5);
//		   GUI_DispDecAt(ts.y,10,40,5);
//		   }
//		   delay(100);
//	   };
//	  LCD_BMP(&fsrc,"0:jas.bmp");

	GPIO_InitTypeDef gpio;
	ADC_InitTypeDef adc;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	ADC1_Configuration();



//		vTraceStart();
	  xTaskCreate(Clock,(char const*)"Clock",1024,NULL,7,&Clock_Handle);
//	  xTaskCreate(Menu,(char const*)"Menu",512,NULL,7, &Menu_Handle);
//	  xTaskCreate(Heading_Task,(char const*)"Heading",512,NULL, 6, &Heading_Handle);
//	  xTaskCreate(BSP_Task,(char const*)"BSP",1024,NULL, 7, &BSP_Handle);
	  xTaskCreate(Heading_Task,(char const*)"Heading",1024,NULL, 6, &Heading_Handle);
	  xTaskCreate(Background_Task,(char const*)"Background",1024,NULL, 7,&Task_Handle);
	  TouchScreenTimer = xTimerCreate ("Timer",50, pdTRUE,1, vTimerCallback);
	  xTimerStart( TouchScreenTimer, 0);
	  vTaskStartScheduler();
}

void dfu_run_bootloader()
{
	*((unsigned long *)0x2000FFF0) = 0xDEADBEEF;
    NVIC_SystemReset();
}
void cb(u8 i, u8 l)
{
	u8 a=0;
	LED_OFF;
}
static void vTimerCallback( xTimerHandle pxTimer )
{
	  taskENTER_CRITICAL();
	  if(BSP_Pointer_Update())sec=0;
	  taskEXIT_CRITICAL();
}
void bitmap_RGB(char *sc , u16 x, u16 y, u16 lx, u16 ly)
{
	LCD_WRITE_COMMAND=(MADCTR);
	LCD_WRITE_DATA = (0x86);

	  f = f_open(&fsrc,sc, FA_READ | FA_OPEN_EXISTING );
	  int read= lx*ly*3;
	  int s1=0;
	  int BUF=2048;
	  LCD_area(x,y,x+lx-1,y+ly-1);
//	  LCD_area(y,x,y+ly-1,x+lx-1);

	  while(read>0)
	  {
		  f = f_read(&fsrc, aucLine, BUF, &s1);

		  DMA_Config(BUF,aucLine);
		  while(!(DMA2->LISR & DMA_LISR_TCIF0)){};
		  DMA2->LISR=0x00000000;
		  read-=BUF;
	  }

		LCD_WRITE_COMMAND=(MADCTR);
		LCD_WRITE_DATA = (0x66);

}
static void BSP_Task(void * pvParameters)
{
	portTickType xLastFlashTime;
	xLastFlashTime = xTaskGetTickCount();

	  while(1)
	  {
		  	vTaskDelay(100);
		    ADC_SoftwareStartConv(ADC1);
		    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		    float result= ADC_GetConversionValue(ADC1);
		    float volt = result /4095 * 240;
		    GUI_SetColor(GUI_BLACK);
			 GUI_DispStringHCenterAt(itoa(volt,t,10), 320 / 2, 20);
	  }
}
static void Background_Task(void * pvParameters)
{
//	portTickType xLastFlashTime;
//	xLastFlashTime = xTaskGetTickCount();
//	xQueue_men = xQueueCreate(10, sizeof(int));
	  while(1)
	  {
//		  vTaskDelay(200);
		  CPU_ON;
		  GUI_Delay(100);
//		  GUI_Exec1();
		  IWDG_ReloadCounter();
	  	  if(wake || !cpu)
	  	  {
	  		  CPU_OFF;
	  	  }
//		  if(vol_down)
//		  {
////			  taskENTER_CRITICAL();
//			  USBD_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
//			  while(!wake)
//			  {
//				  sec=0;
//				  CPU_ON;
//				  IWDG_ReloadCounter();
//			  };
////			  taskEXIT_CRITICAL();
//		  }
	  }
}
void exti_init(void)
{
	EXTI->IMR= EXTI_IMR_MR5;
	EXTI->RTSR = EXTI_RTSR_TR5;
    NVIC_SetPriority(EXTI9_5_IRQn,1 );
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    SYSCFG ->EXTICR[1] = SYSCFG_EXTICR2_EXTI5_PC;
}

void DMA_Config(int ele, int* buf)
{

	DMA_InitTypeDef SDDMA_InitStructure;
//	DMA2->LIFCR=0x00000000;
//	DMA2->HIFCR=0x00000000;
	/* DMA2 Stream3  or Stream6 disable */
//	DMA_Cmd(DMA2_Stream0, DISABLE);
	DMA_Cmd(DMA2_Stream0, ENABLE);
	/* DMA2 Stream3  or Stream6 Config */
	DMA_DeInit(DMA2_Stream0);

	SDDMA_InitStructure.DMA_Channel = DMA_Channel_0;
	SDDMA_InitStructure.DMA_PeripheralBaseAddr = buf;
	SDDMA_InitStructure.DMA_Memory0BaseAddr =&LCD_WRITE_DATA;  /////nokia
//	SDDMA_InitStructure.DMA_Memory0BaseAddr =&LCD_RAM;   ////ili9320
	SDDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
	SDDMA_InitStructure.DMA_BufferSize = ele;
	SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;////////////////////////
	SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; /* DMA_MemoryDataSize_Word; */
	SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_3QuartersFull; /* DMA_FIFOThreshold_Full */
	SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; /* DMA_MemoryBurst_INC4 */
	SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init (DMA2_Stream0, &SDDMA_InitStructure);
	/* DMA2 Stream3  or Stream6 enable */
	DMA_Cmd(DMA2_Stream0, ENABLE);

}
void backlight( int pwm)
{
//	TIM3->PSC =   1000;
//	TIM3->ARR =   1;
//	TIM3->CCR4 =  33;
////	TIM3->CCR4 =  1;
//	TIM3->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2|TIM_CCMR2_OC4PE;
//	TIM3->CCER |= TIM_CCER_CC4P;
////	TIM3->CCER |= TIM_CCER_CC4E;
//	TIM3->CR1 =   TIM_CR1_CEN;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 99;
    TIM_TimeBaseStructure.TIM_Prescaler = 167;  //fclk = 72M/72M - 1 = 0
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;    //0 = nie dzielimy zegara
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //tryb zliczania w górę

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;

  // Konfiguracja kanalu 4 ktory jest wyprowadzony na PB1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //Sygnał z timera będzie wykorzystywany do sterowania kontrolerem przerwań więc musi być Enable
    TIM_OCInitStructure.TIM_Pulse = pwm; //90% PWM
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;   //stan wysoki
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3,ENABLE);
    // Wlaczenie timera
    TIM_Cmd(TIM3, ENABLE);
}

void IR_config(void)
{
	TIM4->PSC = 1;
	TIM4->ARR = 1130; //1166
	TIM4->CCR1 = 350; ///583
	TIM4->CCMR1|= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1PE;
	TIM4->CCER|= TIM_CCER_CC1NE|TIM_CCER_CC1P;
	TIM4->CCER|= TIM_CCER_CC1E;
//	TIM4->CR1&=~TIM_CR1_CEN;
}

void delay_init(void)
{
	TIM2->PSC= 168;
	TIM2->ARR= 1000;
	TIM2->DIER|= TIM_DIER_UIE;
	TIM2->CR1|=TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM2_IRQn);
}
void TIM2_IRQHandler(void)
{
	sec++;
	sec2++;
	if(sec3>0)sec3--;
	if(sec2==1300)sec=0;
	if(sec==5000)
	{
		cpu=0;
//		sec=0;
	}
	TIM2->SR = (uint16_t)~TIM_SR_UIF;
}
void delay(unsigned int ms)
{
	sec3=ms;
	while(sec3>0){};
}
void IWDG_Config(void)
{
	//konfigurowanie watchdoga IWDG
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);    // Zezwolenie na zapis rejestrów IWDG
	IWDG_SetPrescaler(IWDG_Prescaler_16);             // IWDG taktowany zegarem: 40kHz/4 = 10kHz
	IWDG_SetReload(0xFFF);                        // Przepelnienie IWDG po okolo 0,4s
	IWDG_ReloadCounter();                           // Przeladowanie IWDG
	IWDG_Enable();                                  // Wlaczenie IWDG i LSI
}
void RTC_Config(void)
{
		PWR_BackupAccessCmd(ENABLE);							//Wlaczenie dostepu do rejestrow Backup Domain
		RCC_LSEConfig(RCC_LSE_ON);								//Wlaczenie oscylatora LSE
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//UStawienie LSE jako zrodla sygnalu zegarowego dla RTC
		RCC_RTCCLKCmd(ENABLE);										//Wlaczenie taktowania RTC

		RTC_WaitForSynchro();											//Oczekiwanie na synchronizacje
	//	RTC_WaitForLastTask();										//Oczekiwanie az rejestry RTC zostana zapisane								//Ustawienie prescalera Okres RTC = 1s = RTCCLK/RTC_PR = 32.768Hz/32767+1
		RTC->PRER = 0x7fff;
//		RTC_ITConfig(RTC_Alarm_A, ENABLE);
		RCC_ClearFlag();
//		RTC_TimeTypeDef  time;
//		time.RTC_Minutes=0;
//		time.RTC_Seconds=0;
//		time.RTC_Hours=0;
//		RTC_SetTime(RTC_Format_BIN,&time);
//		RTC_DateTypeDef date;
//		date.RTC_Year=2016;
//		date.RTC_Month=1;
//		date.RTC_WeekDay=1;
//		date.RTC_Date=1;



	    RTC_TimeTypeDef RTC_TimeStructure;
	    RTC_DateTypeDef RTC_DateStructure;
//	    EXTI_InitTypeDef EXTI_InitStructure;
//
//	    RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
//	    RTC_TimeStructure.RTC_Hours   = 19;
//	    RTC_TimeStructure.RTC_Minutes = 1;
//	    RTC_TimeStructure.RTC_Seconds = 00;
//	    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

//	    /* Indicator for the RTC configuration */
//	    RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
//
////        RTC_TimeStructure.RTC_Seconds = 0;
////        RTC_TimeStructure.RTC_Minutes = 9;
////        RTC_TimeStructure.RTC_Hours = 4;
////        RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
////        RTC_SetTime(RTC_Format_BCD,&RTC_TimeStructure);
////
//        RTC_DateStructure.RTC_Date = 22;
//        RTC_DateStructure.RTC_Month = 9;
//        RTC_DateStructure.RTC_WeekDay= RTC_Weekday_Thursday;
//        RTC_DateStructure.RTC_Year = 16;
//        RTC_SetDate(RTC_Format_BIN,&RTC_DateStructure);
//        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//上升沿
//        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//        EXTI_InitStructure.EXTI_Line = EXTI_Line17;
//        EXTI_Init(&EXTI_InitStructure);

//		RTC_AlarmTypeDef RTC_AlarmStructure;
//	    RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
//	    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours=1;
//	    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes=1;
//	    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds=5;
//	    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12=RTC_H12_AM;
//	    //RTC_AlarmStructure.RTC_AlarmMask=RTC_AlarmMask_All;
//	    RTC_AlarmStructure.RTC_AlarmMask=RTC_AlarmMask_DateWeekDay|RTC_AlarmMask_Hours|RTC_AlarmMask_Minutes;

//	  RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
//	  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
//	  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
//	  RTC_ClearITPendingBit(RTC_IT_ALRA);
//	  RTC_ClearFlag(RTC_FLAG_ALRAF);
}
void RCC_cfg(void)
{
   /* Pin MCO taktowany z PLL, prescaler 3 - 50MHz */
	RCC->CFGR = RCC_CFGR_MCO1PRE_2 | RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1;

	/* Uruchamiamy HSE i czekamy na gotowosc */
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY));

//		RCC->CR |= RCC_CR_HSION;
//		while (!(RCC->CR & RCC_CR_HSIRDY))

	/* Konfiguracja flasha */
	FLASH->ACR = FLASH_ACR_ICEN |       /* instruction cache                   */
			FLASH_ACR_DCEN |              /* data cache                          */
			FLASH_ACR_PRFTEN |            /* prefetch enable                     */
			FLASH_ACR_LATENCY_4WS;        /* 4 wait states                       */

	/* Konfiguracja PLL HSE jako zrodlo PLL, ustawienie dzielnikow Q, M, N, P  */

	RCC->PLLCFGR = (PLL_Q << 24) | RCC_PLLCFGR_PLLSRC_HSE |
			(((PLL_P >> 1) - 1) << 16) | (PLL_N << 6) | PLL_M;

	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY))
		;

	/* PLL jak sygnal taktowania rdzenia, psk 2 dla APB2, psk 4 dla APB1 */
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_SW_PLL;
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL))
		;

	/* Inicjalizacja koprocesora FPU */
	SCB->CPACR |= ((3 << 10*2)|(3 << 11*2));

	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	 RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	 RCC->AHB1ENR |= RCC_AHB1Periph_DMA2;
	 RCC->AHB3ENR |= RCC_AHB3ENR_FSMCEN;
	 RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	 RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	 RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	 RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
	 RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
	 RCC->APB2ENR |= RCC_APB2ENR_SDIOEN;
//	 RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
}

void NVIC_Config(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);

	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;		 // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

	// finally this enables the complete USART1 peripheral

	NVIC_Init(&NVIC_InitStructure);

}


void RTC_Alarm_IRQHandler(void)
{
//	CPU_OFF;
	RTC_ClearITPendingBit(RTC_IT_ALRA);
	EXTI_ClearITPendingBit(EXTI_Line17);
}


void GPIO_cfg(void)
{
	GPIOB->MODER=0x00000000;
	GPIOG->MODER=0x00000000;

	GPIOD->MODER|=GPIO_MODER_MODER3_0;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR3;
	GPIOD->ODR|=GPIO_ODR_ODR_3;
	CPU_ON;

	//*****************LED_CONFIG*******************

	GPIOD->MODER|=GPIO_MODER_MODER6_0;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR6;
	GPIOD->ODR|=GPIO_ODR_ODR_6;

	GPIOC->MODER|=GPIO_MODER_MODER13_0;                     /////backlight
	GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR13;
	GPIOC->ODR|=GPIO_ODR_ODR_13;

	//******************LCD_CONFIG**************************

	GPIOD->MODER|=GPIO_MODER_MODER0_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR0;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource0, GPIO_AF_FSMC);

	GPIOD->MODER|=GPIO_MODER_MODER1_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR1;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource1, GPIO_AF_FSMC);

	GPIOD->MODER|=GPIO_MODER_MODER4_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR4;
//	GPIOD->PUPDR|=GPIO_PUPDR_PUPDR4_0;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource4, GPIO_AF_FSMC);

	GPIOD->MODER|=GPIO_MODER_MODER5_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR5;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5, GPIO_AF_FSMC);

//	GPIOD->MODER|=GPIO_MODER_MODER7_1;
//	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR7;
//	GPIO_PinAFConfig(GPIOD,GPIO_PinSource7, GPIO_AF_FSMC);

	GPIOD->MODER|=GPIO_MODER_MODER11_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR11;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource11, GPIO_AF_FSMC);

	GPIOD->MODER|=GPIO_MODER_MODER14_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR14;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14, GPIO_AF_FSMC);

	GPIOD->MODER|=GPIO_MODER_MODER15_1;
	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR15;
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15, GPIO_AF_FSMC);

	GPIOE->MODER|=GPIO_MODER_MODER7_1;
	GPIOE->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR7;
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource7, GPIO_AF_FSMC);

	GPIOE->MODER|=GPIO_MODER_MODER8_1;
	GPIOE->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR8;
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource8, GPIO_AF_FSMC);

	GPIOE->MODER|=GPIO_MODER_MODER9_1;
	GPIOE->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR9;
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9, GPIO_AF_FSMC);

	GPIOE->MODER|=GPIO_MODER_MODER10_1;
	GPIOE->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR10;
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource10, GPIO_AF_FSMC);

	GPIOA->MODER|=GPIO_MODER_MODER2_0;
	GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR2;     ////lcd RST
	GPIOA->ODR|=GPIO_ODR_ODR_2;
//	GPIOA->BSRRL|=GPIO_BSRR_BS_2;

	//////////////////////////backlight///////////////////////////////
	GPIOB->MODER|=GPIO_MODER_MODER1_1;
	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR1;
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource1,GPIO_AF_TIM3);
	backlight(0);
	//////////////////////////ON'y-PB9////////////////////////////////////
	GPIOB->MODER|=GPIO_MODER_MODER9_0;
	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR9_1;
	GPIOB->ODR|=GPIO_ODR_ODR_9;
	GPIOB->BSRRL|=GPIO_BSRR_BS_9;

	GPIOF->MODER|=GPIO_MODER_MODER11_0;
	GPIOF->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR11;
	GPIOF->ODR|=GPIO_ODR_ODR_11;
	GPIOF->BSRRL|=GPIO_BSRR_BS_11;

	GPIOB->MODER|=GPIO_MODER_MODER14_0;
	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR14;
	GPIOB->ODR|=GPIO_ODR_ODR_14;
	GPIOB->BSRRL|=GPIO_BSRR_BS_14;

//	GPIOD->MODER|=GPIO_MODER_MODER7_0;
//	GPIOD->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR7_1;
//	GPIOD->ODR|=GPIO_ODR_ODR_7;
//	GPIOD->BSRRH|=GPIO_BSRR_BS_7;

	GPIOF->MODER|=GPIO_MODER_MODER6_0;
	GPIOF->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR6;
	GPIOF->ODR|=GPIO_ODR_ODR_6;
	GPIOF->BSRRL|=GPIO_BSRR_BS_6;

	GPIOB->MODER|=GPIO_MODER_MODER3_0;
	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR3_1;
	GPIOB->ODR|=GPIO_ODR_ODR_3;
	GPIOB->BSRRL|=GPIO_BSRR_BS_3;

	GPIOF->MODER|=GPIO_MODER_MODER9_0;
	GPIOF->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR9_1;
	GPIOF->ODR|=GPIO_ODR_ODR_9;
	GPIOF->BSRRH|=GPIO_BSRR_BS_9;
	//////////////////////////BLUETOOTH//////////////////////////////////
	GPIOG->MODER|=GPIO_MODER_MODER8_0;         ///////////////////ENABLE PIN
	GPIOG->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR8;
	GPIOG->ODR|=GPIO_ODR_ODR_8;
	GPIOG->BSRRH|=GPIO_BSRR_BS_8;

	GPIOG->MODER|=GPIO_MODER_MODER6_0;         ///////////////////BRTS
	GPIOG->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR6_1;
	GPIOG->ODR|=GPIO_ODR_ODR_6;
	GPIOG->BSRRL|=GPIO_BSRR_BS_6;
	//////////////////////////TSOP-PB14///////////////////////////////////
//	GPIOB->MODER
	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR14;
	GPIOB->PUPDR |=GPIO_PUPDR_PUPDR14_0;
	//////////////////////////LED////////////////////////////////////////
	GPIOB->MODER|=GPIO_MODER_MODER6_1;
	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR6;
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_TIM4);
	/////////////////////// button///////////////////////////////////////
	GPIOE->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR6;
	GPIOE->PUPDR |=GPIO_PUPDR_PUPDR6_0;

	GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR1;
	GPIOA->PUPDR |=GPIO_PUPDR_PUPDR1_0;

	GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR12;
//	GPIOB->PUPDR |=GPIO_PUPDR_PUPDR12_1;
	///////////////////////TOUCH_INT////////////////////////////////////
	GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR5_0;
//	GPIOC->OTYPER|=GPIO_OTYPER_IDR_5;
	GPIOC->PUPDR|=GPIO_PUPDR_PUPDR5_0;
	////////////////////////////////////////////////////////////////////

	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DREQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DREQ_PORT, &GPIO_InitStructure);
	 LED_OFF;


}
void ADC1_Configuration(void)
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef       ADC_InitStructure;

  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;

  ADC_CommonInit(&ADC_CommonInitStructure);

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;

  ADC_StructInit(&ADC_InitStructure);

  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_480Cycles);
  ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);

  ADC_Cmd(ADC1, ENABLE);    //The ADC is powered on by setting the ADON bit in the ADC_CR2 register.
  //When the ADON bit is set for the first time, it wakes up the ADC from the Power-down mode.
}
void I2C1_init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* setup SCL and SDA pins
	 * You can connect the I2C1 functions to two different
	 * pins:
	 * 1. SCL on PB6 or PB8
	 * 2. SDA on PB7 or PB9
	 */
	 // we are going to use PB6 and PB9
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB

	// Connect I2C1 pins to AF
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1); // SDA

    I2C_InitTypeDef i2c_init;
    I2C_DeInit(I2C1 );       //Deinit and reset the I2C to avoid it locking up
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    int var;
    for ( var= 0; var < 0xff; ++var) {};
    I2C_SoftwareResetCmd(I2C1, DISABLE);

    i2c_init.I2C_ClockSpeed = 400000;
    i2c_init.I2C_Mode = I2C_Mode_I2C;
    i2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c_init.I2C_OwnAddress1 = 0xD0;
    i2c_init.I2C_Ack = I2C_Ack_Enable;
    i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &i2c_init);

//    I2C_StretchClockCmd(I2C2, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB

	// Connect I2C1 pins to AF
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2); // SDA


    I2C_DeInit(I2C2 );       //Deinit and reset the I2C to avoid it locking up
    I2C_SoftwareResetCmd(I2C2, ENABLE);
    for ( var= 0; var < 0xff; ++var) {};
    I2C_SoftwareResetCmd(I2C2, DISABLE);

    i2c_init.I2C_ClockSpeed = 400000;////////2
    i2c_init.I2C_Mode = I2C_Mode_I2C;
    i2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c_init.I2C_OwnAddress1 = 0x69;
    i2c_init.I2C_Ack = I2C_Ack_Enable;
    i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &i2c_init);

    I2C_Cmd(I2C2, ENABLE);
}

void USBH_ConnectEventCallback(void)
{
	usb_host_connected_flag = 1;
}
void USBH_DisconnectEventCallback(void)
{
	usb_host_disconnected_flag = 1;
}
int get_random(int from,int to)
{
	  int rnd;
	  while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
	  rnd=RNG_GetRandomNumber()%to + from;
	  RNG_ClearFlag(RNG_SR_DRDY);
	  return rnd;
}

int bufftoint(char *buff)
{
	int x=buff[3];
	x=x<<8;
	x=x|buff[2];
	x=x<<8;
	x=x|buff[1];
	x=x<<8;
	x=x|buff[0];

	return x;
}
u8 play_video(char *sc)
{
	u8 flag=1;
	int nobytsread=0;
	long long int indexx=0,movilocation=0,framesize=0,framelocation=0;
	FRESULT fres=0;
	FATFS fsrc;

//	NVIC_DisableIRQ(EXTI9_5_IRQn);
	fres=f_open(&fsrc,sc, FA_READ | FA_OPEN_EXISTING);
	if(fres==0)fres=f_read(&fsrc,pagebuff,4,&nobytsread);

	int act=f_size(&fsrc);

	while(pagebuff[0]!='m' || pagebuff[1]!='o'|| pagebuff[2]!='v'|| pagebuff[3]!='i')
	{
	    	indexx++;
	        fres=f_lseek(&fsrc,indexx); // pointer forward
	    	fres=f_read(&fsrc,pagebuff,4,&nobytsread);
	 }
	       indexx+=4;
	       movilocation=indexx;       //movi tag location set
	       indexx=f_size(&fsrc);
//	 	  BUTTON_Handle hButton = BUTTON_Create(110, 110, 100, 60, GUI_ID_SKIP, WM_CF_SHOW);
//	 	  BUTTON_SetText(hButton, "Skip the video");
	while(pagebuff[0]!='i' || pagebuff[1]!='d'|| pagebuff[2]!='x'|| pagebuff[3]!='1')
	{
	       	  indexx--;
	          fres=f_lseek(&fsrc,indexx); // pointer forward
	       	  fres=f_read(&fsrc,pagebuff,4,&nobytsread);
	}

	  cinfo.err = jpeg_std_error(&jerr);
	  jpeg_create_decompress(&cinfo);

	while(flag==1)
	{

		while(pagebuff[0]!='0' || pagebuff[1]!='0'|| pagebuff[2]!='d'|| pagebuff[3]!='c')   ///dc - compressed    db-decompressed
		{
				  indexx++;
				  fres=f_lseek(&fsrc,indexx); // pointer forward
				  fres=f_read(&fsrc,pagebuff,4,&nobytsread);

				  if(indexx>=act)
				{
					  flag=0;
					  jpeg_destroy_decompress(&cinfo);
					  return 0;
				}
		}

	          indexx+=8;          //4bytes from 00dc---->dwflag +4bytes from dwflag--->dwoffset=frame_loaction
	          fres=f_lseek(&fsrc,indexx);
	          fres=f_read(&fsrc,pagebuff,4,&nobytsread);
	          framelocation=movilocation+bufftoint(pagebuff);
	          f_lseek(&fsrc,framelocation);//length+data
	          fres=f_read(&fsrc,pagebuff,4,&nobytsread);
	          framesize=bufftoint(pagebuff);
	          framelocation+=4;  //only data length removed.
	          f_lseek(&fsrc,framelocation);//data     locating finished
	          jpeg_decode(&fsrc, 320, aucLine, NULL);
//	          load_jpg(&fsrc,"-",Buff,sizeof(Buff));
	          f_lseek(&fsrc,indexx);
	}

	jpeg_destroy_decompress(&cinfo);
	return 0;
}

void vApplicationMallocFailedHook( void )
{
  while (1)
  {
//	  ili9320_String_lc("MALLOC FAILED",5,10,RED,BLACK,2);
//		if(wake)
//		{
//			CPU_OFF;
//		}
  }
}
void vApplicationIdleHook(void)
{
//	int i=0;
	while(1)
	{
//		if(wake)
//		{
//			CPU_OFF;
//		}


//		vTaskDelay(10);
//		i++;

//		if(i>9000000)
//		{
//			ili9320_String_lc("IN IDLE",5,10,RED,GREEN,2);
//			GUI_DispStringAt("IN IDLE",50,50);
//			LCD_box_mid_fill(318,238,4,4,RED);
//			i=0;
//		}
//		LCD_box_mid_fill(0,0,5,5,RED);
//		GUI_DispStringAt("IN IDLE",50,50);
//		LCD_String_lc("in idle",5,10,RED,GREEN,2);

	}
}
void vApplicationStackOverflowHook (void)
{

	while(1)
	{
		LED_OFF;
	}
}
void init_USART(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

	/* enable APB2 peripheral clock for USART1
	 * note that only USART1 and USART6 are connected to APB2
	 * the other USARTs are connected to APB1
	 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	/* enable the peripheral clock for the pins used by
	 * USART1, PB6 for TX and PB7 for RX
	 */

	/* This sequence sets up the TX and RX pins
	 * so they work correctly with the USART1 peripheral
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6 (TX) and 7 (RX) are used
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// this activates the pullup resistors on the IO pins
	GPIO_Init(GPIOC, &GPIO_InitStruct);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers

	/* The RX and TX pins are now connected to their AF
	 * so that the USART1 can take over control of the
	 * pins
	 */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); //
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	/* Now the USART_InitStruct is used to define the
	 * properties of USART1
	 */
	USART_InitStruct.USART_BaudRate = baudrate;				// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(USART6, &USART_InitStruct);					// again all the properties are passed to the USART_Init function which takes care of all the bit setting


	/* Here the USART1 receive interrupt is enabled
	 * and the interrupt controller is configured
	 * to jump to the USART1_IRQHandler() function
	 * if the USART1 receive interrupt occurs
	 */

	USART_Cmd(USART6, ENABLE);

}
void USART_puts( volatile char *s)
{

	while(*s)
	{
		// wait until data register is empty
		while( !(USART6->SR & 0x00000040) );
		USART_SendData(USART6, *s);
		*s++;
	}
}
void USART6_IRQHandler(void)
{

	// check if the USART1 receive interrupt flag was set
	if( USART_GetITStatus(USART6, USART_IT_RXNE) )
	{
		static uint8_t cnt = 0; // this counter is used to determine the string length
		char t = USART6->DR; // the character from the USART1 data register is saved in t
		if(t=='*')cnt=0;
		else if(t=='!')
		{
			AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL_WRTC, 1);
			AB0805_setMonth((received_string[0]-'0')*10+(received_string[1]-'0'));
			AB0805_setDay((received_string[2]-'0')*10+(received_string[3]-'0'));
			AB0805_setDayOfWeek((received_string[4]-'0'));
			AB0805_setHours24((received_string[5]-'0')*10+(received_string[6]-'0'));
			AB0805_setMinutes((received_string[7]-'0')*10+(received_string[8]-'0'));
			AB0805_setSeconds((received_string[9]-'0')*10+(received_string[10]-'0'));
			AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL_WRTC, 0);
		}
		else
		{
			received_string[cnt] = t;
			cnt++;
		}
	}
}
//	  USBD_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
//	  while(1){};



//	  GUI_SelectLayer(1);

//	  GUI_SelectLayer(0);

//	    load_jpg(&fsrc,"0:papiez.jpg",aucLine,sizeof(aucLine));
//	  	jpeg_create_decompress(&cinfo);
//		cinfo.err = jpeg_std_error(&jerr);
//	    f=f_open(&fsrc,"0:papiez.jpg", FA_READ | FA_OPEN_EXISTING );
//	    jpeg_decode(&fsrc, 320, aucLine, 0);
//	    jpeg_finish_decompress(&cinfo);
//
//	    f=f_open(&fsrc,"0:big.jpg", FA_READ | FA_OPEN_EXISTING );
//	    jpeg_decode(&fsrc, 320, _aucLine, 0);
//	    play_video("bird19.avi");
//	    while(1)
//	    {

//		    f=f_open(&fsrc,"0:moje/car2.jpg", FA_READ | FA_OPEN_EXISTING );
//			LCD_SetCursor(0,0);
//			LCD_WriteIndex(R34);
//		    jpeg_decode(&fsrc, 320, _aucLine, 0);
//		    f=f_open(&fsrc,"0:moje/car1.jpg", FA_READ | FA_OPEN_EXISTING );
//			LCD_SetCursor(0,0);
//			LCD_WriteIndex(R34);
//		    jpeg_decode(&fsrc, 320, _aucLine, 0);
//		    f=f_open(&fsrc,"0:moje/car.jpg", FA_READ | FA_OPEN_EXISTING );
//			LCD_SetCursor(0,0);
//			LCD_WriteIndex(R34);
//		    jpeg_decode(&fsrc, 320, _aucLine, 0);
//	    }
//


//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
//	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
//	 RCC->APB1ENR |= RCC_APB1ENR_PWREN;
//	   GPIO_InitTypeDef GPIO_InitStructure;
//	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
//	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//	   GPIO_Init(GPIOA, &GPIO_InitStructure);
//	   GPIO_Init(GPIOB, &GPIO_InitStructure);
//	   GPIO_Init(GPIOC, &GPIO_InitStructure);
//	   GPIO_Init(GPIOD, &GPIO_InitStructure);
//	   GPIO_Init(GPIOE, &GPIO_InitStructure);
//	   GPIO_Init(GPIOF, &GPIO_InitStructure);
//	   GPIO_Init(GPIOG, &GPIO_InitStructure);
//	  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

//	  PWR_EnterSTANDBYMode();
//	   __WFI();
//	  LCD_String("blalal",100,100,1,RED,BLUE,2);
//	  GPIO_WriteBit(CS_PORT,CS_PIN,!State);
//	  GPIO_WriteBit(XDCS_PORT,XDCS_PIN,!State);
//	  GPIO_WriteBit(CS_PORT,CS_PIN,!State);
//	  while(1){};
//
//	  GUI_Init();
//	  GUI_CURSOR_Show();
//	  WM_MOTION_Enable(1);
//	  play_video("avifiles/birds_small2.avi");


//f=f_open(&fsrc,"0:jas.rgb",FA_READ|FA_OPEN_EXISTING);
//
//LCD_area(0,0,239,319);
//
//for(int i=0;i<450;i++)
//{
//	   f=f_read(&fsrc,aucLine,512,&s2);
////		   DMA_Config(512,aucLine);
////		   while(!(DMA2->LISR & DMA_LISR_TCIF0)){};
////		   DMA2->LISR=0x00000000;
//	   SRAM_WriteBuffer(aucLine,i*512+0x00500000,512);
//}
//for(int i=0;i<450;i++)
//{
//	   SRAM_ReadBuffer(aucLine,i*512+0x00500000,512);
//	   DMA_Config(512,aucLine);
//	   while(!(DMA2->LISR & DMA_LISR_TCIF0)){};
//	   DMA2->LISR=0x00000000;
//}
