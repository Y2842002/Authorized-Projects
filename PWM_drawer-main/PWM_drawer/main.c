/*
 * PWM_Reader.c
 *
 * Created: 5/2/2024 5:06:22 PM
 * Author : Yousef Osama Mohamed
 */ 
/*********  UTIL  ************/ 
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#define F_CPU 16000000UL
#include <util/delay.h>


/*********  HAL  ************/ 
#include "LCD_interface.h"
#include "KPD_interface.h"

/*********  MCAL ***********/
#include "DIO_interface.h"
#include "EXTI_interface.h"
#include "EEPROM_interface.h"
/* TIMER 0 */ 
#include "TIMER0_interface.h"
#include "TIMER0_register.h"
#include "GI_interface.h"
#include "PWM_interface.h"
#include "SWITCH_interface.h"



 u32 Global_ovfCounter,Global_state,Global_onTicks,Global_onCounter,Global_totalTicks,Global_totalCounter;
int main(void)
{
	 u32 local_u32DutyCycle,local_u32Frequency,local_u32PeriodicTime,local_Ton,local_Toff;
	 u8 local_u8Switchstate1,local_u8Switchstate2,local_u8Switchstate3,local_u8Switchstate4; 
	 u8 local_firstIteration = 1 ;
	 
	GI_voidEnable();
	LCD_voidInit();
	TMR0_voidInit();
	PWM_voidInitChannel_1A();
	
	DIO_voidSetPinDirection(DIO_PORTD,DIO_PIN5,DIO_PIN_OUTPUT);
	DIO_voidSetPinDirection(DIO_PORTD,DIO_PIN2,DIO_PIN_INPUT);

	SWITCH_voidInit(DIO_PORTC,DIO_PIN0);
	SWITCH_voidInit(DIO_PORTC,DIO_PIN1);
	SWITCH_voidInit(DIO_PORTC,DIO_PIN2);
	SWITCH_voidInit(DIO_PORTC,DIO_PIN3);
	
	LCD_voidDisplayString((u8*)"Welcome to Yousef's Project");
	_delay_ms(1000);
	LCD_voidGoToSpecificPosition(2,0);
	LCD_voidDisplayString((u8*)"PWM_reader Amit 2024 :)");
	_delay_ms(1500);
	
	
	EXTI_voidInit(EXTI_INT0,EXTI_RISING_EDGE);
	TMR0_voidStart();
	while (1)
	{
		
		SWITCH_voidGetState(DIO_PORTC, DIO_PIN0, SWITCH_FORWARD_CONNECTION, &local_u8Switchstate1);
		SWITCH_voidGetState(DIO_PORTC, DIO_PIN1, SWITCH_FORWARD_CONNECTION, &local_u8Switchstate2);
		SWITCH_voidGetState(DIO_PORTC, DIO_PIN2, SWITCH_FORWARD_CONNECTION, &local_u8Switchstate3);
		SWITCH_voidGetState(DIO_PORTC, DIO_PIN3, SWITCH_FORWARD_CONNECTION, &local_u8Switchstate4);
	if (local_u8Switchstate1 == SWITCH_PRESSED)
	{
		PWM_voidGenerate_PWM_Channel_1A(25, 75); // Low Frequency High Duty
	}
	if (local_u8Switchstate2 == SWITCH_PRESSED)
	{
		PWM_voidGenerate_PWM_Channel_1A(50, 15); // High Frequency low Duty
	}
	if (local_u8Switchstate3 == SWITCH_PRESSED)
	{
		PWM_voidGenerate_PWM_Channel_1A(50, 85); // High Frequency High Duty
	}
	if (local_u8Switchstate4 == SWITCH_PRESSED)
	{
		PWM_voidGenerate_PWM_Channel_1A(165, 95);// Very High Frequency Very High Duty
	}
	if(local_u8Switchstate1 != SWITCH_PRESSED && local_u8Switchstate2 != SWITCH_PRESSED && local_u8Switchstate3 != SWITCH_PRESSED && local_u8Switchstate4 != SWITCH_PRESSED)
	{
		PWM_voidGenerate_PWM_Channel_1A(25, 10); // Default Low Frequency Low Duty
	}



			if(Global_state == 3)
			{ 
					EXTI_voidInit(EXTI_INT0,EXTI_RISING_EDGE);
					Global_state=0;
					Global_ovfCounter=0;
					
					PWM_voidDutyCycleCalculations(&local_u32DutyCycle,Global_onTicks,Global_onCounter,Global_totalTicks,Global_totalCounter);
					PWM_voidFrequencyCalculation(&local_u32Frequency,Global_totalTicks,Global_totalCounter);
					PWM_voidPeriodicTimeCalculations(local_u32Frequency,&local_u32PeriodicTime);
					PWM_voidOnTimeDuration(local_u32PeriodicTime,local_u32DutyCycle,&local_Ton);
					PWM_voidOffTimeDuration(local_u32PeriodicTime,local_Ton,&local_Toff);
				if(local_firstIteration)
				{
					local_firstIteration = 0;
				}
				else
				{
				LCD_voidDisplayPWMCalculations(local_u32Frequency,local_u32PeriodicTime,local_u32DutyCycle,local_Ton,local_Toff);
				LCD_voidDisplayPWMSignal(local_u32DutyCycle,local_u32Frequency,local_Ton,local_Toff);
				}
			}		
		}
	}

void __vector_11(void) __attribute__ ((signal));
void __vector_11(void)
{
	Global_ovfCounter++;
}
void __vector_1(void) __attribute__ ((signal));
void __vector_1(void)
{
	if(Global_state == 0)
	{ 
		TCNT0_REG = 0;
		Global_ovfCounter = 0;
		EXTI_voidInit(EXTI_INT0,EXTI_FALLING_EDGE);
		Global_state++;
	}
	else if(Global_state == 1)
	{
		Global_onTicks = TCNT0_REG;
		Global_onCounter = Global_ovfCounter;
		EXTI_voidInit(EXTI_INT0,EXTI_RISING_EDGE);
		Global_state++;
	}
	else if(Global_state == 2)
	{
		Global_totalTicks = TCNT0_REG;
		Global_totalCounter = Global_ovfCounter;
	    Global_state++;
	}
}