/*
 * EXTI_register.h
 *
 * Created: 2/18/2024 9:51:49 PM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef EXTI_REGISTER_H_
#define EXTI_REGISTER_H_


// MCU CONTROL REGISTER
#define  MCUCR_REG			(*(volatile u8*)0x55)
#define   ISC00             0
#define	  ISC01             1
#define	  ISC10             2
#define	  ISC11             3

// MCU CONTROL AND STATUS REGISTER
#define  MCUCSR_REG			(*(volatile u8*)0x54)
#define  ISC2               6

// GENERAL INTERRUPT CONTROL REGISTER
#define  GICR_REG			(*(volatile u8*)0x5B)
#define  INT2               5
#define	 INT0				6
#define	 INT1				7

// GENERAL INTERRUPT FLAG REGISTER
#define  GIFR_REG			(*(volatile u8*)0x5A)
#define  INTF2              5
#define  INTF0              6
#define  INTF1              7



#endif /* EXTI_REGISTER_H_ */