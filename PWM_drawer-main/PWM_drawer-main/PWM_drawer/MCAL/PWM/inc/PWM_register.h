/*
 * PWM_register.h
 *
 * Created: 3/1/2024 7:50:40 PM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef PWM_REGISTER_H_
#define PWM_REGISTER_H_

 /* TIMER0 REGISTERS*/
 
 // Timer/Counter Control Register
 #define TCCR0_REG			(*(volatile u8*)0x53)
 #define CS00                0
 #define CS01                1
 #define CS02                2
 #define WGM01               3
 #define COM00               4
 #define COM01               5
 #define WGM00               6
 #define FOC0                7

 // Timer/Counter Register
 #define TCNT0_REG           (*(volatile u8*)0x52)

 // Output Compare Register
 #define OCR0_REG            (*(volatile u8*)0x5C)

 // Timer/Counter Interrupt Mask Register
 #define TIMSK_REG           (*(volatile u8*)0x59)
 #define TOIE0               0
 #define OCIE0               1

 // Timer/Counter Interrupt Flag Register
 #define TIFR_REG            (*(volatile u8*)0x58)
 #define TOV0                0
 #define OCF0                1


 /* TIMER1 REGISTERS*/
 
 // Timer/Counter1 Control Register A
 #define TCCR1A_REG			(*(volatile u8*)0x4F)
 #define WGM10               0
 #define WGM11               1
 #define FOC1B               2
 #define FOC1A               3
 #define COM1B0              4
 #define COM1B1              5
 #define COM1A0              6
 #define COM1A1              7

 // Timer/Counter1 Control Register B
 #define TCCR1B_REG			(*(volatile u8*)0x4E)
 #define CS10                0
 #define CS11                1
 #define CS12                2
 #define WGM12               3
 #define WGM13               4
 #define ICES1               6
 #define ICNC1               7

 // Timer/Counter1
 #define TCNT1_u16_REG       (*(volatile u16*)0x4C)

 // Output Compare Register 1 A
 #define OCR1A_u16_REG       (*(volatile u16*)0x4A)

 // Output Compare Register 1 B
 #define OCR1B_u16_REG       (*(volatile u16*)0x48)

 // Input Capture Register 1
 #define ICR1_u16_REG        (*(volatile u16*)0x46)

 // Timer/Counter Interrupt Mask Register
 #define TIMSK_REG          (*(volatile u8*)0x59)
 #define TOIE1              2
 #define OCIE1B             3
 #define OCIE1A             4
 #define TICIE1             5

 // Timer/Counter Interrupt Flag Register
 #define TIFR_REG           (*(volatile u8*)0x58)
 #define TOV1               2
 #define OCF1B              3
 #define OCF1A              4
 #define ICF1               5


//TIMER2
#define TCCR2_REG			(*(volatile u8*)0x45)
#define  FOC2               7
#define  WGM20 				6
#define  COM21 				5
#define  COM20 				4
#define  WGM21 				3
#define  CS22 				2
#define  CS21 				1
#define  CS20               0

#define TCNT2_REG			(*(volatile u8*)0x44)

#define OCR2_REG			(*(volatile u8*)0x43)

#define ASSR_REG			(*(volatile u8*)0x42)
#define  AS2                3
#define  TCN2UB 			2
#define  OCR2UB 			1
#define  TCR2UB				0

#define TIMSK_REG			(*(volatile u8*)0x59)
#define OCIE2 7
#define TOIE2 6

#define TIFR_REG			(*(volatile u8*)0x58)
#define OCF2                7
#define TOV2                 6

#define SFIOR_REG			(*(volatile u8*)0x50)
#define PSR2 1


#endif /* PWM_REGISTER_H_ */