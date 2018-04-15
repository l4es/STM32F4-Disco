// I2Cdev library collection - AB0805 I2C device class
// Based on Abracon AB08X5 Real-Time Clock Family datasheet, 2015
// 03/07/2015 by Curran Sinha (curransinha@gmail.com)
// Based heavily off of Jeff Rowberg's DS1307 RTC Library
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
// I2C Device Library hosted at http://www.i2cdevlib.com
//
// Changelog:
//     2015-03-09 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "AB0805.h"
#include "inv_mpu.h"
#include <stm32f4xx.h>
#include <system_stm32f4xx.h>
#include <core_cm4.h>
#include "itoa.h"
#include <misc.h>
#include <stm32f4xx_fsmc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_i2c.h>
/** Default constructor, uses default I2C address.
 * @see AB0805_DEFAULT_ADDRESS
 */


uint8_t dec2bcd(uint8_t dec)
{
	return ((dec / 10)<<4) | (dec % 10);
}

uint8_t bcd2dec(uint8_t bcd)
{
    return ((((bcd) >> 4) & 0x0F) * 10) + ((bcd) & 0x0F);
}


void AB0805_WriteBits(u8 slave_addr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data)
{
//	slave_addr=0xD2;
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t tmp;
    AB0805_BufferRead( slave_addr, &tmp, regAddr, 1);
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    tmp &= ~(mask); // zero all important bits in existing byte
    tmp |= data; // combine data with existing byte
    AB0805_writeByte(slave_addr,  &tmp, regAddr);
}

/** write a single bit in an 8-bit device register.
 * @param slaveAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 */
void AB0805_WriteBit( uint8_t slave_addr, uint8_t regAddr, uint8_t bitNum, uint8_t data)
{
    uint8_t tmp;
    AB0805_BufferRead(slave_addr, &tmp, regAddr, 1);
    tmp = (data != 0) ? (tmp | (1 << bitNum)) : (tmp & ~(1 << bitNum));
    AB0805_writeByte(slave_addr,regAddr,&tmp);

}

/** Read multiple bits from an 8-bit device register.
 * @param slaveAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in readTimeout)
 */
void AB0805_readBits(u8 slave_addr,  uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data)
{
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
//	slave_addr=0xD2;
    uint8_t tmp;
    AB0805_BufferRead(slave_addr, &tmp, regAddr, 1);
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    tmp &= mask;
    tmp >>= (bitStart - length + 1);
    *data = tmp;
}

/** Read a single bit from an 8-bit device register.
 * @param slaveAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in readTimeout)
 */
void AB0805_ReadByte(u8 slave_addr, uint8_t regAddr, uint8_t *data)
{
//	slave_addr=0xD2;
//    uint8_t tmp;
    AB0805_BufferRead(slave_addr ,data, regAddr, 1);
//    *data = tmp & (1 << bitNum);
}

void AB0805_writeByte(u8 slave_addr, u8 writeAddr,u8 pBuffer )
{
//	slave_addr=0xD2;
    I2C_GenerateSTART(AB0805_I2C, ENABLE);
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(AB0805_I2C, slave_addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(AB0805_I2C, writeAddr);
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(AB0805_I2C, pBuffer);
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(AB0805_I2C, ENABLE);
}

/**
 * @brief  Reads a block of data from the MPU6050.
 * @param  slaveAddr  : slave address MPU6050_DEFAULT_ADDRESS
 * @param  pBuffer : pointer to the buffer that receives the data read from the MPU6050.
 * @param  readAddr : MPU6050's internal address to read from.
 * @param  NumByteToRead : number of bytes to read from the MPU6050 ( NumByteToRead >1  only for the Mgnetometer readinf).
 * @return None
 */
void AB0805_BufferRead(u8 slave_addr, u8* pBuffer, u8 readAddr, u16 NumByteToRead)
{
    // ENTR_CRT_SECTION();

//	slave_addr=0xD2;
    /* While the bus is busy */
    while (I2C_GetFlagStatus(AB0805_I2C, I2C_FLAG_BUSY));

    /* Send START condition */
    I2C_GenerateSTART(AB0805_I2C, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send MPU6050 address for write */
    I2C_Send7bitAddress(AB0805_I2C, slave_addr, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(AB0805_I2C, ENABLE);

    /* Send the MPU6050's internal address to write to */
    I2C_SendData(AB0805_I2C, readAddr);

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Send STRAT condition a second time */
    I2C_GenerateSTART(AB0805_I2C, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send MPU6050 address for read */
    I2C_Send7bitAddress(AB0805_I2C, slave_addr, I2C_Direction_Receiver);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /* While there is data to be read */
    while (NumByteToRead)
    {
        if (NumByteToRead == 1)
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(AB0805_I2C, DISABLE);

            /* Send STOP Condition */
            I2C_GenerateSTOP(AB0805_I2C, ENABLE);
        }

        /* Test on EV7 and clear it */
        if (I2C_CheckEvent(AB0805_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            /* Read a byte from the MPU6050 */
            *pBuffer = I2C_ReceiveData(AB0805_I2C);

            /* Point to the next location where the byte read will be saved */
            pBuffer++;

            /* Decrement the read bytes counter */
            NumByteToRead--;
        }
    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(AB0805_I2C, ENABLE);
    // EXT_CRT_SECTION();
}

void AB0805_initialize()
{
	devAddr = (0xD2);
    uint8_t stopClk = 0x91;
    uint8_t allowOscEdit = 0xA1;
    uint8_t xtOscSel = 0x08;//0x08
//    AB0805_WriteBit(0xD2, AB0805_RA_CONTROL1, AB0805_CONTROL_WRTC, 1);
//    AB0805_writeByte(devAddr, AB0805_RA_CONTROL1, stopClk);  //stops clock
    AB0805_writeByte(devAddr, AB0805_RA_CONTROL2, 0x03); //interrupt////////////
//    AB0805_writeByte(devAddr, AB0805_RA_CONFIG_KEY, allowOscEdit);  //Allows edits to osc. cntrl register (0x1C)
//    AB0805_writeByte(devAddr, AB0805_RA_OSC_CONTROL, xtOscSel);   //Crystal used (switch to RC if XT osc failure)
    AB0805_writeByte(devAddr, 0x12, 0x84); //// interrupt alarm enble /// level///////////////
    AB0805_WriteBit(devAddr, 0x0f,2,0);
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return 1 if connection is valid, 0 otherwise
 */
u8 AB0805_testConnection()
{
//    if (AB0805_ReadByte(devAddr, AB0805_RA_ID0 , buffer_AB) == 0x08) {
        return 1;
//    }
//    return 0;
}

void AB0805_startClock()
{
	AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL1_STOP_BIT, 0);
 }

void AB0805_stopClock()
{
	AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL1_STOP_BIT, 1);
}
void AB0805_useRcOsc()
{
    uint8_t allowOscEdit = 0xA1;
    AB0805_writeByte(devAddr, AB0805_RA_CONFIG_KEY, allowOscEdit);  //Allows edits to osc. cntrl register (0x1C)
    AB0805_WriteBit(devAddr, AB0805_RA_OSC_CONTROL, AB0805_OSC_CONTROL_OSC_SEL, 1);
}

// HUNDREDTHS register -- only valid with XT oscillator
uint16_t AB0805_getHundredths() {
    AB0805_ReadByte(devAddr, AB0805_RA_HUNDREDTHS, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0xF0) >> 4) * 10;
}
void AB0805_setHundredths(uint16_t hundredths) {
    if (hundredths > 99) return;
    uint8_t value = ((hundredths/ 10) << 4) + (hundredths % 10);
    AB0805_writeByte(devAddr, AB0805_RA_HUNDREDTHS, value);
}

// SECONDS register
uint8_t AB0805_getSeconds() {
    // Byte: [7 = CH] [6:4 = 10SEC] [3:0 = 1SEC]
    AB0805_ReadByte(devAddr, AB0805_RA_SECONDS, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x70) >> 4) * 10;
}
uint8_t AB0805_getSeconds_Alarm()
{
    // Byte: [7 = CH] [6:4 = 10SEC] [3:0 = 1SEC]
    AB0805_ReadByte(devAddr, 0x09, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x70) >> 4) * 10;
}
void AB0805_setSeconds(uint8_t seconds) {
    if (seconds > 59) return;
    uint8_t value = 0x00 + ((seconds / 10) << 4) + (seconds % 10);
    AB0805_writeByte(devAddr, AB0805_RA_SECONDS, value);
}
void AB0805_setSeconds_Alarm(uint8_t seconds) {
    if (seconds > 59) return;
    uint8_t value = 0x00 + ((seconds / 10) << 4) + (seconds % 10);
    AB0805_writeByte(devAddr, 0x09, value);
}
// MINUTES register
uint8_t AB0805_getMinutes() {
    // Byte: [7 = 0] [6:4 = 10MIN] [3:0 = 1MIN]
    AB0805_ReadByte(devAddr, AB0805_RA_MINUTES, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x70) >> 4) * 10;
}
uint8_t AB0805_getMinutes_Alarm() {
    // Byte: [7 = 0] [6:4 = 10MIN] [3:0 = 1MIN]
    AB0805_ReadByte(devAddr, 0x0A, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x70) >> 4) * 10;
}
void AB0805_setMinutes(uint8_t minutes) {
    if (minutes > 59) return;
    uint8_t value = ((minutes / 10) << 4) + (minutes % 10);
    AB0805_writeByte(devAddr, AB0805_RA_MINUTES, value);
}
void AB0805_setMinutes_Alarm(uint8_t minutes) {
    if (minutes > 59) return;
    uint8_t value = ((minutes / 10) << 4) + (minutes % 10);
    AB0805_writeByte(devAddr, 0x0A, value);
}

// HOURS register
uint8_t AB0805_getMode() {
    AB0805_readBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL1_12OR24_BIT, buffer_AB);
    mode12 = buffer_AB[0];
    return buffer_AB[0];
}
void AB0805_setMode(uint8_t mode) {
    AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL1_12OR24_BIT, mode);
}
uint8_t AB0805_getAMPM() {
    AB0805_readBit(devAddr, AB0805_RA_HOURS, AB0805_HOURS_AMPM_BIT, buffer_AB);
    return buffer_AB[0];
}
//0 = AM Hours, 1 = PM Hours
void AB0805_setAMPM(uint8_t ampm) {
    AB0805_WriteBit(devAddr, AB0805_RA_HOURS, AB0805_HOURS_AMPM_BIT, ampm);
}
uint8_t AB0805_getHours12() {
    AB0805_ReadByte(devAddr, AB0805_RA_HOURS, buffer_AB);
    mode12 = getMode();
    if (mode12) {
        // bit 6 is high, 12-hour mode
        // Byte: [5 = AM/PM] [4 = 10HR] [3:0 = 1HR]
        return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x10) >> 4) * 10;
    } else {
        // bit 6 is low, 24-hour mode (default)
        // Byte: [5:4 = 10HR] [3:0 = 1HR]
        uint8_t hours = (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x30) >> 4) * 10;

        // convert 24-hour to 12-hour format, since that's what's requested
        if (hours > 12) hours -= 12;
        else if (hours == 0) hours = 12;
        return hours;
    }
}
void AB0805_setHours12(uint8_t hours, uint8_t ampm) {
    if (hours > 12 || hours < 1) return;
    if (mode12) {
        // bit 6 is high, 12-hour mode
        // Byte: [5 = AM/PM] [4 = 10HR] [3:0 = 1HR]
        if (ampm > 0) ampm = 0x20;
        uint8_t value = ampm + ((hours / 10) << 4) + (hours % 10);
        AB0805_writeByte(devAddr, AB0805_RA_HOURS, value);
    } else {
        // bit 6 is low, 24-hour mode (default)
        // Byte: [5:4 = 10HR] [3:0 = 1HR]
        if (ampm > 0) hours += 12;
        if (hours == 0) hours = 12; // 12 AM
        else if (hours == 24) hours = 12; // 12 PM, after +12 adjustment
        uint8_t value = ((hours / 10) << 4) + (hours % 10);
        AB0805_writeByte(devAddr, AB0805_RA_HOURS, value);
    }
}
uint8_t AB0805_getHours24()
{
    AB0805_ReadByte(devAddr, AB0805_RA_HOURS, buffer_AB);
//    mode12 = getMode();
    mode12 = 1;
    if (mode12) {
        // bit 6 is high, 12-hour mode
        // Byte: [5 = AM/PM] [4 = 10HR] [3:0 = 1HR]
        uint8_t hours = (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x10) >> 4) * 10;

        // convert 12-hour to 24-hour format, since that's what's requested
        if (buffer_AB[0] & 0x20) {
            // currently PM
            if (hours < 12) hours += 12;
        } else {
            // currently AM
            if (hours == 12) hours = 0;
        }
        return hours;
    } else {
        // bit 6 is low, 24-hour mode (default)
        // Byte: [5:4 = 10HR] [3:0 = 1HR]
        return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x30) >> 4) * 10;
    }
}
uint8_t AB0805_getHours24_Alarm()
{
    AB0805_ReadByte(devAddr, 0x0B, buffer_AB);
//    mode12 = getMode();
    mode12 = 1;
    if (mode12) {
        // bit 6 is high, 12-hour mode
        // Byte: [5 = AM/PM] [4 = 10HR] [3:0 = 1HR]
        uint8_t hours = (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x10) >> 4) * 10;

        // convert 12-hour to 24-hour format, since that's what's requested
        if (buffer_AB[0] & 0x20) {
            // currently PM
            if (hours < 12) hours += 12;
        } else {
            // currently AM
            if (hours == 12) hours = 0;
        }
        return hours;
    } else {
        // bit 6 is low, 24-hour mode (default)
        // Byte: [5:4 = 10HR] [3:0 = 1HR]
        return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x30) >> 4) * 10;
    }
}
void AB0805_setHours24(uint8_t hours) {
    if (hours > 23) return;
//    if (mode12)
//    {
        // bit 6 is high, 12-hour mode
        // Byte: [5 = AM/PM] [4 = 10HR] [3:0 = 1HR]
        uint8_t ampm = 0;
        if (hours > 11) ampm = 0x20;
        if (hours > 12) hours -= 12;
        else if (hours == 0) hours = 12;
        uint8_t value = ampm + ((hours / 10) << 4) + (hours % 10);
        AB0805_writeByte(devAddr, AB0805_RA_HOURS, value);
//    } else {
        // bit 6 is low, 24-hour mode (default)
        // Byte: [5:4 = 10HR] [3:0 = 1HR]
//        uint8_t value = ((hours / 10) << 4) + (hours % 10);
//        AB0805_writeByte(devAddr, AB0805_RA_HOURS, value);
//    }
}
void AB0805_setHours24_Alarm(uint8_t hours) {
    if (hours > 23) return;
//    if (mode12) {
        // bit 6 is high, 12-hour mode
        // Byte: [5 = AM/PM] [4 = 10HR] [3:0 = 1HR]
        uint8_t ampm = 0;
        if (hours > 11) ampm = 0x20;
        if (hours > 12) hours -= 12;
        else if (hours == 0) hours = 12;
        uint8_t value = ampm + ((hours / 10) << 4) + (hours % 10);
        AB0805_writeByte(devAddr, 0x0B, value);
//    } else {
        // bit 6 is low, 24-hour mode (default)
        // Byte: [5:4 = 10HR] [3:0 = 1HR]
//        uint8_t value = ((hours / 10) << 4) + (hours % 10);
//        AB0805_writeByte(devAddr, 0x0B, value);
//    }
}

// DAY register
uint8_t AB0805_getDayOfWeek()
{
//    AB0805_readBits(devAddr, AB0805_RA_DAY, AB0805_DAY_BIT, AB0805_DAY_LENGTH, buffer_AB);
	AB0805_ReadByte(devAddr, AB0805_RA_DAY, buffer_AB);
	return buffer_AB[0]+1;
}
void AB0805_setDayOfWeek(uint8_t dow)
{
    if (dow==0)dow=7;
//    AB0805_WriteBits(devAddr, AB0805_RA_DAY, AB0805_DAY_BIT, AB0805_DAY_LENGTH, dow);
	AB0805_writeByte(devAddr, AB0805_RA_DAY, dow-1);
}

// DATE register
uint8_t AB0805_getDay() {
    // Byte: [7:6 = 0] [5:4 = 10DAY] [3:0 = 1DAY]
    AB0805_ReadByte(devAddr, AB0805_RA_DATE, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x30) >> 4) * 10;
}
void AB0805_setDay(uint8_t day) {
    uint8_t value = ((day / 10) << 4) + (day % 10);
    AB0805_writeByte(devAddr, AB0805_RA_DATE, value);
}

// MONTH register
uint8_t AB0805_getMonth() {
    // Byte: [7:5 = 0] [4 = 10MONTH] [3:0 = 1MONTH]
    AB0805_ReadByte(devAddr, AB0805_RA_MONTH, buffer_AB);
    return (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0x10) >> 4) * 10;
}
void AB0805_setMonth(uint8_t month) {
    if (month < 1 || month > 12) return;
    uint8_t value = ((month / 10) << 4) + (month % 10);
    AB0805_writeByte(devAddr, AB0805_RA_MONTH, value);
}

// YEAR register
uint16_t AB0805_getYear() {
    AB0805_ReadByte(devAddr, AB0805_RA_YEAR, buffer_AB);
    return 2000 + (buffer_AB[0] & 0x0F) + ((buffer_AB[0] & 0xF0) >> 4) * 10;
}
void AB0805_setYear(uint16_t year) {
    if (year < 2000) return;
    year -= 2000;
    uint8_t value = ((year / 10) << 4) + (year % 10);
    AB0805_writeByte(devAddr, AB0805_RA_YEAR, value);
}


// convenience methods

void AB0805_getDate(uint16_t *year, uint8_t *month, uint8_t *day) {
    *year = AB0805_getYear();
    *month = AB0805_getMonth();
    *day = AB0805_getDay();
}
void AB0805_setDate(uint16_t year, uint8_t month, uint8_t day) {
	AB0805_setYear(year);
    AB0805_setMonth(month);
    AB0805_setDay(day);
}

void AB0805_getTime12(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *ampm) {
    *hours = AB0805_getHours12();
    *minutes = AB0805_getMinutes();
    *seconds = AB0805_getSeconds();
    *ampm = AB0805_getAMPM();
}
void AB0805_setTime12(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t ampm) {
	AB0805_setSeconds(seconds);
	AB0805_setMinutes(minutes);
	AB0805_setHours12(hours, ampm);
}

void AB0805_getTime24(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    *hours = AB0805_getHours24();
    *minutes = AB0805_getMinutes();
    *seconds = AB0805_getSeconds();
}
void AB0805_setTime24(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	AB0805_setSeconds(seconds);
	AB0805_setMinutes(minutes);
	AB0805_setHours24(hours);
}

void AB0805_getDateTime12(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *ampm) {
	AB0805_getTime12(hours, minutes, seconds, ampm);
	AB0805_getDate(year, month, day);
}
void AB0805_setDateTime12(uint16_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t ampm) {
	AB0805_setTime12(hours, minutes, seconds, ampm);
	AB0805_setDate(year, month, day);
}

void AB0805_getDateTime24(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
	AB0805_getTime24(hours, minutes, seconds);
	AB0805_getDate(year, month, day);
}
void AB0805_setDateTime24(uint16_t year, uint8_t  month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL_WRTC, 1);
	AB0805_setHours24(hours);
	AB0805_setTime24(hours, minutes, seconds);
	AB0805_setDate(year, month, day);
	AB0805_WriteBit(devAddr, AB0805_RA_CONTROL1, AB0805_CONTROL_WRTC, 0);
}
