/*
 * stm32f446xx_i2c_driver.h
 *
 *  Created on: Oct 14, 2025
 *      Author: chikamso
 */

#ifndef STM32F446XX_I2C_DRIVER_H_
#define STM32F446XX_I2C_DRIVER_H_

#include "STM32F44xx__driver.h"

/*
 * Defining the configuration structure for the i2c peripherals
 */
typedef struct{
	uint32_t sclSpeed;
	uint8_t deviceAddress;
	uint8_t ackControl;
	uint8_t fmDutyCycle;
}I2C_config_t;

/*
 * Defining the handle structure for the I2C peripherals
 */
typedef struct{
	I2C_RegDef_t* pI2Cx;
	I2C_config_t i2cConfigure;
	uint8_t* pTxBuffer;			/*!<To store the app. Tx buffer address>*/
	uint8_t* pRxBuffer;			/*!<To store the app. Rx buffer address>*/
	uint32_t TxLen;				/*!<To store Tx len>*/
	uint32_t RxLen;				/*!<To store Rx len>*/
	uint8_t TxRxState;			/*!<To store communication state>*/
	uint8_t DevAddr;			/*!<To store slave/device address>*/
	uint32_t RxSize;			/*!<To store Rx size>*/
	uint8_t Sr;					/*!<To store repeated start value>*/
}I2C_handle_t;


void I2C_PeriClockControl(I2C_RegDef_t* pI2Cx,uint8_t EnorDi);

void I2C_Init(I2C_handle_t* handle);
void I2C_DeInit(I2C_RegDef_t* pI2Cx);

/*
 * Data Send and Receive
 */
void I2C_MasterSendData(I2C_handle_t* pI2CHandle, uint8_t *pTxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR);
void I2C_MasterReceiveData(I2C_handle_t* pI2CHandle, uint8_t *pRxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR);

void I2C_SlaveSendData(I2C_RegDef_t* pI2C, uint8_t data);
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t* pI2C);

void I2C_EnableInterrupts(I2C_handle_t *pI2CHandle, uint8_t EnDi);

uint8_t I2C_MasterSendDataIT(I2C_handle_t* pI2CHandle, uint8_t *pTxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR);
uint8_t I2C_MasterReceiveDataIT(I2C_handle_t* pI2CHandle, uint8_t *pRxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR);
/*
 * Peripheral control API
 */

void I2C_CloseSendData(I2C_handle_t* pI2CHandle);
void I2C_CloseReceiveData(I2C_handle_t* pI2CHandle);
void I2CEnable(I2C_RegDef_t* pI2Cx, uint8_t en_di);
uint8_t I2C_GetFlagStatus(I2C_RegDef_t* pI2Cx, uint32_t FlagName);

/*
 * Application callback
 */
void I2C_ApplicationEventCallback(I2C_handle_t* pI2CHandle, uint8_t AppEv);

void I2C_IRQConfig(uint8_t IRQNumber,uint8_t ENorDi);
void I2C_IRQPriorityConfig(uint8_t IRQNUmber, uint8_t IRQpriority);
void I2C_EV_IRQHandling(I2C_handle_t* pI2CHandle);
void I2C_ER_IRQHandling(I2C_handle_t* pI2CHandle);

#define I2C_SCL_SPEED_SM				100000
#define I2C_SCL_SPEED_FM4K				400000
#define I2C_SCL_SPEED_FM2K				200000

#define Fm_DC1				0		//t(low)/t(high) = 2
#define Fm_DC2				1		//t(low)/t(high) = 16/9

#define I2C_ACK_EN			1
#define I2C_ACK_DIS			0

#define I2C_PERI_EN			0
#define I2C_PERI_DIS		1

#define I2C_SR_ENABLE		1
#define I2C_SR_DISABLE		0

/*
 * I2C Application states
 */
#define I2C_READY			0
#define I2C_BUSY_IN_RX		1
#define I2C_BUSY_IN_TX		2

#define I2C_EV_TX_CMPLT		1
#define I2C_EV_RX_CMPLT		2
#define I2C_EV_DATA_REQ		3
#define I2C_EV_DATA_RCVD	4
#define I2C_EV_STOPF		5

#define I2C_ERROR_BERR  3
#define I2C_ERROR_ARLO  4
#define I2C_ERROR_AF    5
#define I2C_ERROR_OVR   6
#define I2C_ERROR_TIMEOUT 7


#endif /* STM32F446XX_I2C_DRIVER_H_ */
