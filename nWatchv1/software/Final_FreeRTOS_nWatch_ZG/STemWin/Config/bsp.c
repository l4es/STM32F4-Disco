/**
  ******************************************************************************
  * @file    bsp.c
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    28-January-2014
  * @brief   This file provides targets hardware configuration 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "stmpe811.h"
#include "ff.h"
#include "portmacro.h"
#include "ili9320.h"
#include "global_inc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t BSP_TSC_Init(void);
/* Private functions ---------------------------------------------------------*/
u8 _acBuffer[8192];//__attribute((section(".ExRam")));;
FIL pFile;
int lastyyy=0;
int prev;
touch ts;
touch prev_state;
int z=0;

uint32_t BSP_Initt (void)
{
  return 0;
}

void LCD_BMP( char *sc)
{
	  if(f_open(&pFile, sc , FA_READ | FA_OPEN_EXISTING)==FR_OK)
	  {
		 GUI_BMP_DrawEx(APP_GetData, &pFile, 0, 0);
	  }
	  f_close(&pFile);

}


int APP_GetData(void * p, const U8 * * ppData, unsigned NumBytesReq, U32 Off)
{
	FRESULT fres=FR_OK;
	unsigned NumBytesRead=0;
	//You must create an pointer on structure
	FIL *phFile;
	//and then initialize the pointer value is passed to the function APP_GetData
	phFile=(FIL*) p;
	//And then use this pointer to your function
	fres=f_lseek(phFile,Off);
	fres=f_read(phFile,_acBuffer,NumBytesReq,&NumBytesRead);
	*ppData = _acBuffer;

	return NumBytesRead;
}

/**
  * @brief  Initializes the IO Expander registers.
  * @param  None
  * @retval - 0: if all initializations are OK.
*/
static uint32_t BSP_TSC_Init(void)
{
//  __disable_irq();
//  IOE_Config();
//  __enable_irq();
  return 0; /* Configuration is OK */
}

/**
  * @brief  Provide the GUI with current state of the touch screen
  * @param  None
  * @retval None
  */
u8 BSP_Pointer_Update(void)
{
  GUI_PID_STATE TS_State;
  uint16_t xDiff, yDiff;

  int act=touch_reg(&ts);

  xDiff = (prev_state.x > ts.x) ? (prev_state.x - ts.x) : (ts.x - prev_state.x);
  yDiff = (prev_state.y > ts.y) ? (prev_state.y - ts.y) : (ts.y - prev_state.y);

  if((xDiff > 2 ) || (yDiff > 2) || act!=prev)
  {

	  prev=act;
      TS_State.Layer = 0;
      TS_State.x = ts.x;
      TS_State.y = ts.y;
      TS_State.Pressed=act;
      GUI_TOUCH_StoreStateEx(&TS_State);
      prev_state.x=ts.x;
      prev_state.y=ts.y;
      prev=act;
//      LCD_circle(ts.x,ts.y,5,0xff00ff,1);
      return 1;
  }
  prev=act;
  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
