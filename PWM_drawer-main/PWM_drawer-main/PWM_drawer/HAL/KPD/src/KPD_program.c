/*
 * KPD_program.c
 *
 * Created: 2/16/2024 3:31:00 PM
 *  Author: Yousef Osama Mohamed
 */ 

/* HAL */
#include "KPD_interface.h"
#include "KPD_config.h"

/* MCAL */
#include "DIO_interface.h"

/* UTILES_LIB */
#include "STD_TYPES.h"
#include "BIT_MATH.h"

void KPD_voidInit(void)
{
	//Intialize KPD column pins as Output Pins
	
        DIO_voidSetPinDirection(KPD_COL0_PORT,KPD_COL0_PIN,DIO_PIN_OUTPUT);
        DIO_voidSetPinDirection(KPD_COL1_PORT,KPD_COL1_PIN,DIO_PIN_OUTPUT);
        DIO_voidSetPinDirection(KPD_COL2_PORT,KPD_COL2_PIN,DIO_PIN_OUTPUT);
        DIO_voidSetPinDirection(KPD_COL3_PORT,KPD_COL3_PIN,DIO_PIN_OUTPUT);
	
	//Intialize KPD Rows pins as Input pins
		DIO_voidSetPinDirection(KPD_ROW0_PORT,KPD_ROW0_PIN,DIO_PIN_INPUT);
		DIO_voidSetPinDirection(KPD_ROW1_PORT,KPD_ROW1_PIN,DIO_PIN_INPUT);
		DIO_voidSetPinDirection(KPD_ROW2_PORT,KPD_ROW2_PIN,DIO_PIN_INPUT);
		DIO_voidSetPinDirection(KPD_ROW3_PORT,KPD_ROW3_PIN,DIO_PIN_INPUT);
		
		
	//Activate Internal Pull UP Resistance
		DIO_voidActivePinInPullUpResistance(KPD_ROW0_PORT,KPD_ROW0_PIN);
		DIO_voidActivePinInPullUpResistance(KPD_ROW1_PORT,KPD_ROW1_PIN);
		DIO_voidActivePinInPullUpResistance(KPD_ROW2_PORT,KPD_ROW2_PIN);
		DIO_voidActivePinInPullUpResistance(KPD_ROW3_PORT,KPD_ROW3_PIN);
	
}


void KPD_voidGetValue(u8* copy_pu8ReturnedValue)
{
	if(copy_pu8ReturnedValue != NULL)
	{
		u8 local_u8ColsCounter;
		u8 local_u8RowsCounter;
		u8 local_u8RowValue;
		
		u8 local_u8Keys[4][4] = KPD_KEYS;
		
		u8 local_u8ArrayColsPorts[4] = {KPD_COL0_PORT,KPD_COL1_PORT,KPD_COL2_PORT,KPD_COL3_PORT};
		u8 local_u8ArrayColsPins [4] = {KPD_COL0_PIN,KPD_COL1_PIN,KPD_COL2_PIN,KPD_COL3_PIN};
			
		u8 local_u8ArrayROWPorts[4] = {KPD_ROW0_PORT,KPD_ROW1_PORT,KPD_ROW2_PORT,KPD_ROW3_PORT};
		u8 local_u8ArrayROWPins [4] = {KPD_ROW0_PIN , KPD_ROW1_PIN, KPD_ROW2_PIN, KPD_ROW3_PIN };	
		*copy_pu8ReturnedValue = KPD_NOT_PRESSED;
		for (local_u8ColsCounter = 0; local_u8ColsCounter < 4; local_u8ColsCounter++)
		{
			//Activate each Column
			DIO_voidSetPinValue(local_u8ArrayColsPorts[local_u8ColsCounter], local_u8ArrayColsPins[local_u8ColsCounter], DIO_PIN_LOW);
			
			//Read Each Row
			for(local_u8RowsCounter = 0;local_u8RowsCounter < 4; local_u8RowsCounter++)
			{
				DIO_voidGetPinValue(local_u8ArrayROWPorts[local_u8RowsCounter], local_u8ArrayROWPins[local_u8RowsCounter], &local_u8RowValue);
				
				//IF Pressed
				if(0 == local_u8RowValue)
				{
					while (0 == local_u8RowValue)
					{	
					   DIO_voidGetPinValue(local_u8ArrayROWPorts[local_u8RowsCounter], local_u8ArrayROWPins[local_u8RowsCounter], &local_u8RowValue);
					}
					
					*copy_pu8ReturnedValue = local_u8Keys[local_u8RowsCounter][local_u8ColsCounter];
					return;
				}
			}
			
			//Deactivate current Column
			DIO_voidSetPinValue(local_u8ArrayColsPorts[local_u8ColsCounter], local_u8ArrayColsPins[local_u8ColsCounter], DIO_PIN_HIGH);
		}
	}
	else
	{
		//retun Error State
	}
}