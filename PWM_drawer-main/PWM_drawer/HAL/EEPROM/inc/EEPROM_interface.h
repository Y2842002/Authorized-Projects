/*
 * EEPROM_interface.h
 *
 * Created: 3/26/2024 7:10:05 AM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef EEPROM_INTERFACE_H_
#define EEPROM_INTERFACE_H_

void EEPROM_voidInit     (void);
void EEPROM_voidWriteByte(u16 copy_u16WordAddress, u8  copy_u8Data);
void EEPROM_voidReadByte (u16 copy_u16WordAddress, u8* copy_pu8Data);


#endif /* EEPROM_INTERFACE_H_ */