/*
 * SWITCH_interface.h
 *
 * Created: 2/3/2024 9:05:00 PM
 *  Author: Yousef Osama
 */ 


#ifndef SWITCH_INTERFACE_H_
#define SWITCH_INTERFACE_H_



#define SWITCH_PRESSED			    1
#define SWITCH_NOT_PRESSED			2


#define SWITCH_FORWARD_CONNECTION			1
#define SWITCH_REVERSE_CONNECTION           2


                            /*************** APIS PROTO TYPES ***************/

void SWITCH_voidInit(u8 copy_u8SwitchPortId, u8 copy_u8SwitchPinId);
void SWITCH_voidGetState(u8 copy_u8SwitchPortId, u8 copy_u8SwitchPinId, u8 copy_u8SwitchConnectionType, u8* copy_pu8SwitchState);


#endif /* SWITCH_INTERFACE_H_ */