/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_fsmc.h"
#include "ili9320.h"
#include "global_inc.h"
#include "FONT8x8.h"
#include <stm32f4xx.h>
#include <system_stm32f4xx.h>
#include <core_cm4.h>

#include <stm32f4xx_fsmc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

#define LED_ON GPIOD->BSRRL|=GPIO_BSRR_BS_6
#define LED_OFF GPIOD->BSRRH|=GPIO_BSRR_BS_6

#define Set_Rst GPIOA->BSRRL|=GPIO_BSRR_BS_2
#define Clr_Rst GPIOA->BSRRH|=GPIO_BSRR_BS_2


#define LCD_REG      (*((volatile unsigned short *) 0x60000000))
#define LCD_RAM      (*((volatile unsigned short *) 0x60020000))



void LCD_WriteIndex(uint16_t index)
{
        LCD_REG = index;
}

void LCD_WriteData(uint16_t data)
{
        LCD_RAM = data;
}

uint16_t LCD_ReadData(void)
{
  return LCD_RAM;
}

void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{
        /* Write 16-bit Index, then Write Reg */
        LCD_WriteIndex(LCD_Reg);
        /* Write 16-bit Reg */
        LCD_WriteData(LCD_RegValue);
}

uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
        /* Write 16-bit Index (then Read Reg) */
        LCD_WriteIndex(LCD_Reg);

        /* Read 16-bit Reg */
        return LCD_ReadData();
}

static void ili9320_Configuration(void)
{

		ili9320_FSMCConfig();
        /*LCD RESET PIN init*/
        Set_Rst;
        Delay(100);
        Clr_Rst;
        Delay(100);
        Set_Rst;
        Delay(100);
        LCD_WriteReg(0x0000,0x0001); //uruchomienie oscylatora wywietlacza
        Delay(100);
}

void LCD_SetCursor(u8 Xpos, u16 Ypos)
{
  LCD_WriteReg(R32, Xpos);
  LCD_WriteReg(R33, Ypos);
}

void ili9320_Clear(u16 Color)
{
	u32 index=0;
	LCD_SetCursor(0,0);
	LCD_WriteIndex(R34);
	for(index=0;index<76800;index++)
	{
		LCD_RAM=Color;
	}
}

void ili9320_Pixel(int x, int y, int color)
{
	  LCD_SetCursor(x,y);
	  LCD_WriteIndex(R34);
	  LCD_WriteData(color);
}

void ili9320_init(void)
{


	ili9320_Configuration();

        int DeviceCode = 0x0001;

        DeviceCode = LCD_ReadReg(0x0000);               /* Read LCD ID  */

        if( DeviceCode == 0x9320)
        {

          LCD_WriteReg(0x00,0x0000);
                LCD_WriteReg(0x01,0x0100);      /* Driver Output Contral */
                LCD_WriteReg(0x02,0x0700);      /* LCD Driver Waveform Contral */
                LCD_WriteReg(0x03,0x1030);      /* Entry Mode Set */
//                LCD_WriteReg(0x03,0x10c0);      /* Entry Mode Set */

                LCD_WriteReg(0x04,0x0000);      /* Scalling Contral */
          LCD_WriteReg(0x08,0x0202);    /* Display Contral */
                LCD_WriteReg(0x09,0x0000);      /* Display Contral 3.(0x0000) */
                LCD_WriteReg(0x0a,0x0000);      /* Frame Cycle Contal.(0x0000) */
          LCD_WriteReg(0x0c,(1<<0));    /* Extern Display Interface Contral */
                LCD_WriteReg(0x0d,0x0000);      /* Frame Maker Position */
                LCD_WriteReg(0x0f,0x0000);      /* Extern Display Interface Contral 2. */

          Delay(100);  /* delay 100 ms */
                LCD_WriteReg(0x07,0x0101);      /* Display Contral */
          Delay(100);  /* delay 100 ms */

                LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4)); /* Power Control 1.(0x16b0)     */
                LCD_WriteReg(0x11,0x0007);                                                              /* Power Control 2 */
                LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));                                /* Power Control 3.(0x0138)     */
                LCD_WriteReg(0x13,0x0b00);                                                              /* Power Control 4 */
                LCD_WriteReg(0x29,0x0000);                                                              /* Power Control 7 */

                LCD_WriteReg(0x2b,(1<<14)|(1<<4));

//                LCD_WriteReg(0x20,0);
//                LCD_WriteReg(0x21,319);
                LCD_WriteReg(0x50,0);       /* Set X Start */
                LCD_WriteReg(0x51,239);     /* Set X End */
                LCD_WriteReg(0x52,0);       /* Set Y Start */
                LCD_WriteReg(0x53,319);     /* Set Y End */

                LCD_WriteReg(0x60,0x2700);      /* Driver Output Control */
                LCD_WriteReg(0x61,0x0001);      /* Driver Output Control */
                LCD_WriteReg(0x6a,0x0000);      /* Vertical Srcoll Control */

                LCD_WriteReg(0x80,0x0000);      /* Display Position? Partial Display 1 */
                LCD_WriteReg(0x81,0x0000);      /* RAM Address Start? Partial Display 1 */
                LCD_WriteReg(0x82,0x0000);      /* RAM Address End-Partial Display 1 */
                LCD_WriteReg(0x83,0x0000);      /* Displsy Position? Partial Display 2 */
                LCD_WriteReg(0x84,0x0000);      /* RAM Address Start? Partial Display 2 */
                LCD_WriteReg(0x85,0x0000);      /* RAM Address End? Partial Display 2 */

          LCD_WriteReg(0x90,(0<<7)|(16<<0));    /* Frame Cycle Contral.(0x0013) */
                LCD_WriteReg(0x92,0x0000);      /* Panel Interface Contral 2.(0x0000) */
                LCD_WriteReg(0x93,0x0001);      /* Panel Interface Contral 3. */
          LCD_WriteReg(0x95,0x0110);    /* Frame Cycle Contral.(0x0110) */
                LCD_WriteReg(0x97,(0<<8));
                LCD_WriteReg(0x98,0x0000);      /* Frame Cycle Contral */

          LCD_WriteReg(0x07,0x0173);

        }

    Delay(50);   /* delay 50 ms */

    ili9320_Clear(0xcccc);
}
void ili9320_CtrlLinesConfig(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;

        /* Enable GPIOs clock */
//        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB ,ENABLE );

        /* Enable FSMC clock */
//        RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

        /*-- GPIOs Configuration ------------------------------------------------------*/
        /*
        +-------------------+--------------------+------------------+------------------+
        +                       SRAM pins assignment                                   +
        +-------------------+--------------------+------------------+------------------+
        | PD0  <-> FSMC_D2              | PE7  <-> FSMC_D4   |
        | PD1  <-> FSMC_D3              | PE8  <-> FSMC_D5   |
        | PD4  <-> FSMC_NOE(RD) | PE9  <-> FSMC_D6   |
        | PD5  <-> FSMC_NWE(WR) | PE10 <-> FSMC_D7   |
        | PD7  <-> FSMC_NE1(CS) | PE11 <-> FSMC_D8   |
        | PD8  <-> FSMC_D13             | PE12 <-> FSMC_D9   |
        | PD9  <-> FSMC_D14             | PE13 <-> FSMC_D10  |
        | PD10 <-> FSMC_D15             | PE14 <-> FSMC_D11  |
        | PD11 <-> FSMC_A16(RS) | PE15 <-> FSMC_D12  |
        | PD14 <-> FSMC_D0              |                                                                |
        | PD15 <-> FSMC_D1                      |                                                                |
        PB7  <-> RSET
        +-------------------+------------------------+
        */
        /* GPIOD configuration */
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 |
                                    GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

        GPIO_Init(GPIOD, &GPIO_InitStructure);


        /* GPIOE configuration */
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
        GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

        GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7 | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 |
                                     GPIO_Pin_11| GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

        GPIO_Init(GPIOE, &GPIO_InitStructure);

        /* Pin PB7 - sygnal RSET  */
        GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
}
 void ili9320_FSMCConfig(void)
{
        FSMC_Bank1->BTCR[0] =  FSMC_BCR2_MBKEN | FSMC_BCR2_MWID_0 | FSMC_BCR1_WREN   ;
        FSMC_Bank1->BTCR[1] =  0x1404;
}

 void ili9320_box_mid_fill(unsigned int x, unsigned int y, unsigned int lx, unsigned int ly, unsigned int color)
 {
	 ili9320_area(x-(lx/2),y-(ly/2),x+(lx/2),y+(ly/2));

 		int i=0;
 		for(i=0; i<(lx+1)*(ly+1); i++)
 		{
 			LCD_RAM=color;
 		}

 }

 void ili9320_char_lc(char c, int col, int line, int size, int fColor, int bColor, int resize)
 {
 	 int i,j;
 	 int x=0,y=0;

 	 	if(resize==1)
 		{
 			x=col*8;
 			y=line*16;
 		}
 		else if(resize==2 || resize==3)
 		{
 			x=col*16;
 			y=line*32;
 		}
 		else if(resize>=4)
 		{
 			x=col*32;
 			y=line*64;
 		}

 	 unsigned int nCols;
 	 unsigned int nRows;
 	 unsigned int nBytes;
 	 unsigned char PixelRow;
 	 unsigned char Mask;
 	 unsigned int Word0;
 	 unsigned int Word1;
 	 unsigned char *pFont;

 	 unsigned char *pChar;
	 unsigned char *FontTable[] =
	 {
		(unsigned char *)FONT6x8,
//		(unsigned char *)Times19x23,
		(unsigned char *)FONT6x8,
		(unsigned char *)FONT8x16
	 };
	pFont = (unsigned char *)FontTable[size];
 	nCols = *pFont;
 	nRows = *(pFont + 1);
 	nBytes = *(pFont + 2);
 	pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;

 	if(resize!=1)
 	{
 		resize/=2;
 		resize*=2;
 	}

 	for (i = resize*nRows ; i >=resize ; i-=resize)
 	{

 	// copy pixel row from font table and then decrement row
 	 PixelRow = *pChar--;

 	 Mask = 0x80;

 	    for (j = 0; j < resize*nCols+1; j +=resize)
 	    {
 	        if ((PixelRow & Mask) == 0)Word0 = bColor;
 	        else Word0 = fColor;

 	        Mask = Mask >> 1;

 	        if(resize!=1)
 	        {
 	        	ili9320_box_mid_fill(x+j,y+i,resize+1,resize+1,Word0);
 	        }
 	        else ili9320_box_mid_fill(x+j,y+i,resize,resize,Word0);
 	    }
 	}

 }


 void ili9320_char(char c, int x, int y, int size, int fColor, int bColor, int resize)
 {

 	 int i,j;
 	 unsigned int nCols;
 	 unsigned int nRows;
 	 unsigned int nBytes;
 	 unsigned char PixelRow;
 	 unsigned char Mask;
 	 unsigned int Word0;
 	 unsigned int Word1;
 	 unsigned char *pFont;

 	 unsigned char *pChar;
	 unsigned char *FontTable[] =
	 {
		(unsigned char *)FONT6x8,
//		(unsigned char *)Times19x23,
		(unsigned char *)FONT6x8,
		(unsigned char *)FONT8x16
	 };
	pFont = (unsigned char *)FontTable[size];
 	nCols = *pFont;
 	nRows = *(pFont + 1);
 	nBytes = *(pFont + 2);
 	pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;

 	if(resize!=1)
 	{
 		resize/=2;
 		resize*=2;
 	}

 	for (i = resize*nRows ; i >=resize ; i-=resize)
 	{

 	// copy pixel row from font table and then decrement row
 	 PixelRow = *pChar--;

 	 Mask = 0x80;

 	    for (j = 0; j < resize*nCols+1; j +=resize)
 	    {
 	        if ((PixelRow & Mask) == 0)Word0 = bColor;
 	        else Word0 = fColor;

 	        Mask = Mask >> 1;

 	        if(resize!=1)
 	        {
 	        	ili9320_box_mid_fill(x+j,y+i,resize+1,resize+1,Word0);
// 	        	ili9320_Pixel(x+j,y+i,Word0);
 	        }
 	        else ili9320_box_mid_fill(x+j,y+i,resize,resize,Word0);
 	    }
 	}

 }

 void ili9320_String( char *pString, int x, int  y, int Size, int fColor, int bColor, int resize)
 {
 	while (*pString != 0x00)
 	{
 		ili9320_char(*pString++, x, y,Size, fColor, bColor, resize);

 		switch(resize)
 		{
 			case 1:
 			{
 				x=x+8;
 				if(x>=316)
 				{
 					y=y-16;
 					x=3;
 				}
 				break;
 			}
 			case 2:
 			case 3:
 			{
 				x=x+16;
 				if(x>=240)
 				{
 					y=y+34;
 					x=5;
 				}
 				break;
 			}
 			case 4:
 			{
 				x=x+32;
 				if(x>=240)
 				{
 					y=y+64;
 					x=8;
 				}
 				break;
 			}
 		}
 	}
 }

 void ili9320_String_lc( char *pString, int col, int  line, int fColor, int bColor, int resize)
 {
 	int y=0,x=0;

 	if(resize==1)x=col*8;
 	else if(resize==2 || resize==3)x=col*16;
 	else if(resize>4)x=col*32;

 	while (*pString != 0x00)
 	{
 		switch(resize)
 		{
 			case 1:
 			{
 				x+=8;
 				y=line*16;
 				if(x>=316)
 				{
 					line++;
 					x=3;
 				}
 				break;
 			}
 			case 2:
 			case 3:
 			{
 				x+=16;
 				y=line*34;
 				if(x>=320)
 				{
 					line++;
 					x=5;
 				}
 				break;
 			}
 			case 4:
 			{
 				x+=32;
 				y=line*64;
 				if(x>=320)
 				{
 					line++;
 					x=8;
 				}
 				break;
 			}
 		}

 		ili9320_char(*pString++, x, y,2, fColor, bColor, resize);
 	}
 }


void ili9320_area(int  x0, int  y0,int  x1, int  y1)
{
	LCD_SetCursor(x0,y0);
    LCD_WriteReg(0x0050,x0);       /* Set X Start */
    LCD_WriteReg(0x0051,x1);     /* Set X End */
    LCD_WriteReg(0x0052,y0);       /* Set Y Start */
    LCD_WriteReg(0x0053,y1);     /* Set Y End */
    LCD_WriteIndex(R34);
}



