/*
 * EEPROM_program.c
 *
 * Created: 3/26/2024 7:10:42 AM
 *  Author: Yousef Osama Mohamed
 */
#define F_CPU 16000000UL
#include <util/delay.h>

/* UTILES_LIB */
#include "STD_TYPES.h"
#include "BIT_MATH.h"

/* MCAL */
#include "TWI_interface.h"

/* HAL */
#include "EEPROM_interface.h"
#include "EEPROM_private.h"


void EEPROM_voidInit(void)
{
	//In fact We are Initializing the MC to interface with the EEPROM 
	TWI_voidInitMaster();
}

void EEPROM_voidWriteString(u16 copy_u16WordAddress, u8 * copy_pu8String)
{
	
	 if(copy_pu8String!=NULL)
	 {
		 u8 local_u8Counter=0;
		 while(copy_pu8String[local_u8Counter]!='\0')
		 {
			 EEPROM_voidWriteByte(copy_u16WordAddress,copy_pu8String[local_u8Counter]);
			 local_u8Counter++;
		 }
	 }
	 else
	 {
		 // return Error state
	 }
}
void EEPROM_voidWriteByte(u16 copy_u16WordAddress, u8 copy_u8Data)
{
	u8 local_u8DeviceAddress = (copy_u16WordAddress>>8)|EEPROM_FIXED_ADDRESS;
	
	// Send start condition
	TWI_voidSendStartCondition();
	
	// Send device address with write operation
	TWI_voidSendSlaveAddWithWrite(local_u8DeviceAddress);
	
	// Send word address
	TWI_voidTransmitMasterDataByte((u8)copy_u16WordAddress);
	
	// Send byte data
	TWI_voidTransmitMasterDataByte(copy_u8Data);
	
	// Send stop condition
	TWI_voidSendStopCondition();
	
	// Self-timed Write Cycle delay
	_delay_ms(5);
}


void EEPROM_voidReadByte(u16 copy_u16WordAddress, u8* copy_pu8Data)
{
	if(copy_pu8Data!=NULL)
	{
		u8 local_u8DeviceAddress = (copy_u16WordAddress>>8)|EEPROM_FIXED_ADDRESS;
		
		// Send start condition
		TWI_voidSendStartCondition();
		
		// Send device address with write operation
		TWI_voidSendSlaveAddWithWrite(local_u8DeviceAddress);
		
		// Send word address
		TWI_voidTransmitMasterDataByte((u8)copy_u16WordAddress);
		
		// Send repeated start condition in order to switch i2c operation
		TWI_voidSendRepeatedStartCondition();
		
		// Send slave address with read operation
		TWI_voidSendSlaveAddWithRead(local_u8DeviceAddress);
		
		// Read byte data and respond with not ACK
		TWI_voidReceiveMasterDataByteWithNack(copy_pu8Data);
		
		// Send stop condition
		TWI_voidSendStopCondition();
		
		// Self-timed Write Cycle delay
		_delay_ms(5);
	}
	else
	{
		// return Error State
	}
}