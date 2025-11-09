/*
 * DS3231_Driver.h
 *
 *  Created on: Jun 23, 2025
 *      Author: nora
 */

#ifndef INC_DS3231_DRIVER_H_
#define INC_DS3231_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

#define DS3231_DELAY HAL_MAX_DELAY

#define DS3231_I2C_ADDRESS (0x68 << 1)


//Macro define
#define DS_READ_BIT(value, bit) (((value) >> (bit)) & 0x01)
#define DS_SET_BIT(value, bit) ((value) |= (1U << (bit)))
#define DS_CLEAR_BIT(value, bit) ((value) &= ~(1U << (bit)))


//Register defines
#define DS3231_SECONDS_REG 0x00
#define DS3231_MINUTES_REG 0x01
#define DS3231_HOURS_REG 0x02
#define DS3231_DAY_OF_WEEK_REG 0x03
#define DS3231_DATE_REG 0x04
#define DS3231_MONTH_REG 0x05
#define DS3231_YEAR_REG 0x06
#define DS3231_ALARM1_SECONDS_REG 0x07
#define DS3231_ALARM1_MINUTES_REG 0x08
#define DS3231_ALARM1_HOURS_REG 0x09
#define DS3231_ALARM1_DAY_DATE_REG 0x0A
#define DS3231_ALARM2_MINUTES_REG 0x0B
#define DS3231_ALARM2_HOURS_REG 0x0C
#define DS3231_ALARM2_DAY_DATE_REG 0x0D
#define DS3231_CONTROL_REG 0x0E
#define DS3231_CONTROL_STATUS_REG 0x0F
#define DS3231_AGING_OFFSET_REG 0x10
#define DS3231_TEMPERATURE_MSB_REG 0x11
#define DS3231_TEMPERATURE_LSB_REG 0x12

//Special Bit defines
#define DS3231_12_24 6
#define DS3231_AM_PM_20_HOUR 5
#define DS3231_CENTURY 7
#define DS3231_DY_DT 6

//Alarm Bit Defines
#define DS3231_A1M1 7
#define DS3231_A1M2 7
#define DS3231_A1M3 7
#define DS3231_A1M4 7
#define DS3231_A2M2 7
#define DS3231_A2M3 7
#define DS3231_A2M4 7

//Control Bit Defines
#define DS3231_EOSC 7
#define DS3231_BBSQW 6
#define DS3231_CONV 5
#define DS3231_RS_2 4
#define DS3231_RS_1 3
#define DS3231_INTCN 2
#define DS3231_A2IE 1
#define DS3231_A1IE 0

//Control Status Defines
#define DS3231_OSF 7
#define DS3231_EN32kHz 3
#define DS3231_BSY 2
#define DS3231_A2F 1
#define DS3231_A1F 0


//DS3231 Struct
typedef struct DS3231{
	//I2C-Handle
	I2C_HandleTypeDef *i2cHandle;
	//Time data
	uint8_t time[3];
	//date data
	uint8_t date[4];
	//signed temp data
	int16_t temp;
	//Error status
	HAL_StatusTypeDef status;
}DS3231;


//typedefs
typedef enum DS3231_Rate{
	Rate_1_HZ, Rate_1024_HZ, Rate_4096_HZ, Rate_8192_HZ
}DS3231_Rate;

typedef enum DS3231_States{
	Enabled, Disabled
}DS3231_States;

typedef enum DS3231_HourMode{
	Hour_24, Hour_12_AM_PM
}DS3231_HourMode;

typedef enum DS3231_Alarmmode1{
	ALARM_1_EVERY_S = 0x0F, ALARM_1_MATCH_S = 0x0E, ALARM_1_MATCH_S_M = 0x0C, ALARM_1_MATCH_S_M_H = 0x08, ALARM_1_MATCH_DATE_S_M_H= 0x00, ALARM_1_MATCH_DAY_S_M_H = 0x80
}DS3231_Alarmmode1;

typedef enum DS3231_Alarmmode2{
	ALARM_2_EVERY_M = 0x07, ALARM_2_MATCH_M = 0x06, ALARM_2_MATCH_M_H = 0x04, ALARM_2_MATCH_DATE_M_H= 0x00, ALARM_2_MATCH_DAY_M_H = 0x80
}DS3231_Alarmmode2;


//Init Function
uint8_t DS3231_Init(DS3231* dev, I2C_HandleTypeDef* i2cHandle);


//Number conversion functions
uint8_t DecToBCD(uint8_t dec);
uint8_t BCDToDec(uint8_t bin);


//Time Functions
void DS3231_GetSeconds(DS3231* dev);
void DS3231_GetMinutes(DS3231* dev);
void DS3231_GetHours(DS3231* dev);
void DS3231_GetTime(DS3231* dev);

void DS3231_SetSeconds(DS3231* dev, uint8_t sec);
void DS3231_SetMinutes(DS3231* dev, uint8_t min);
void DS3231_SetHours(DS3231* dev, uint8_t hour);
void DS3231_SetHourMode(DS3231* dev, DS3231_HourMode mode);
void DS3231_SetTime(DS3231* dev, uint8_t sec, uint8_t min,uint8_t hour);


//Date Functions
void DS3231_GetDayOfWeek(DS3231* dev);
void DS3231_GetDate(DS3231* dev);
void DS3231_GetMonth(DS3231* dev);
void DS3231_GetYear(DS3231* dev);
void DS3231_GetFullDate(DS3231* dev);

void DS3231_SetDayOfWeek(DS3231* dev, uint8_t dow);
void DS3231_SetDate(DS3231* dev, uint8_t date);
void DS3231_SetMonth(DS3231* dev, uint8_t month);
void DS3231_SetYear(DS3231* dev, uint8_t year);
void DS3231_SetFullDate(DS3231* dev, uint8_t day, uint8_t date, uint8_t month, uint8_t year);


//Alarm Functions
//Alarm 1 Functions
void DS3231_Alarm1Enable(DS3231* dev, DS3231_States state);
uint8_t DS3231_IsAlarm1FlagSet(DS3231* dev);
void DS3231_SetAlarm1Mode(DS3231* dev,DS3231_Alarmmode1 mode);
void DS3231_ClearAlarm1Flag(DS3231*dev);
void DS3231_SetAlarm1Seconds(DS3231* dev, uint8_t sec);
void DS3231_SetAlarm1Minutes(DS3231* dev, uint8_t min);
void DS3231_SetAlarm1Hours(DS3231* dev, uint8_t hour);
void DS3231_SetAlarm1HourMode(DS3231* dev, DS3231_HourMode mode);
void DS3231_SetAlarm1DayOfWeek(DS3231* dev, uint8_t dow);
void DS3231_SetAlarm1Date(DS3231* dev, uint8_t date);


//Alarm 2 Functions
void DS3231_Alarm2Enable(DS3231* dev, DS3231_States state);
uint8_t DS3231_IsAlarm2FlagSet(DS3231* dev);
void DS3231_SetAlarm2Mode(DS3231* dev,DS3231_Alarmmode2 mode);
void DS3231_ClearAlarm2Flag(DS3231*dev);
void DS3231_SetAlarm2Minutes(DS3231* dev, uint8_t min);
void DS3231_SetAlarm2Hours(DS3231* dev, uint8_t hour);
void DS3231_SetAlarm2HourMode(DS3231* dev, DS3231_HourMode mode);
void DS3231_SetAlarm2DayOfWeek(DS3231* dev, uint8_t dow);
void DS3231_SetAlarm2Date(DS3231* dev, uint8_t date);


//Configuration Functions
void DS3231_OscillatorEnable(DS3231* dev, DS3231_States state);
uint8_t DS3231_IsOscillatorSet(DS3231* dev);
void DS3231_BBSQWEnable(DS3231* dev, DS3231_States state);
uint8_t DS3231_IsBBSQWSet(DS3231* dev);
void DS3231_RateSelect(DS3231* dev, DS3231_Rate rate);
uint8_t DS3231_IsRateSelectSet(DS3231* dev);
void DS3231_InterruptEnable(DS3231* dev, DS3231_States state);
uint8_t DS3231_IsInterruptSet(DS3231* dev);
void DS3231_32kHzEnable(DS3231* dev, DS3231_States state);
uint8_t DS3231_Is32kHzSet(DS3231* dev);
void DS3231_ClearOscillatorStoppedFlag(DS3231* dev);
uint8_t DS3231_IsOscillatorStoppedSet(DS3231* dev);

//Temperature Functions
void DS3231_Get_Temp(DS3231* dev);
void DS3231_Force_TempConversion(DS3231 *dev);


//Low-Level Functions
HAL_StatusTypeDef DS3231_ReadRegister(DS3231* dev,uint8_t reg, uint8_t* data);
HAL_StatusTypeDef DS3231_ReadRegisters(DS3231* dev, uint8_t reg, uint8_t* data, uint8_t length);
HAL_StatusTypeDef DS3231_WriteRegister(DS3231* dev, uint8_t reg, uint8_t* data);
HAL_StatusTypeDef DS3231_WriteRegisters(DS3231* dev, uint8_t reg, uint8_t* data, uint8_t length);

#ifdef __cplusplus
}
#endif


#endif /* INC_DS3231_DRIVER_H_ */
