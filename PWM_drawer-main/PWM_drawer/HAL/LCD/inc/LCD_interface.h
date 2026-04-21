/*
 * LCD_interface.h
 *
 * Created: 2/9/2024 10:50:31 PM
 *  Author: Yousef Osama
 */ 
#include "STD_TYPES.h"

#ifndef LCD_INTERFACE_H_
#define LCD_INTERFACE_H_


void LCD_voidInit                (void);
void LCD_voidSendCommand         (u8 copy_u8Cmnd);
void LCD_voidDisplayChar         (u8 copy_u8Data);
void LCD_voidDisplayString       (u8* copy_pu8String);



void LCD_voidDisplayNumber       (u32 copy_u32Number); 
void LCD_voidClear               (void); // cmnd
void LCD_voidShift               (u8 copy_u8ShifttingDirection); //cmnd
void LCD_voidGoToSpecificPosition(u8 copy_u8LineNumber, u8 copy_u8Position); //cmnd
void LCD_voidClearSpecificPosition(u8 copy_u8LineNumber, u8 copy_u8Position);
void LCD_voidDisplayPWMCalculations(u32 copy_u32Frequency,u32 copy_u32PeriodicTime,u32 copy_u32DutyCycle,u32 cop_u32Ton,u32 copu_u32Toff);
void LCD_voidDisplayPWMSignal(u32 copy_u32DutyCycle,u32 copy_u32Frequency,u32 copy_u32Ton,u32 copy_u32Toff);
#endif /* LCD_INTERFACE_H_ */