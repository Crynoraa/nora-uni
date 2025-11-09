/*
 * DS3231_Driver.c
 *
 *  Created on: Jun 23, 2025
 *      Author: nora
 */
#include "DS3231_Driver.h"

#ifdef __cplusplus
extern "C"{
#endif



//Init Function
uint8_t DS3231_Init(DS3231* dev, I2C_HandleTypeDef* i2cHandle)
{
	dev->i2cHandle = i2cHandle;

	dev->time[0]=0; //seconds
	dev->time[1]=0; //minutes
	dev->time[2]=0; //hours; always in 24h format

	dev->date[0]=0; //dayofweek
	dev->date[1]=0; //date
	dev->date[2]=0; //month
	dev->date[3]=0; //year

	dev->temp = 0; //temperature (signed)

	dev->status = 0; //status of last I2C-Transaction

	uint8_t errors = 0;



	//Check for Power on Reset values
	if(DS3231_IsOscillatorSet(dev))
	{
		if(dev->status != HAL_OK)
		{
			errors++;
		}
	}
	if(DS3231_IsBBSQWSet(dev))
	{
		if(dev->status != HAL_OK)
		{
			errors++;
		}
	}
	if(DS3231_IsRateSelectSet(dev))
	{
		if(dev->status != HAL_OK)
		{
			errors++;
		}
	}

	if(DS3231_IsInterruptSet(dev))
	{
		if(dev->status != HAL_OK)
		{
		errors++;
		}
	}

	if(DS3231_Is32kHzSet(dev))
	{
		if(dev->status != HAL_OK)
		{
			errors++;
		}
	}

	//Clear Flags
	DS3231_ClearAlarm1Flag(dev);
	if(dev->status != HAL_OK)
	{
		errors++;
	}

	DS3231_ClearAlarm2Flag(dev);
	if(dev->status != HAL_OK)
	{
		errors++;
	}

	DS3231_ClearOscillatorStoppedFlag(dev);
	if(dev->status != HAL_OK)
	{
		errors++;
	}

	//Disable Alarms & Set INTCN to Alarms
	DS3231_Alarm1Enable(dev,Disabled);
	if(dev->status != HAL_OK)
	{
		errors++;
	}

	DS3231_Alarm2Enable(dev,Disabled);
	if(dev->status != HAL_OK)
	{
		errors++;
	}

	DS3231_InterruptEnable(dev,Enabled);
	if(dev->status != HAL_OK)
	{
		errors++;
	}

	return errors;
}


//Number Conversion Functions
uint8_t DecToBCD(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

uint8_t BCDToDec(uint8_t bin) {
    return ((bin >> 4) * 10) + (bin & 0x0F);
}


//Time Functions
void DS3231_GetSeconds(DS3231* dev)
{
	uint8_t sec;
	dev->status = DS3231_ReadRegister(dev,DS3231_SECONDS_REG,&sec);
	if(dev->status == HAL_OK)
	{
		dev->time[0] = BCDToDec(sec & 0x7F);
		return;
	}
	else
	{
		dev->time[0] = 0;
		return;
	}
}

void DS3231_GetMinutes(DS3231* dev)
{
	uint8_t min;
	dev->status = DS3231_ReadRegister(dev,DS3231_MINUTES_REG,&min);
	if(dev->status == HAL_OK)
	{
		dev->time[1] = BCDToDec(min & 0x7F);
		return;
	}
	else
	{
		dev->time[1] = 0;
		return;
	}
}

void DS3231_GetHours(DS3231* dev)
{
	uint8_t hours;
	dev->status = DS3231_ReadRegister(dev,DS3231_HOURS_REG,&hours);
	if(dev->status == HAL_OK)
	{
		if(DS_READ_BIT(hours,DS3231_12_24) == 0) //If already in 24 hour mode
		{
			dev->time[2] = BCDToDec(hours & 0x6F);
			return;
		}
		else //convert current AM/PM hours to 24-Hour Formats
		{
			uint8_t hour = BCDToDec(hours &0x1F);
			if(DS_READ_BIT(hours,DS3231_AM_PM_20_HOUR) == 1)
			{
				hour = hours + 12;
			}
			dev->time[2] = hour;
			return;
		}
	}
	else
	{
		dev->time[2] = 0;
		return;
	}
}

void DS3231_GetTime(DS3231* dev)
{
	DS3231_GetSeconds(dev);
	DS3231_GetMinutes(dev);
	DS3231_GetHours(dev);
	return;
}

void DS3231_SetSeconds(DS3231* dev, uint8_t sec)
{
	uint8_t conv = DecToBCD(sec & 0x7F);
	dev->status = DS3231_WriteRegister(dev,DS3231_SECONDS_REG,&conv);
	return;
}

void DS3231_SetMinutes(DS3231* dev, uint8_t min)
{
	uint8_t conv = DecToBCD(min & 0x7F);
	dev->status = DS3231_WriteRegister(dev,DS3231_MINUTES_REG,&conv);
	return;
}

void DS3231_SetHours(DS3231* dev, uint8_t hour)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_HOURS_REG,&control);

	uint8_t time = 0;

	if(dev->status == HAL_OK)
	{
		uint8_t bit_12_24 = DS_READ_BIT(control,DS3231_12_24);
		if(bit_12_24 == 1) //if 12/24 bit is set (12-Hour Mode/ AM/PM enabled)
		{
			uint8_t bits12 = control & 0xE0; //preserves 12/24 bit & AM/PM bit

			uint8_t pm = (hour >= 12);
			uint8_t hours_pm = hour % 12;
			if(hours_pm==0)
			{
				hours_pm = 12;
			}

			if(pm)
			{
				DS_SET_BIT(bits12,DS3231_AM_PM_20_HOUR);
			}

			uint8_t conv = DecToBCD(hours_pm & 0x1F);
			time = bits12 | conv;
		}
		else //if 12_24 bit is not set (24-Hour Mode enabled)
		{
			uint8_t bits24 = control & 0xC0; //preserves 12/24 bit

			uint8_t conv = DecToBCD(hour & 0x3F);

			time = bits24 | conv;
		}

		dev->status = DS3231_WriteRegister(dev,DS3231_HOURS_REG,&time);
		return;

	}
	else
	{
		return;
	}

}

void DS3231_SetHourMode(DS3231* dev, DS3231_HourMode mode)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_HOURS_REG,&control);
	if(dev->status == HAL_OK)
	{
		if (mode == Hour_24) //Set to 24-Hour Mode
		{
			if(DS_READ_BIT(control,DS3231_12_24) == 0) //If already in 24 hour mode
			{
				return;
			}
			else //convert current AM/PM hours to 24-Hour Format
			{
				uint8_t hours = BCDToDec(control &0x1F);
				if(DS_READ_BIT(control,DS3231_AM_PM_20_HOUR) == 1)
				{
					hours = hours + 12;
				}
				DS_CLEAR_BIT(control,DS3231_AM_PM_20_HOUR);
				DS_CLEAR_BIT(control,DS3231_12_24);
				DS_CLEAR_BIT(control,7);
				uint8_t new_hour = control | hours;
				dev->status = DS3231_WriteRegister(dev,DS3231_HOURS_REG,&new_hour);
				return;
			}
		}
		else if(mode == Hour_12_AM_PM)//Set to 12-Hour AM/PM Mode
		{
			if(DS_READ_BIT(control,DS3231_12_24) == 1) //If already in 12-Hour AM/PM mode
			{
				return;
			}
			else //convert current 24-Hour to AM/PM Format
			{
				uint8_t hours = BCDToDec(control &0x3F);

				uint8_t pm = (hours >= 12);
				uint8_t hours_pm = hours % 12;
				if(hours_pm==0)
				{
					hours_pm = 12;
				}

				if(pm)
				{
					DS_SET_BIT(control,DS3231_AM_PM_20_HOUR);
				}
				else
				{
					DS_CLEAR_BIT(control,DS3231_AM_PM_20_HOUR);
				}


				DS_SET_BIT(control,DS3231_12_24);
				DS_CLEAR_BIT(control,7);
				uint8_t new_hour = control | hours;
				dev->status = DS3231_WriteRegister(dev,DS3231_HOURS_REG,&new_hour);
				return;
			}

		}
	}
	else
	{
		return;
	}
}

void DS3231_SetTime(DS3231* dev, uint8_t sec, uint8_t min,uint8_t hour)
{
	DS3231_SetSeconds(dev,sec);
	DS3231_SetMinutes(dev,min);
	DS3231_SetHours(dev,hour);
	return;
}


//Date Functions
void DS3231_GetDayOfWeek(DS3231* dev)
{
	uint8_t dow;
	dev->status = DS3231_ReadRegister(dev,DS3231_DAY_OF_WEEK_REG,&dow);
	if(dev->status == HAL_OK)
	{
		dev->date[0] = BCDToDec(dow & 0x07);
	}
	else
	{
		dev->date[0] = 0;
		return;
	}
}

void DS3231_GetDate(DS3231* dev)
{
	uint8_t date;
	dev->status = DS3231_ReadRegister(dev,DS3231_DATE_REG,&date);
	if(dev->status == HAL_OK)
	{
		dev->date[1] = BCDToDec(date & 0x3F);
	}
	else
	{
		dev->date[1] = 0;
		return;
	}
}

void DS3231_GetMonth(DS3231* dev)
{
	uint8_t month;
	dev->status = DS3231_ReadRegister(dev,DS3231_MONTH_REG,&month);
	if(dev->status == HAL_OK)
	{
		dev->date[2] = BCDToDec(month & 0x1F);
		return;
	}
	else
	{
		dev->date[2] = 0;
		return;
	}
}

void DS3231_GetYear(DS3231* dev)
{
	uint8_t year;
	dev->status = DS3231_ReadRegister(dev,DS3231_YEAR_REG,&year);
	if(dev->status == HAL_OK)
	{
		dev->date[3] = BCDToDec(year);
		return;
	}
	else
	{
		dev->date[3] = 0;
		return;
	}
}

void DS3231_GetFullDate(DS3231* dev)
{
	DS3231_GetDayOfWeek(dev);
	DS3231_GetDate(dev);
	DS3231_GetMonth(dev);
	DS3231_GetYear(dev);
	return;
}


void DS3231_SetDayOfWeek(DS3231* dev, uint8_t dow)
{
	uint8_t conv = DecToBCD(dow & 0x07);
	dev->status = DS3231_WriteRegister(dev,DS3231_DAY_OF_WEEK_REG,&conv);
	return;
}

void DS3231_SetDate(DS3231* dev, uint8_t date)
{
	uint8_t conv = DecToBCD(date & 0x3F);
	dev->status = DS3231_WriteRegister(dev,DS3231_DATE_REG,&conv);
	return;
}

void DS3231_SetMonth(DS3231* dev, uint8_t month)
{
	uint8_t conv = DecToBCD(month & 0x1F);
	uint8_t century;
	dev->status = DS3231_ReadRegister(dev, DS3231_MONTH_REG, &century);
	if(dev->status == HAL_OK)
	{
		century = DS_READ_BIT(century,DS3231_CENTURY);

			if(century == 1)
			{
				DS_SET_BIT(conv,DS3231_CENTURY);
				dev->status = DS3231_WriteRegister(dev,DS3231_MONTH_REG,&conv);
				return;
			}
			else
			{
				dev->status = DS3231_WriteRegister(dev,DS3231_MONTH_REG,&conv);
				return;
			}
	}
	else
	{
		return;
	}

}

void DS3231_SetYear(DS3231* dev, uint8_t year)
{
	uint8_t conv = DecToBCD(year);
	dev->status = DS3231_WriteRegister(dev,DS3231_YEAR_REG,&conv);
	return;
}

void DS3231_SetFullDate(DS3231* dev, uint8_t dow, uint8_t date, uint8_t month, uint8_t year)
{
	DS3231_SetDayOfWeek(dev,dow);
	DS3231_SetDate(dev,date);
	DS3231_SetMonth(dev,month);
	DS3231_SetYear(dev,year);
	return;
}


//Alarm Functions


//Alarm 1 Functions
void DS3231_Alarm1Enable(DS3231* dev, DS3231_States state)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	if(dev->status == HAL_OK)
	{
		if(state == Enabled)
		{
			if(DS_READ_BIT(control,DS3231_A1IE))
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_A1IE);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}

		}
		else if(state == Disabled)
		{
			if(!DS_READ_BIT(control,DS3231_A1IE))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_A1IE);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

uint8_t DS3231_IsAlarm1FlagSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG,&control);
	return (DS_READ_BIT(control,DS3231_A1F));
}

void DS3231_SetAlarm1Mode(DS3231* dev,DS3231_Alarmmode1 mode)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_SECONDS_REG,&control);
	control = control | DS_READ_BIT(mode,0) << DS3231_A1M1; //Insert A1M1 bit
	if(dev->status == HAL_OK)
	{
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_SECONDS_REG,&control);

		dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_MINUTES_REG,&control);
		control = control | DS_READ_BIT(mode,1) << DS3231_A1M2; //Insert A1M2 bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_MINUTES_REG,&control);

		dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_HOURS_REG,&control);
		control = control | DS_READ_BIT(mode,2) << DS3231_A1M3; //Insert A1M3 bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_HOURS_REG,&control);

		dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&control);
		control = control | (DS_READ_BIT(mode,3) << DS3231_A1M4) | mode & 0x80; //Insert A1M4 & DY/DT bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&control);
		return;

	}
	else
	{
		return;
	}
}

void DS3231_ClearAlarm1Flag(DS3231*dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG,&control);
	if(dev->status == HAL_OK)
	{
		control = control & 0xFE;
		DS_CLEAR_BIT(control,DS3231_A1F);
		dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_STATUS_REG,&control);
		return;
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm1Seconds(DS3231* dev, uint8_t sec)
{
	uint8_t conv = DecToBCD(sec & 0x7F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_MINUTES_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t sec = (control & 0x80) | conv; //preserve Alarm Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_MINUTES_REG,&sec);
		return;
	}
	else
	{
		return;
	}

}

void DS3231_SetAlarm1Minutes(DS3231* dev, uint8_t min)
{
	uint8_t conv = DecToBCD(min & 0x7F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_MINUTES_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t min = (control & 0x80) | conv; //preserve Alarm Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_MINUTES_REG,&min);
		return;
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm1Hours(DS3231* dev, uint8_t hour)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_HOURS_REG,&control);

	uint8_t time = 0;

	if(dev->status == HAL_OK)
	{
		uint8_t bit_12_24 = DS_READ_BIT(control,DS3231_12_24);
		if(bit_12_24 == 1) //if 12/24 bit is set (12-Hour Mode/ AM/PM already enabled)
		{
			uint8_t bits_alarm_12 = control & 0xE0; //preserves A1M3 & 12/24 bit & AM/PM bit

			uint8_t pm = (hour >= 12);
			uint8_t hours_pm = hour % 12;
			if(hours_pm==0)
			{
				hours_pm = 12;
			}

			if(pm)
			{
				DS_SET_BIT(bits_alarm_12,DS3231_AM_PM_20_HOUR);
			}

			uint8_t conv = DecToBCD(hours_pm & 0x1F);
			time = bits_alarm_12 | conv;
		}
		else //if 12_24 bit is not set (24-Hour Mode already enabled)
		{
			uint8_t bits_alarm_24 = control & 0xC0;//preserves A1M3 & 12/24 bit

			uint8_t conv = DecToBCD(hour & 0x3F);

			time = bits_alarm_24 | conv;
		}

		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_HOURS_REG,&time);
		return;

	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm1HourMode(DS3231* dev, DS3231_HourMode mode)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_HOURS_REG,&control);
	if(dev->status == HAL_OK)
	{
		if (mode == Hour_24) //Set to 24-Hour Mode
		{
			if(DS_READ_BIT(control,DS3231_12_24) == 0) //If already in 24 hour mode
			{
				return;
			}
			else //convert current AM/PM hours to 24-Hour Format
			{
				uint8_t hours = BCDToDec(control &0x1F);
				if(DS_READ_BIT(control,DS3231_AM_PM_20_HOUR) == 1)
				{
					hours = hours + 12;
				}
				DS_CLEAR_BIT(control,DS3231_AM_PM_20_HOUR);
				DS_CLEAR_BIT(control,DS3231_12_24);
				uint8_t new_hour = control | hours;
				dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_HOURS_REG,&new_hour);
				return;
			}
		}
		else if(mode == Hour_12_AM_PM)//Set to 12-Hour AM/PM Mode
		{
			if(DS_READ_BIT(control,DS3231_12_24) == 1) //If already in 12-Hour AM/PM mode
			{
				return;
			}
			else //convert current 24-Hour to AM/PM Format
			{
				uint8_t hours = BCDToDec(control &0x3F);

				uint8_t pm = (hours >= 12);
				uint8_t hours_pm = hours % 12;
				if(hours_pm==0)
				{
					hours_pm = 12;
				}

				if(pm)
				{
					DS_SET_BIT(control,DS3231_AM_PM_20_HOUR);
				}
				else
				{
					DS_CLEAR_BIT(control,DS3231_AM_PM_20_HOUR);
				}


				DS_SET_BIT(control,DS3231_12_24);
				uint8_t new_hour = control | hours;
				dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_HOURS_REG,&new_hour);
				return;
			}

		}
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm1DayOfWeek(DS3231* dev, uint8_t dow)
{
	uint8_t conv = DecToBCD(dow & 0x0F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t dow = (control & 0xC0) | conv; //preserve Alarm Bit & DY/DT-Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&dow);
		return;
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm1Date(DS3231* dev, uint8_t date)
{
	uint8_t conv = DecToBCD(date & 0x3F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t date = (control & 0xC0) | conv; //preserve Alarm Bit & DY/DT-Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&date);
		return;
	}
	else
	{
		return;
	}

}

//Alarm 2 Functions
void DS3231_Alarm2Enable(DS3231* dev, DS3231_States state)
{
	uint8_t control;
		dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
		if(dev->status == HAL_OK)
		{
			if(state == Enabled)
			{
				if(DS_READ_BIT(control,DS3231_A2IE))
				{
					return;
				}
				else
				{
					DS_SET_BIT(control,DS3231_A2IE);
					dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
					return;
				}

			}
			else if(state == Disabled)
			{
				if(!DS_READ_BIT(control,DS3231_A2IE))
				{
					return;
				}
				else
				{
					DS_CLEAR_BIT(control,DS3231_A2IE);
					dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
					return;
				}
			}
		}
		else
		{
			return;
		}
}

uint8_t DS3231_IsAlarm2FlagSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG,&control);
	return (DS_READ_BIT(control,DS3231_A2F));
}

void DS3231_SetAlarm2Mode(DS3231* dev,DS3231_Alarmmode2 mode)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_MINUTES_REG,&control);
	control = control | DS_READ_BIT(mode,0) << DS3231_A2M2; //Insert A2M2 bit
	if(dev->status == HAL_OK)
	{
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_MINUTES_REG,&control);

		dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_HOURS_REG,&control);
		control = control | DS_READ_BIT(mode,1) << DS3231_A2M3; ///Insert A2M3 bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_HOURS_REG,&control);

		dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_DAY_DATE_REG,&control);
		control = control | (DS_READ_BIT(mode,2) << DS3231_A2M4) | mode & 0x80; //Insert A2M4 bit & DY/DT
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_DAY_DATE_REG,&control);
		return;

	}
	else
	{
		return;
	}
}

void DS3231_ClearAlarm2Flag(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG,&control);
	if(dev->status == HAL_OK)
	{
		control = control & 0xFD;
		DS_CLEAR_BIT(control,DS3231_A2F);
		dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_STATUS_REG,&control);
		return;
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm2Minutes(DS3231* dev, uint8_t min)
{
	uint8_t conv = DecToBCD(min & 0x7F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_MINUTES_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t min = (control & 0x80) | conv; //preserve Alarm Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_MINUTES_REG,&min);
		return;
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm2Hours(DS3231* dev, uint8_t hour)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_HOURS_REG,&control);

	uint8_t time = 0;

	if(dev->status == HAL_OK)
	{
		uint8_t bit_12_24 = DS_READ_BIT(control,DS3231_12_24);
		if(bit_12_24 == 1) //if 12/24 bit is set (12-Hour Mode/ AM/PM already enabled)
		{
			uint8_t bits_alarm_12 = control & 0xE0; //preserves A2M3 & 12/24 bit & AM/PM bit

			uint8_t pm = (hour >= 12);
			uint8_t hours_pm = hour % 12;
			if(hours_pm==0)
			{
				hours_pm = 12;
			}

			if(pm)
			{
				DS_SET_BIT(bits_alarm_12,DS3231_AM_PM_20_HOUR);
			}

			uint8_t conv = DecToBCD(hours_pm & 0x1F);
			time = bits_alarm_12 | conv;
		}
		else //if 12_24 bit is not set (24-Hour Mode already enabled)
		{
			uint8_t bits_alarm_24 = control & 0xC0;//preserves A2M3 & 12/24 bit

			uint8_t conv = DecToBCD(hour & 0x3F);

			time = bits_alarm_24 | conv;
		}

		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_HOURS_REG,&time);
		return;

	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm2HourMode(DS3231* dev, DS3231_HourMode mode)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_HOURS_REG,&control);
	if(dev->status == HAL_OK)
	{
		if (mode == Hour_24) //Set to 24-Hour Mode
		{
			if(DS_READ_BIT(control,DS3231_12_24) == 0) //If already in 24 hour mode
			{
				return;
			}
			else //convert current AM/PM hours to 24-Hour Format
			{
				uint8_t hours = BCDToDec(control &0x1F);
				if(DS_READ_BIT(control,DS3231_AM_PM_20_HOUR) == 1)
				{
					hours = hours + 12;
				}
				DS_CLEAR_BIT(control,DS3231_AM_PM_20_HOUR);
				DS_CLEAR_BIT(control,DS3231_12_24);
				uint8_t new_hour = control | hours;
				dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_HOURS_REG,&new_hour);
				return;
			}
		}
		else if(mode == Hour_12_AM_PM)//Set to 12-Hour AM/PM Mode
		{
			if(DS_READ_BIT(control,DS3231_12_24) == 1) //If already in 12-Hour AM/PM mode
			{
				return;
			}
			else //convert current 24-Hour to AM/PM Format
			{
				uint8_t hours = BCDToDec(control &0x3F);

				uint8_t pm = (hours >= 12);
				uint8_t hours_pm = hours % 12;
				if(hours_pm==0)
				{
					hours_pm = 12;
				}

				if(pm)
				{
					DS_SET_BIT(control,DS3231_AM_PM_20_HOUR);
				}
				else
				{
					DS_CLEAR_BIT(control,DS3231_AM_PM_20_HOUR);
				}


				DS_SET_BIT(control,DS3231_12_24);
				uint8_t new_hour = control | hours;
				dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_HOURS_REG,&new_hour);
				return;
			}

		}
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm2DayOfWeek(DS3231* dev, uint8_t dow)
{
	uint8_t conv = DecToBCD(dow & 0x0F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM2_DAY_DATE_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t dow = (control & 0xC0) | conv; //preserve Alarm Bit & DY/DT-Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM2_DAY_DATE_REG,&dow);
		return;
	}
	else
	{
		return;
	}
}

void DS3231_SetAlarm2Date(DS3231* dev, uint8_t date)
{
	uint8_t conv = DecToBCD(date & 0x3F);
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&control);
	if(dev->status == HAL_OK)
	{
		uint8_t date = (control & 0xC0) | conv; //preserve Alarm Bit & DY/DT-Bit
		dev->status = DS3231_WriteRegister(dev,DS3231_ALARM1_DAY_DATE_REG,&date);
		return;
	}
	else
	{
		return;
	}
}


//Configuration Functions
void DS3231_OscillatorEnable(DS3231* dev, DS3231_States state)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	if(dev->status == HAL_OK)
	{
		if(state == Enabled)
		{
			if(!DS_READ_BIT(control,DS3231_EOSC)) //EOSC is negated
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_EOSC);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}

		}
		else if(state == Disabled)
		{
			if(DS_READ_BIT(control,DS3231_EOSC))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_EOSC);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

uint8_t DS3231_IsOscillatorSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	return (!DS_READ_BIT(control,DS3231_EOSC)); //returns 1 for Enabled Oscillator (on Power On) (EOSC is negated)
}

void DS3231_BBSQWEnable(DS3231* dev, DS3231_States state)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	if(dev->status == HAL_OK)
	{
		if(state == Enabled)
		{
			if(DS_READ_BIT(control,DS3231_BBSQW))
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_BBSQW);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}

		}
		else if(state == Disabled)
		{
			if(!DS_READ_BIT(control,DS3231_BBSQW))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_BBSQW);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

uint8_t DS3231_IsBBSQWSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
	return (DS_READ_BIT(control,DS3231_BBSQW)); //returns 1 for BBSQW = 1 (on Power On)
}

void DS3231_RateSelect(DS3231* dev, DS3231_Rate rate)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	if(dev->status == HAL_OK)
	{
		if(rate == Rate_1_HZ)
		{
			if((!DS_READ_BIT(control,DS3231_RS_2) && !DS_READ_BIT(control,DS3231_RS_1)))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_RS_2);
				DS_CLEAR_BIT(control,DS3231_RS_1);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}

		}
		else if(rate == Rate_1024_HZ)
		{
			if((!DS_READ_BIT(control,DS3231_RS_2) && DS_READ_BIT(control,DS3231_RS_1)))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_RS_2);
				DS_SET_BIT(control,DS3231_RS_1);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
		else if(rate == Rate_4096_HZ)
		{
			if(DS_READ_BIT(control,DS3231_RS_2) && !DS_READ_BIT(control,DS3231_RS_1))
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_RS_2);
				DS_CLEAR_BIT(control,DS3231_RS_1);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
		else if(rate == Rate_8192_HZ)
		{
			if(DS_READ_BIT(control,DS3231_RS_2) && DS_READ_BIT(control,DS3231_RS_1))
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_RS_2);
				DS_SET_BIT(control,DS3231_RS_1);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

uint8_t DS3231_IsRateSelectSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	return (DS_READ_BIT(control,DS3231_RS_2) && DS_READ_BIT(control,DS3231_RS_1)); //returns 1 for RS2 & RS1 = 1 (on Power On)
}

void DS3231_InterruptEnable(DS3231* dev, DS3231_States state)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	if(dev->status == HAL_OK)
	{
		if(state == Enabled)//AlarmInterrupt-Mode
		{
			if(DS_READ_BIT(control,DS3231_INTCN))
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_INTCN);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}

		}
		else if(state == Disabled)//SQW-Mode
		{
			if(DS_READ_BIT(control,DS3231_INTCN))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_INTCN);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_REG, &control);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

uint8_t DS3231_IsInterruptSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_REG, &control);
	return (DS_READ_BIT(control,DS3231_INTCN)); //returns 1 for INTCN = 1 (on Power On)
}

void DS3231_32kHzEnable(DS3231* dev, DS3231_States state)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
	if(dev->status == HAL_OK)
	{
		if(state == Enabled)
		{
			if(DS_READ_BIT(control,DS3231_EN32kHz))
			{
				return;
			}
			else
			{
				DS_SET_BIT(control,DS3231_EN32kHz);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
				return;
			}

		}
		else if(state == Disabled)
		{
			if(DS_READ_BIT(control,DS3231_EN32kHz))
			{
				return;
			}
			else
			{
				DS_CLEAR_BIT(control,DS3231_EN32kHz);
				dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

uint8_t DS3231_Is32kHzSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
	return (DS_READ_BIT(control,DS3231_EN32kHz)); //returns 1 for EN32kHz = 1 (on Power On)
}

void DS3231_ClearOscillatorStoppedFlag(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
	DS_CLEAR_BIT(control,DS3231_OSF);
	if(dev->status==HAL_OK)
	{
		dev->status = DS3231_WriteRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
		return;
	}
}

uint8_t DS3231_IsOscillatorStoppedSet(DS3231* dev)
{
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev,DS3231_CONTROL_STATUS_REG, &control);
	return (DS_READ_BIT(control,DS3231_OSF)); //returns 1 for EN32kHz = 1 (on Power On)
}


//Temperature Functions
void DS3231_Get_Temp(DS3231* dev)
{
	uint8_t temp[2];
	dev->status = DS3231_ReadRegisters(dev,DS3231_TEMPERATURE_MSB_REG,temp,sizeof(temp));

	if(dev->status == HAL_OK)
	{
		int8_t msb = (int8_t)temp[0]; // signed integer part
		uint8_t lsb = temp[1];

		int16_t fraction = (lsb >> 6) * 25; // convert to hundredths of °C
		if(msb >= 0)
		{
			dev->temp = msb * 100 + fraction;
			return;
		}
		else
		{
			dev->temp = msb * 100 - fraction;
			return;
		}

	}
	else
	{
		dev->temp = 0;
		return;
	}
}

void DS3231_Force_TempConversion(DS3231 *dev) {
	uint8_t control;
	dev->status = DS3231_ReadRegister(dev, DS3231_CONTROL_STATUS_REG, &control);
	if (dev->status == HAL_OK) {
		uint8_t bsy = DS_READ_BIT(control, DS3231_BSY);
		if (bsy == 1) {
			return;
		} else {
			dev->status = DS3231_ReadRegister(dev, DS3231_CONTROL_REG,
					&control);
			DS_SET_BIT(control, DS3231_CONV);
			dev->status = DS3231_WriteRegister(dev, DS3231_CONTROL_REG,
					&control);
			return;
		}
	} else {
		return;
	}
}

//Low-Level Functions
HAL_StatusTypeDef DS3231_ReadRegister(DS3231* dev, uint8_t reg, uint8_t* data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, DS3231_I2C_ADDRESS,reg, I2C_MEMADD_SIZE_8BIT,data,1,DS3231_DELAY);
}

HAL_StatusTypeDef DS3231_ReadRegisters(DS3231* dev, uint8_t reg, uint8_t* data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, DS3231_I2C_ADDRESS,reg, I2C_MEMADD_SIZE_8BIT,data,length,DS3231_DELAY);
}

HAL_StatusTypeDef DS3231_WriteRegister(DS3231* dev, uint8_t reg, uint8_t* data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, DS3231_I2C_ADDRESS,reg, I2C_MEMADD_SIZE_8BIT,data,1,DS3231_DELAY);
}

HAL_StatusTypeDef DS3231_WriteRegisters(DS3231* dev, uint8_t reg, uint8_t* data, uint8_t length)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, DS3231_I2C_ADDRESS,reg, I2C_MEMADD_SIZE_8BIT,data,length,DS3231_DELAY);
}


#ifdef __cplusplus
}
#endif

