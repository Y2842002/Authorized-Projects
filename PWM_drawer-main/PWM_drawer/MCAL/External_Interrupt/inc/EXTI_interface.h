/*
 * EXTI_interface.h
 *
 * Created: 2/18/2024 9:51:35 PM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef EXTI_INTERFACE_H_
#define EXTI_INTERFACE_H_

#define EXTI_INT0               0
#define EXTI_INT1               1
#define EXTI_INT2               2

#define EXTI_RISING_EDGE        0
#define	EXTI_FALLING_EDGE       1
#define	EXTI_LOW_LEVEL          2
#define	EXTI_ANY_LOGICAL_CHANGE 3


void EXTI_voidInit   (u8 copy_u8InterruptSource, u8 copy_u8TriggerEdge);
void EXTI_voidDisable(u8 copy_u8InterruptSource);




#endif /* EXTI_INTERFACE_H_ */