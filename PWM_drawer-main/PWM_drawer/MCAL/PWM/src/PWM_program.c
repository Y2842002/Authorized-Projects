/*
 * PWM_program.c
 *
 * Created: 3/1/2024 7:51:41 PM
 *  Author: Yousef Osama Mohamed
 */ 

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "PWM_private.h"
#include "PWM_interface.h"
#include "PWM_register.h"
#include "PWM_configuration.h"


void PWM_voidInitChannel_0              (void)
{
	    // Select Mode = Fast Normal Mode 
		SET_BIT(TCCR0_REG,WGM00);
		SET_BIT(TCCR0_REG,WGM01);
		
		//Select Non Inverting Mode PWM
		SET_BIT(TCCR0_REG,COM01);
		CLR_BIT(TCCR0_REG,COM00);
		
}
void PWM_voidGenerateChannel_PWM_Channel_0(u8 copy_u8DutyCycle)
{
	if(copy_u8DutyCycle <=100)
	{
		
	//Under Condition fast PWM in NON Inverting Mode
	OCR0_REG = ((copy_u8DutyCycle * 256)/100)-1;
	
	//Set PreScaler Value = 64
	SET_BIT(TCCR0_REG,CS00);
	SET_BIT(TCCR0_REG,CS01);
	CLR_BIT(TCCR0_REG,CS02);
	}
	else
	{
		//retun error state 
	}
}
void PWM_voidInitChannel_1A(void)
{
	// Select Mode = Fast PWM Mode(14)
	CLR_BIT(TCCR1A_REG,WGM10);
	SET_BIT(TCCR1A_REG,WGM11);
	SET_BIT(TCCR1B_REG,WGM12);
	SET_BIT(TCCR1B_REG,WGM13);
	
	// Select Non Inverting Mode
	CLR_BIT(TCCR1A_REG,COM1A0);
	SET_BIT(TCCR1A_REG,COM1A1);
}

void PWM_voidGenerate_PWM_Channel_1A(u16 copy_u16Frequency_hz,f32 copy_f32DutyCycle)
{
	if(copy_f32DutyCycle<=100)
	{
		// under condition tick time 4 MS by setting prescaller 64
		ICR1_u16_REG = ((1000000UL/copy_u16Frequency_hz)/4)-1;
		
		// under condition non inverting fast PWM
		OCR1A_u16_REG = ((copy_f32DutyCycle*(ICR1_u16_REG+1))/100)-1;
		
		// Select Prescaler Value = 64
		SET_BIT(TCCR1B_REG,CS10);
		SET_BIT(TCCR1B_REG,CS11);
		CLR_BIT(TCCR1B_REG,CS12);
	}
	else
	{
		// return Error state
	}
}
void PWM_voidInitChannel_2(void)
{
	
	// Select Mode = Fast Normal Mode
	SET_BIT(TCCR2_REG,WGM21);
	SET_BIT(TCCR2_REG,WGM20);
	
		//Select Non Inverting Mode PWM
		SET_BIT(TCCR2_REG,COM21);
		CLR_BIT(TCCR2_REG,COM20);
}	
void PWM_voidGenerateChannel_PWM_Channel_2(u8 copy_u8DutyCycle)
{
		if(copy_u8DutyCycle <=100)
		{
			
			//Under Condition fast PWM in NON Inverting Mode
			OCR2_REG = ((copy_u8DutyCycle * 256)/100)-1;
			
			//Set PreScaler Value = 64
			SET_BIT(TCCR2_REG,CS22);
			CLR_BIT(TCCR2_REG,CS21);
			CLR_BIT(TCCR2_REG,CS20);
		}
		else
		{
			//retun error state
		}
}
void PWM_voidDutyCycleCalculations(u32* copy_pu32DutyCycle,u32 copy_pu32onTicks,u32 copy_pu32onCounter,u32 copy_pu32totalTicks,u32 copy_pu32totalCounter)
{
	 
	 
	if( copy_pu32DutyCycle!= NULL)
	{
	   		*copy_pu32DutyCycle = (copy_pu32onTicks + (copy_pu32onCounter * 256)) * 100 / (copy_pu32totalTicks + (copy_pu32totalCounter * 256));
	}
	else
	{
		//return any error state
	}
}
void PWM_voidFrequencyCalculation(u32* copy_pu32Frequency ,u32 copy_pu32totalTicks,u32 copy_pu32totalCounter)
{
	 
	 if(  copy_pu32Frequency!= NULL)
	 {
		 *copy_pu32Frequency = 1000000UL / ((copy_pu32totalTicks + (copy_pu32totalCounter * 256)  ) * 4);
	 }
	 else
	 {
		 //return any error state
	 }
}
void PWM_voidPeriodicTimeCalculations(u32 copy_pu32Frequency,u32* copy_pu32PeriodicTime)
{
	if( copy_pu32PeriodicTime!= NULL)
	{
		
	*copy_pu32PeriodicTime = (1000UL /(copy_pu32Frequency));// in millisecond
	}
	else
	{
		
	}
}
void PWM_voidOnTimeDuration(u32 copy_pu32PeriodicTime,u32 copy_pu32DutyCycle,u32* copy_pu32Ton)
{
	if(  copy_pu32Ton != NULL)
	{
		
	*copy_pu32Ton = ((((float)copy_pu32DutyCycle)/100UL)*(copy_pu32PeriodicTime));
	}
	else
	{
		//return any error state
	}
}
void PWM_voidOffTimeDuration(u32 copy_pu32PeriodicTime,u32 copy_pu32Ton,u32* copy_pu32TOff)
{
	if(copy_pu32TOff != NULL)
	{
		
	*copy_pu32TOff = (copy_pu32PeriodicTime - copy_pu32Ton);
	}
	else
	{
		//return any error state
	}
}

