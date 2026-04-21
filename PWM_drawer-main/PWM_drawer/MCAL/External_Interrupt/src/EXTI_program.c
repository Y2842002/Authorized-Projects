/*
 * EXTI_program.c
 *
 * Created: 2/18/2024 9:52:02 PM
 *  Author: Yousef Osama Mohamed
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "EXTI_register.h"
#include "EXTI_interface.h"

//PB2 INT2

//PD2 INT0

//PD3 INT1
void EXTI_voidInit(u8 copy_u8InterruptSource, u8 copy_u8TriggerEdge)
{
	//Triggering Mode
	switch(copy_u8InterruptSource)
	{
		case EXTI_INT0:
		switch(copy_u8TriggerEdge)
		{
			case EXTI_RISING_EDGE:
			SET_BIT(MCUCR_REG,ISC00);
			SET_BIT(MCUCR_REG,ISC01);
			break;
			
			case EXTI_FALLING_EDGE:
			CLR_BIT(MCUCR_REG,ISC00);
			SET_BIT(MCUCR_REG,ISC01);
			break;
			
			case EXTI_LOW_LEVEL:
			CLR_BIT(MCUCR_REG,ISC00);
			CLR_BIT(MCUCR_REG,ISC01);
			break;
			
			case EXTI_ANY_LOGICAL_CHANGE:
			SET_BIT(MCUCR_REG,ISC00);
			CLR_BIT(MCUCR_REG,ISC01);
			break;
		}
		//Enable EXTERNAL INTERRUPT ZERO(PERIPHRAL INTERRUPT ENABLE)
		SET_BIT(GICR_REG,INT0);
		break;
		
		case EXTI_INT1:
		switch(copy_u8TriggerEdge)
		{
			case EXTI_RISING_EDGE:
			SET_BIT(MCUCR_REG,ISC11);
			SET_BIT(MCUCR_REG,ISC10);
			break;
			
			case EXTI_FALLING_EDGE:
			SET_BIT(MCUCR_REG,ISC11);
			CLR_BIT(MCUCR_REG,ISC10);
			break;
			
			case EXTI_ANY_LOGICAL_CHANGE:
			SET_BIT(MCUCR_REG,ISC10);
			CLR_BIT(MCUCR_REG,ISC11);
			break;
			
			case EXTI_LOW_LEVEL:
			CLR_BIT(MCUCR_REG,ISC11);
			CLR_BIT(MCUCR_REG,ISC10);
			break;
		}
		SET_BIT(GICR_REG,INT1);
		break;
		
		case EXTI_INT2:
		switch(copy_u8TriggerEdge)
		{
			case EXTI_FALLING_EDGE:
			CLR_BIT(MCUCSR_REG,ISC2);
			break;
			
			case EXTI_RISING_EDGE:
			SET_BIT(MCUCSR_REG,ISC2);
		}
		SET_BIT(GICR_REG,INT2);
		break;
	}
}
void EXTI_voidDisable(u8 copy_u8InterruptSource)
{
	//DISABLE EXTERNAL INTERRPUT PID (PERIPHRAL INTERRUPT DISABLE)
	switch(copy_u8InterruptSource)
	{
		case EXTI_INT0:
		CLR_BIT(GICR_REG,INT0);
		break;
		
		case EXTI_INT1:
		CLR_BIT(GICR_REG,INT1);
		break;
		
		case EXTI_INT2:
		CLR_BIT(GICR_REG,INT2);
		break;
	}
}