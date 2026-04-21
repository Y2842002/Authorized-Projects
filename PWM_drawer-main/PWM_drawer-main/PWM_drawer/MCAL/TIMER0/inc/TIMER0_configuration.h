/*
 * TIMER0_configuration.h
 *
 * Created: 2/29/2024 1:14:52 AM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef TIMER0_CONFIGURATION_H_
#define TIMER0_CONFIGURATION_H_


/* Options FOR TMR0 Mode:
1- TMR0_NORMAL_MODE
2- TMR0_CTC_MODE
*/
#define TMR0_MODE                           TMR0_CTC_MODE


#define TMR0_PRELOAD_VALUE					113
#define TMR0_OVER_FLOW_COUNTER      		977

#define TMR0_COMPARE_VALUE					249
#define TMR0_OUTPUT_COMPARE_COUNTER			1500


#endif /* TIMER0_CONFIGURATION_H_ */