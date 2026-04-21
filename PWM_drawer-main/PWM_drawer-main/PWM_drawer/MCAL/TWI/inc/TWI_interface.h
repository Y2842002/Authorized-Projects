/*
 * TWS_interface.h
 *
 * Created: 3/15/2024 11:17:34 PM
 *  Author: Yousef Osama Mohamed
 */ 


#ifndef TWS_INTERFACE_H_
#define TWS_INTERFACE_H_

//Master APIS
void TWI_voidInitMaster                   (void);
void TWI_voidSendStartCondition           (void);
void TWI_voidSendRepeatedStartCondition   (void);
void TWI_voidSendStopCondition            (void);
void TWI_voidSendSlaveAddWithWrite        (u8  copy_u8SlaveAdd);
void TWI_voidSendSlaveAddWithRead         (u8  copy_u8SlaveAdd);
void TWI_voidTransmitMasterDataByte       (u8  copy_u8TxData);
void TWI_voidReceiveMasterDataByteWithAck (u8* copy_pu8RxData);
void TWI_voidReceiveMasterDataByteWithNack(u8* copy_pu8RxData);



#endif /* TWS_INTERFACE_H_ */