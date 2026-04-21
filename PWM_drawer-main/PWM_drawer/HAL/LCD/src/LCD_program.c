/*
 * LCD_program.c
 *
 * Created: 2/9/2024 10:51:17 PM
 *  Author: Yousef Osama
 */ 


#define F_CPU 16000000UL
#include <util/delay.h>

/* UTILES_LIB */
#include "STD_TYPES.h"
#include "BIT_MATH.h"

/* MCAL */
#include "DIO_interface.h"

/* HAL */
#include "LCD_interface.h"
#include "LCD_private.h"
#include "LCD_config.h"


void LCD_voidInit(void)
{
	// Intialize LCD pins As OutPut Pins
	DIO_voidSetPinDirection(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(LCD_E_PORT , LCD_E_PIN , DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(LCD_D4_PORT, LCD_D4_PIN, DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(LCD_D5_PORT, LCD_D5_PIN, DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(LCD_D6_PORT, LCD_D6_PIN, DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(LCD_D7_PORT, LCD_D7_PIN, DIO_PIN_OUTPUT);
	
	
	_delay_ms(35);
	
	/* Function Set command */
	// set Rs pin = 0 (command)
	DIO_voidSetPinValue(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_LOW);
	
	// set RW pin = 0 (write)
	DIO_voidSetPinValue(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_LOW);
	
	//first row of functionSet
	PRV_voidWriteHalfPort(0b0010);
	
	/* Enable Pulse *//* H => L */
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_HIGH);
	_delay_ms(1);
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_LOW);
	
	//second and third row of FunctionSet //2 lines
	LCD_voidSendCommand(0b00101000);
	_delay_us(45);
	
	// Display on off Control (DisplayOn, Cursor on, Blink on)
	LCD_voidSendCommand(0b00001100);
	_delay_us(45);
	
	// Clear Display
	LCD_voidSendCommand(0b00000001);
	_delay_ms(2);
	
	// Set Entry Mode (Increment on, Shift off)
	LCD_voidSendCommand(0b00000110);
}


void LCD_voidSendCommand(u8 copy_u8Cmnd)
{
	// set Rs pin = 0 (command)
	DIO_voidSetPinValue(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_LOW);
	
	// set RW pin = 0 (write)
	DIO_voidSetPinValue(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_LOW);
	
	// Write The Most 4 bits Of command on Data Pins
	PRV_voidWriteHalfPort(copy_u8Cmnd>>4);
	
	/* Enable Pulse *//* H => L */
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_HIGH);
	_delay_ms(1);
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_LOW);
	
	// Write The Least 4 bits Of command on Data Pins
	PRV_voidWriteHalfPort(copy_u8Cmnd);
	
	/* Enable Pulse *//* H => L */
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_HIGH);
	_delay_ms(1);
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_LOW);
}


void LCD_voidDisplayChar(u8 copy_u8Data)
{
	// set Rs pin = 1 (data)
	DIO_voidSetPinValue(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_HIGH);
	
	// set RW pin = 0 (write)
	DIO_voidSetPinValue(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_LOW);
	
	// Write The Most 4 bits Of data on Data Pins
	PRV_voidWriteHalfPort(copy_u8Data>>4);
	
	/* Enable Pulse *//* H => L */
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_HIGH);
	_delay_ms(1);
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_LOW);
	
	// Write The Least 4 bits Of data on Data Pins
	PRV_voidWriteHalfPort(copy_u8Data);
	
	/* Enable Pulse *//* H => L */
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_HIGH);
	_delay_ms(1);
	DIO_voidSetPinValue(LCD_E_PORT,LCD_E_PIN, DIO_PIN_LOW);
}


void LCD_voidDisplayString(u8* copy_pu8String)
{
	if(copy_pu8String!=NULL)
	{
		u8 local_u8Counter=0;
		while(copy_pu8String[local_u8Counter]!='\0')
		{
			LCD_voidDisplayChar(copy_pu8String[local_u8Counter]);
			local_u8Counter++;
		}
	}
	else
	{
		// return Error state
	}
}


static void PRV_voidWriteHalfPort(u8 copy_u8Value)
{
	if(1 == GET_BIT(copy_u8Value,0))
	{
		DIO_voidSetPinValue(LCD_D4_PORT, LCD_D4_PIN, DIO_PIN_HIGH);
	}
	else
	{
		DIO_voidSetPinValue(LCD_D4_PORT, LCD_D4_PIN, DIO_PIN_LOW);
	}
	
	if(1 == GET_BIT(copy_u8Value,1))
	{
		DIO_voidSetPinValue(LCD_D5_PORT, LCD_D5_PIN, DIO_PIN_HIGH);
	}
	else
	{
		DIO_voidSetPinValue(LCD_D5_PORT, LCD_D5_PIN, DIO_PIN_LOW);
	}
	
	if(1 == GET_BIT(copy_u8Value,2))
	{
		DIO_voidSetPinValue(LCD_D6_PORT, LCD_D6_PIN, DIO_PIN_HIGH);
	}
	else
	{
		DIO_voidSetPinValue(LCD_D6_PORT, LCD_D6_PIN, DIO_PIN_LOW);
	}
	
	if(1 == GET_BIT(copy_u8Value,3))
	{
		DIO_voidSetPinValue(LCD_D7_PORT, LCD_D7_PIN, DIO_PIN_HIGH);
	}
	else
	{
		DIO_voidSetPinValue(LCD_D7_PORT, LCD_D7_PIN, DIO_PIN_LOW);
	}
}

void LCD_voidDisplayNumber(u32 copy_u32Number)
{
	u8 local_u8Str[50];
	sprintf(local_u8Str,"%lu",copy_u32Number);//"%lu"
	LCD_voidDisplayString(local_u8Str);
}

void LCD_voidClear               (void)
{
	LCD_voidSendCommand(0b00000001);
}
void LCD_voidShift               (u8 copy_u8ShifttingDirection)
{
	_delay_us(45);
	if((copy_u8ShifttingDirection<4)&&(copy_u8ShifttingDirection>-1))
	{
		switch(copy_u8ShifttingDirection)
		{
			case LCD_CURSOR_LEFT:
			LCD_voidSendCommand(0b00010000);
			_delay_us(45);
			break;
			case LCD_CURSOR_RIGHT:
			LCD_voidSendCommand(0b00010100);
			_delay_us(45);
			break;
			
			case LCD_DISPLAY_LEFT:
			LCD_voidSendCommand(0b00011000);
			_delay_us(45);
			break;
			
			case LCD_DISPALY_RIGHT:
			LCD_voidSendCommand(0b00011100);
			_delay_us(45);
			break;
		}
	}
	else
	{
		//retun error state 
	}
}

void LCD_voidGoToSpecificPosition(u8 copy_u8LineNumber, u8 copy_u8Position)
{
	u8 local_address;
	if (((copy_u8LineNumber <= 2 && copy_u8LineNumber > 0) && (copy_u8Position < 40 && copy_u8Position >= 0)))
	{
		switch (copy_u8LineNumber)
		{
			case LCD_FIRST_LINE:
			if(copy_u8Position <= 15)
				local_address = copy_u8Position;
			else
					local_address = copy_u8Position;
			break;
			case LCD_SECOND_LINE:
			local_address = copy_u8Position + 0x40;
			break;
		}
		LCD_voidSendCommand(0x80 | local_address);
		_delay_ms(45);
	}
	else
	{
		// Handle error state or return an error code
	}
}
void LCD_voidClearSpecificPosition(u8 copy_u8LineNumber, u8 copy_u8Position)
 {
	LCD_voidGoToSpecificPosition(copy_u8LineNumber, copy_u8Position);
	LCD_voidDisplayChar(' ');
}
void LCD_voidDisplayPWMCalculations(u32 copy_u32Frequency,u32 copy_u32PeriodicTime,u32 copy_u32DutyCycle,u32 cop_u32Ton,u32 copu_u32Toff)
{	
LCD_voidGoToSpecificPosition(1,0);
LCD_voidDisplayString((u8*)"f=");
LCD_voidDisplayNumber(copy_u32Frequency);
LCD_voidDisplayString((u8*)"Hz ");
LCD_voidDisplayString((u8*)"DUTY_Cycle=");
LCD_voidDisplayNumber(copy_u32PeriodicTime);
LCD_voidDisplayString((u8*)"ms ");
LCD_voidDisplayString((u8*)"Duty= ");
LCD_voidDisplayNumber(copy_u32DutyCycle);
LCD_voidDisplayChar(37);
LCD_voidGoToSpecificPosition(2,0);
LCD_voidDisplayString((u8*)"Ton=");
LCD_voidDisplayNumber(cop_u32Ton);
LCD_voidDisplayString((u8*)"ms ");
LCD_voidDisplayString((u8*)"Toff=");
LCD_voidDisplayNumber(copu_u32Toff);
LCD_voidDisplayString((u8*)"ms ");
}
void LCD_voidDisplayPWMSignal(u32 copy_u32DutyCycle,u32 copy_u32Frequency,u32 copy_u32Ton,u32 copy_u32Toff)
{
	/*
	u8 local_counter;
	u8 lcd_position = 18 + copy_u32Ton +1; // Calculate the starting position for the "off" signal

	// Draw the "on" signal
	LCD_voidDisplayChar('|');
	for(local_counter=0 ;local_counter < copy_u32Ton ;local_counter++)
	{
		LCD_voidDisplayChar('-');
	}
	LCD_voidDisplayChar('|');
	_delay_ms(500);

	// Draw the "off" signal
	LCD_voidGoToSpecificPosition(2, lcd_position);
	for(local_counter=0 ;local_counter < copy_u32Toff ;local_counter++)
	{
		LCD_voidDisplayChar('_');
	}
	LCD_voidClearSpecificPosition(2,18);
	LCD_voidDisplayString((u8*)"_|");
	// Draw the "on" signal
	for(local_counter=0 ;local_counter < copy_u32Ton;local_counter++)
	{
		LCD_voidDisplayChar('-');
	}
	LCD_voidDisplayChar('|');
	// Draw the "off" signal
	LCD_voidGoToSpecificPosition(2, lcd_position);
	for(local_counter=0 ;local_counter < copy_u32Toff ;local_counter++)
	{
		LCD_voidDisplayChar('_');
	}
	*/
	
	
	if(copy_u32Frequency < 50)
	{
		
	if(copy_u32DutyCycle <15)
	{
		LCD_voidDisplayString((u8*)"|-|_|-|_|-|_|-|_|-|_");
		LCD_voidGoToSpecificPosition(2,18);
		_delay_ms(400);
		LCD_voidDisplayString((u8*)"_|-|_|-|_|-|_|-|_|-|_");
	}
	else if(copy_u32DutyCycle <25)
	{
		LCD_voidDisplayString((u8*)"--|__|--|__");
		_delay_ms(400);
		LCD_voidGoToSpecificPosition(2,18);
		LCD_voidDisplayString((u8*)"__|--|__|--|__|");
		
	}
	else if(copy_u32DutyCycle <50)
	{
		LCD_voidDisplayString((u8*)"---|___");
		_delay_ms(500);
		LCD_voidGoToSpecificPosition(2,18);
		LCD_voidDisplayString((u8*)"___|---|___|");
	}
	else if(copy_u32DutyCycle <75)
	{
		LCD_voidDisplayString((u8*)"----|____");
		_delay_ms(400);
		LCD_voidGoToSpecificPosition(2,18);
		LCD_voidDisplayString((u8*)"____|----|____|");
	}
	else
	{
		LCD_voidDisplayString((u8*)"|-----|_____");
		_delay_ms(400);
		LCD_voidGoToSpecificPosition(2,18);
		LCD_voidDisplayString((u8*)"_____|-----|_____");
	}
	}
	else
	{
			if(copy_u32DutyCycle <15)
			{
				LCD_voidDisplayString((u8*)"|-|_|-|_|-|_|");
				LCD_voidGoToSpecificPosition(2,19);
				LCD_voidDisplayString((u8*)"_|-|_|-|_|-|_|");
			}
			else if(copy_u32DutyCycle <25)
			{
				LCD_voidDisplayString((u8*)"|--|__|--|__|");
				LCD_voidGoToSpecificPosition(2,18);
				LCD_voidDisplayString((u8*)"__|--|__|--|__|");
				
			}
			else if(copy_u32DutyCycle <50)
			{
				LCD_voidDisplayString((u8*)"---|___");
				LCD_voidGoToSpecificPosition(2,18);
				LCD_voidDisplayString((u8*)"___|---|___|");
			}
			else if(copy_u32DutyCycle <75)
			{
				LCD_voidDisplayString((u8*)"----|____");
				LCD_voidGoToSpecificPosition(2,18);
				LCD_voidDisplayString((u8*)"____|----|____|");
			}
			else
			{
				LCD_voidDisplayString((u8*)"|-----|_____");
				LCD_voidGoToSpecificPosition(2,18);
				LCD_voidDisplayString((u8*)"_____|-----|______");
			}
	}
	
}