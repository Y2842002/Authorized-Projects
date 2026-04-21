/*
 * SWITCH_program.c
 *
 * Created: 2/3/2024 9:05:12 PM
 *  Author: Yousef Osama
 */ 


 
 /* UTILES_LIB */
 #include "STD_TYPES.h"
 #include "BIT_MATH.h"

 /* MCAL */
 #include "DIO_interface.h"

 /* HAL */
 #include "SWITCH_interface.h"
 
 
void SWITCH_voidInit(u8 copy_u8SwitchPortId, u8 copy_u8SwitchPinId)
{
	DIO_voidSetPinDirection(copy_u8SwitchPortId,copy_u8SwitchPinId,DIO_PIN_INPUT);
}


void SWITCH_voidGetState(u8 copy_u8SwitchPortId, u8 copy_u8SwitchPinId, u8 copy_u8SwitchConnectionType, u8* copy_pu8SwitchState)
{
	if(copy_pu8SwitchState!=NULL)
	{
		u8 local_u8PinValue;
		DIO_voidGetPinValue(copy_u8SwitchPortId,copy_u8SwitchPinId,&local_u8PinValue);
		switch(copy_u8SwitchConnectionType)
		{
			case SWITCH_FORWARD_CONNECTION:
			if(1 == local_u8PinValue)
			{
				*copy_pu8SwitchState = SWITCH_PRESSED;
			}
			else
			{
				*copy_pu8SwitchState = SWITCH_NOT_PRESSED;
			}
			break;
			
			case SWITCH_REVERSE_CONNECTION:
			if(0 == local_u8PinValue)
			{
				*copy_pu8SwitchState = SWITCH_PRESSED;
			}
			else
			{
				*copy_pu8SwitchState = SWITCH_NOT_PRESSED;
			}
			break;
		}
	}
	else
	{
		//return Error State
	}
}