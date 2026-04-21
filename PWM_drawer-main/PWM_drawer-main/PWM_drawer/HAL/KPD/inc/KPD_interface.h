/*
 * KPD_interface.h
 *
 * Created: 2/16/2024 3:31:21 PM
 *  Author: Yousef Osama Mohamed
 */ 


#include "STD_TYPES.h"

#ifndef KPD_INTERFACE_H_
#define KPD_INTERFACE_H_

#define KPD_NOT_PRESSED      0xff


/************************************************************************/
/*                    API PROTO TYPES                                   */
/************************************************************************/

void KPD_voidInit    (void);
void KPD_voidGetValue(u8* copy_pu8ReturnedValue);


#endif /* KPD_INTERFACE_H_ */