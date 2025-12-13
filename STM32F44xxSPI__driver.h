/*
 * STM32F44xxSPI__driver.h
 *
 *  Created on: Jun 5, 2025
 *      Author: chika
 */

#ifndef STM32F44XXSPI__DRIVER_H_
#define STM32F44XXSPI__DRIVER_H_
#include "STM32F44xx__driver.h"

/*
 * Configuration structure for SPIx peripherals
 */
typedef struct{
	uint8_t SPI_DeviceMode;
	uint8_t SPI_BusConfig;
	uint8_t SPI_SclkSpeed;
	uint8_t SPI_DFF;
	uint8_t SPI_CPOL;
	uint8_t SPI_CPHA;
	uint8_t SPI_SSM;
}SPI_Config_t;

/*
 * Handle structure for SPIx peripherals
 */
typedef struct{
	SPI_RegDef_t *pSPIx; /*!<Holds the base address of SPIx(x:0,1,2..) peripherals*/
	SPI_Config_t SPIConfig;
	uint8_t *pTxBuffer; /*!<To store the app. Tx buffer address>*/
	uint8_t *pRxBuffer; /*!<To store the app. Rx buffer addres>*/
	uint32_t TxLen; /*!<To store Tx len>*/
	uint32_t RxLen; /*!<To store Rx len>*/
	uint8_t TxState; /*!<To store Tx state>*/
	uint8_t RxState; /*!<To store Rx state>*/
}SPI_Handle_t;

void SPI_PeriClockControl(SPI_RegDef_t* pSPIx,uint8_t EnorDi);

void spiInit(SPI_Handle_t* handle);
void spiDeInit(SPI_RegDef_t* pSPIx);

void SPI_SendData(SPI_RegDef_t* pSPIx, uint8_t *pTxBuffer, uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t* pSPIx, uint8_t *pRxBuffer, uint32_t Len);
uint8_t SPI_SendDataIT(SPI_Handle_t* pSPIHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t* pSPIHandle, uint8_t *pRxBuffer, uint32_t Len);

/*
 * Peripheral control API
 */
void I2C_ClearOVRFlag(SPI_RegDef_t* pSPIx);
void SPI_CloseTransmission(SPI_Handle_t* pSPIHandle);
void SPI_CloseReception(SPI_Handle_t* pSPIHandle);
void setSSI(SPI_RegDef_t* pSPIx, uint8_t en_di);
void setSSOE(SPI_RegDef_t* pSPIx, uint8_t en_di);
void spiEnable(SPI_RegDef_t* pSPIx, uint8_t en_di);

/*
 * Application callback
 */
void SPI_ApplicationEventCallback(SPI_Handle_t* pSPIHandle, uint8_t AppEv);

void SPI_IRQConfig(uint8_t IRQNumber,uint8_t ENorDi);
void SPI_IRQHandling(SPI_Handle_t* pSPIHandle);
void SPI_IRQPriorityConfig(uint8_t IRQNUmber, uint8_t IRQpriority);

#define SLAVE 							0
#define MASTER 							1

/*
 * @SPI bus configuration
 */
#define SPI_BUS_CONFIG_FD				1
#define SPI_BUS_CONFIG_HD				2
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY	3
#define SPI_BUS_CONFIG_SIMPLEX_TXONLY	4

/*
 * @SPI clock speed
 */
#define SPI_CLK_SPEED_DIV2				0
#define SPI_CLK_SPEED_DIV4				1
#define SPI_CLK_SPEED_DIV8				2
#define SPI_CLK_SPEED_DIV16				3
#define SPI_CLK_SPEED_DIV32				4
#define SPI_CLK_SPEED_DIV64				5
#define SPI_CLK_SPEED_DIV128			6
#define SPI_CLK_SPEED_DIV256			7

/*
 * @Data frame format
 */
#define DFF8							0
#define DFF16							1

/*
 * @Clock phase
 */
#define SPI_CPHA1						0
#define SPI_CPHA2						1

/*
 * SPI state
 */
#define SPI_READY 						0
#define SPI_BUSY_IN_RX					1
#define SPI_BUSY_IN_TX					2

/*
 * Possible SPI application events
 */
#define SPI_EVENT_TX_CMPLT				1
#define SPI_EVENT_RX_CMPLT				2
#define SPI_EVENT_OVRERR				3

/*
 * @Clock polarity
 */
#define SPI_CPOL0						0
#define SPI_CPOL1						1

/*
 * @Software slave management
 */
#define SSM_HW 							0
#define SSM_SW							1
#endif /* STM32F44XXSPI__DRIVER_H_ */
