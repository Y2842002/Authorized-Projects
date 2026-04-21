/*
 * TIMER0_program.c
 *
 * Created: 2/29/2024 1:16:22 AM
 *  Author: Yousef Osama Mohamed
 */ 

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "TIMER0_configuration.h"
#include "TIMER0_interface.h"
#include "TIMER0_register.h"
#include "TIMER0_private.h"

static void(*PRV_pFunCallBackOVF)(void)=NULL;
static void(*PRV_pFunCallBackCTC)(void)=NULL;


void TMR0_voidInit(void)
{
	//#if TMR0_MODE == TMR0_NORMAL_MODE
	// Select Mode = Normal Mode
	CLR_BIT(TCCR0_REG,WGM00);
	CLR_BIT(TCCR0_REG,WGM01);
	
	// Enable OVF Interrupt
	SET_BIT(TIMSK_REG,TOIE0);
	
	// Init Timer With Preload Value
	//TCNT0_REG = TMR0_PRELOAD_VALUE;

/*
	#elif TMR0_MODE == TMR0_CTC_MODE
	 //Select Mode = CTC Mode
	CLR_BIT(TCCR0_REG,WGM00);
	SET_BIT(TCCR0_REG,WGM01);
	
	// Enable OC Interrupt
	SET_BIT(TIMSK_REG,OCIE0);
	
	// Init Timer With output compare Value
	OCR0_REG = TMR0_COMPARE_VALUE;
	#endif
	*/
}


void TMR0_voidStart(void)
{

	
	
	// Select Prescaler Value = 64 
	
	SET_BIT(TCCR0_REG,CS00);
	SET_BIT(TCCR0_REG,CS01);
	CLR_BIT(TCCR0_REG,CS02);
	
	
}


void TMR0_voidStop(void)
{
	CLR_BIT(TCCR0_REG,CS00);
	CLR_BIT(TCCR0_REG,CS01);
	CLR_BIT(TCCR0_REG,CS02);
}


void TMR0_voidSetCallBackOVF(void(*copy_pFunAction)(void))
{
	if(copy_pFunAction!=NULL)
	{
		PRV_pFunCallBackOVF = copy_pFunAction;
	}
	else
	{
		// return Error State
	}
}


void TMR0_voidSetCallBackCTC(void(*copy_pFunAction)(void))
{
	if(copy_pFunAction!=NULL)
	{
		PRV_pFunCallBackCTC = copy_pFunAction;
	}
	else
	{
		// return Error State
	}
}

/*
void __vector_11(void) __attribute__ ((signal));
void __vector_11(void)
{
	static u16 local_u16OVFCounter = 0;
	local_u16OVFCounter++;
	
	if(TMR0_OVER_FLOW_COUNTER == local_u16OVFCounter)
	{
		// Init Timer With Preload Value
		TCNT0_REG = TMR0_PRELOAD_VALUE;
		
		// Clear Counter
		local_u16OVFCounter = 0;
		
		// Call action
		if(PRV_pFunCallBackOVF!=NULL)
		{
			PRV_pFunCallBackOVF();
		}
	}
}


void __vector_10(void) __attribute__ ((signal));
void __vector_10(void)
{
	static u16 local_u16CTCCounter = 0;
	local_u16CTCCounter++;
	
	if(TMR0_OUTPUT_COMPARE_COUNTER == local_u16CTCCounter)
	{
		// Clear Counter
		local_u16CTCCounter = 0;
		
		// Call action
		if(PRV_pFunCallBackCTC!=NULL)
		{
			PRV_pFunCallBackCTC();
		}
	}
}
*/