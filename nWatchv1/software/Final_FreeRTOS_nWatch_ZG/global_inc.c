#include <global_inc.h>

inline void cpu_off(void)
{
//	USART_puts("TTM:RST-SYSTEMRESET");

	GPIOG->BSRRL|=GPIO_BSRR_BS_6;
	GPIOG->BSRRL|=GPIO_BSRR_BS_8;

	GPIOC->MODER|=GPIO_MODER_MODER7_0;
	GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR7;
	GPIOC->ODR|=GPIO_ODR_ODR_7;

	GPIOC->MODER|=GPIO_MODER_MODER6_0;
	GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR6;
	GPIOC->ODR|=GPIO_ODR_ODR_6;

	GPIOC->BSRRH|=GPIO_BSRR_BS_6;
	GPIOC->BSRRH|=GPIO_BSRR_BS_7;

//	vTaskEndScheduler();
	taskENTER_CRITICAL();
//	motion_init();
//	LED_ON;
	for(int i=0;i<160;i++)
	{
		IWDG_ReloadCounter();
		LCD_line(i,0,i,240,BLACK,1);
		LCD_line(320-i,0,320-i,240,BLACK,1);
//		GUI_SetColor(GUI_WHITE);
//		GUI_DrawLine(i,0,i,320);
//		GUI_DrawLine(240-i,0,240-i,320);
		backlight((160-i)/3);
		delay(1);
	}
//	LED_OFF;
	GPIOD->BSRRH|=GPIO_BSRR_BS_3;

	while(1)
	{
//		GPIOD->BSRRH|=GPIO_BSRR_BS_3;
//		AB0805_WriteBit(0xD2, 0x0f,2,0);
	};
	taskEXIT_CRITICAL();
}
