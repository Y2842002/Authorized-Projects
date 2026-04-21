/*
 * PWM_interface.h
 *
 * Created: 3/1/2024 7:50:57 PM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef PWM_INTERFACE_H_
#define PWM_INTERFACE_H_

void PWM_voidInitChannel_0                (void);
void PWM_voidGenerateChannel_PWM_Channel_0(u8 copy_u8DutyCycle);

void PWM_voidInitChannel_1A               (void);
void PWM_voidGenerate_PWM_Channel_1A      (u16 copy_u16Frequency_Hz ,f32 copy_f32DutyCycle);

void PWM_voidGenerateChannel_PWM_Channel_2(u8 copy_u8DutyCycle);
void PWM_voidInitChannel_2(void);

void PWM_voidDutyCycleCalculations(u32* copy_pu32DutyCycle,u32 copy_pu32onTicks,u32 copy_pu32onCounter,u32 copy_pu32totalTicks,u32 copy_pu32totalCounter);
void PWM_voidOffTimeDuration(u32 copy_pu32PeriodicTime,u32 copy_pu32Ton,u32* copy_pu32TOff);
void PWM_voidOnTimeDuration(u32 copy_pu32PeriodicTime,u32 copy_pu32DutyCycle,u32* copy_pu32Ton);
void PWM_voidPeriodicTimeCalculations(u32 copy_pu32Frequency,u32* copy_pu32PeriodicTime);
void PWM_voidFrequencyCalculation(u32* copy_pu32Frequencyu ,u32 copy_pu32totalTicks,u32 copy_pu32totalCounter);



#endif /* PWM_INTERFACE_H_ */