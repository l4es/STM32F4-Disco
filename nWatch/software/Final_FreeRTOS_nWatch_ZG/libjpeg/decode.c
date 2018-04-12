/**
  ******************************************************************************
  * @file    LibJPEG/LibJPEG_Decoding/Src/decode.c 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   This file contain the decompress method.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#include "decode.h"
#include "jpeglib.h"
#include "jmorecfg.h"
#include "ili9320.h"
/* Private typedef -----------------------------------------------------------*/
  /* This struct contains the JPEG decompression parameters */
int line_cnt=0;
//uint8_t _aucLine[4096];
//uint8_t pagebuff[23400];
//struct jpeg_decompress_struct cinfo;
//  /* This struct represents a JPEG error handler */
//struct jpeg_error_mgr jerr;
///* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Decode
  * @param  callback: line decoding callback
  * @param  file1:    pointer to the jpg file
  * @param  width:    image width
  * @param  buff:     pointer to the image line    
  * @retval None
  */
void jpeg_decode(FIL *file, uint32_t width, uint32_t * buff, uint8_t (*callback)(uint8_t*, uint32_t))
{ 
	JSAMPROW buffer[1]; /* Output row buffer */
	uint8_t buffek[720];
  uint32_t row_stride = 0; /* physical row width in image buffer */
  
  buffer[0] = buff;
	
  /* Step 1: allocate and initialize JPEG decompression object */
//  cinfo.err = jpeg_std_error(&jerr);/////////////////////////////////////////////////////

  /* Initialize the JPEG decompression object */  
//  jpeg_create_decompress(&cinfo);/////////////////////////////////////////////////////////
  
  jpeg_stdio_src(&cinfo, file);

  /* Step 3: read image parameters with jpeg_read_header() */
  jpeg_read_header(&cinfo, TRUE);

  /* Step 4: set parameters for decompression */
  cinfo.dct_method = JDCT_FLOAT;

  /* Step 5: start decompressor */
  jpeg_start_decompress(&cinfo);

  row_stride = width * 3;

//  LCD_area(0,0,320,240);///////////////////////////////////////////////////////////////////
  while (cinfo.output_scanline < cinfo.output_height)
  {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    
	  if(line_cnt==320)line_cnt=0;
	  int a=0;

	  LCD_area(0,line_cnt,cinfo.image_width,320);

//	  for(int i=0;i<cinfo.image_width*cinfo.num_components;)
//	  {
////		  LCD_RAM=rgb888Torgb565(buffer[0][i++], buffer[0][i++] ,buffer[0][i++]);
////		  buffek[a++]=rgb888Torgb565(buffer[0][i++], buffer[0][i++] ,buffer[0][i++]);
//		  buffek[a++]=buffer[0][i++];
//		  buffek[a++]=buffer[0][i++];
//		  buffek[a++]=buffer[0][i++];
//	  }

	   DMA_Config(720,buffer[0]);
	   while(!(DMA2->LISR & DMA_LISR_TCIF0)){};
	   DMA2->LISR=0x00000000;
	  line_cnt++;
  }

  /* Step 6: Finish decompression */
//  jpeg_finish_decompress(&cinfo);

  /* Step 7: Release JPEG decompression object */
//  jpeg_destroy_decompress(&cinfo);
    
}

static uint8_t Jpeg_CallbackFunction(uint8_t* Row, uint32_t DataLength)
{
//	  if(line_counter==240)line_counter=0;
//	  LCD_area(0,line_counter,320,line_counter);
//	  DMA_Config(DataLength,Row);
//	  line_counter++;
//	  return 0;
}
//unsigned short int rgb888Torgb565(unsigned char rgb888Pixel)
int rgb888Torgb565(uint8_t red,uint8_t green,uint8_t blue)
{
//    int red   = (rgb888Pixel >> 16) & 0xff;
//    int green = (rgb888Pixel >> 8 ) & 0xff;
//    int blue  =  rgb888Pixel        & 0xff;

    unsigned short  B =  (blue  >> 3)        & 0x001F;
    unsigned short  G = ((green >> 2) <<  5) & 0x07E0; // not <
    unsigned short  R = ((red   >> 3) << 11) & 0xF800; // not <

    return (int) (R | G | B);
}

unsigned short int rgb888Torgb565v(int rgb888Pixel)
{
    int red   = (rgb888Pixel >> 16) & 0xff;
    int green = (rgb888Pixel >> 8 ) & 0xff;
    int blue  =  rgb888Pixel        & 0xff;

    unsigned short  B =  (blue  >> 3)        & 0x001F;
    unsigned short  G = ((green >> 2) <<  5) & 0x07E0; // not <
    unsigned short  R = ((red   >> 3) << 11) & 0xF800; // not <

    return (unsigned short int) (R | G | B);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
