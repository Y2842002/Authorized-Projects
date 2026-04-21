/*
 * GI_program.c
 *
 * Created: 3/1/2024 2:34:48 AM
 *  Author: Yousef Osama Mohamed
 */ 


#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "GI_interface.h"
#include "GI_register.h"


void GI_voidEnable(void)
{
	// Enable GI
	SET_BIT(SREG_REG,I);
}

void GI_voidDisable(void)
{
	//Disable GI
	CLR_BIT(SREG_REG,I);
}