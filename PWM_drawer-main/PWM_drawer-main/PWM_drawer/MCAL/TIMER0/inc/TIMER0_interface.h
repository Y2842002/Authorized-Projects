/*
 * TIMER0_interface.h
 *
 * Created: 2/29/2024 1:15:13 AM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef TIMER0_INTERFACE_H_
#define TIMER0_INTERFACE_H_

                   /*************** APIS PROTOTYPES ***************/
                   
void TMR0_voidInit                (void);
void TMR0_voidStart               (void);
void TMR0_voidStop                (void);
void TMR0_voidSetCallBackOVF      (void(*copy_pFunAction)(void));
void TMR0_voidSetCallBackCTC      (void(*copy_pFunAction)(void));
void TMR0_voidSetDelay_ms_usingCTC(u16 copy_u16Delay_ms);



#endif /* TIMER0_INTERFACE_H_ */