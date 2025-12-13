/*
 * STM32F44xxspi_driver.c
 *
 *  Created on: Jun 5, 2025
 *      Author: chika
 */

#include "STM32F44xxSPI__driver.h"
#include "STM32F44xx__driver.h"

static void spi_txe_interrupt_handle(SPI_Handle_t* pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t* pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t* pSPIHandle);

uint8_t checkStatusFlag(SPI_RegDef_t* pSPIx, uint32_t flagName){
	if(pSPIx->SR & flagName){
		return FLAG_SET;
	}
	return FLAG_RESET;
}

void SPI_PeriClockControl(SPI_RegDef_t* pSPIx,uint8_t EnorDi){
	if(EnorDi == ENABLE){
		if(pSPIx == SPI1){
			SPI1_PCLK_EN();
		}else if(pSPIx == SPI2){
			 SPI2_PCLK_EN();
		}else if(pSPIx == SPI3){
			SPI3_PCLK_EN();
		}else{
			SPI4_PCLK_EN();
		}
	}else{
		if(pSPIx == SPI1){
			SPI1_PCLK_DIS();
		}else if(pSPIx == SPI2){
			SPI2_PCLK_DIS();
		}else if(pSPIx == SPI3){
			SPI3_PCLK_DIS();
		}else{
			SPI4_PCLK_DIS();
		}
	}
}

void spiInit(SPI_Handle_t* handle){
	//Enable the clock
	SPI_PeriClockControl(handle->pSPIx, ENABLE);

	uint32_t value = 0;
	//Setting mode for peripheral; SLAVE OR MASTER
	if(handle->SPIConfig.SPI_DeviceMode == SLAVE){
		handle->pSPIx->CR1 &= ~(0x1 << CR_MSTR);
	}else if(handle->SPIConfig.SPI_DeviceMode == MASTER){
		value |= (0x1 << CR_MSTR);
		handle->pSPIx->CR1 |= value;
		value = 0;
	}
	//Bus configuration
	if(handle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD){
		value &= ~(0x1 << CR_BIDIMODE);
		handle->pSPIx->CR1 |= value;
		value = 0;
	}else if(handle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD){
		handle->pSPIx->CR1 |= (0x1 << CR_BIDIMODE);
	}else if(handle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY){
		handle->pSPIx->CR1 |= (0x1 << CR_RXONLY);
	}else if(handle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_TXONLY){
		handle->pSPIx->CR1 |= (0x1 << CR_BIDIMODE);
		handle->pSPIx->CR1 |= (0x1 << CR_BIDIOE);
	}
	//Setting the Data frame for data transmission
	if(handle->SPIConfig.SPI_DFF ==  DFF8){
		handle->pSPIx->CR1 &= ~(0x1 << CR_DFF);
	}else{
		handle->pSPIx->CR1 |= (0x1 << CR_DFF);
	}
	//Setting the clock phase
	if(handle->SPIConfig.SPI_CPHA == SPI_CPHA1){
		handle->pSPIx->CR1 &= ~(0x1 << CR_CPHA);
	}else{
		handle->pSPIx->CR1 |= (0x1 << CR_CPHA);
	}
	//Setting the clock polarity
	if(handle->SPIConfig.SPI_CPOL == SPI_CPOL0){
		handle->pSPIx->CR1 &= ~(0x1 << CR_CPOL);
	}else{
		handle->pSPIx->CR1 |= (0x1 << CR_CPOL);
	}
	//Setting software slave management
	if(handle->SPIConfig.SPI_SSM == SSM_HW){
		handle->pSPIx->CR1 &= ~(0x1 << CR_SSM);
	}else{
		handle->pSPIx->CR1 |= (0x1 << CR_SSM);
	}
	//Setting the speed
	value |= (handle->SPIConfig.SPI_SclkSpeed << CR_BR);
	handle->pSPIx->CR1 |= value;
}

void spiDeInit(SPI_RegDef_t* pSPIx){
	if(pSPIx == SPI1){
		RESET_SPI1_REG();
	}else if(pSPIx == SPI2){
		RESET_SPI2_REG();
	}else if(pSPIx == SPI3){
		RESET_SPI3_REG();
	}else{
		RESET_SPI4_REG();
	}
}

void SPI_SendData(SPI_RegDef_t* pSPIx, uint8_t *pTxBuffer, uint32_t Len){
	while(Len > 0){
		while(!checkStatusFlag(pSPIx,SR_TXE));
		if(pSPIx->CR1 & (0x1 << CR_DFF)){
			pSPIx->DR = *((uint16_t*)pTxBuffer);
			(uint16_t*)pTxBuffer++;
			Len--;
			Len--;
		}else{
			pSPIx->DR = *pTxBuffer;
			pTxBuffer++;
			Len--;
		}
	}
}
uint8_t SPI_SendDataIT(SPI_Handle_t* pSPIHandle, uint8_t *pTxBuffer, uint32_t Len){
	uint8_t state = pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX){
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;

		pSPIHandle->TxState = SPI_BUSY_IN_TX;
		pSPIHandle->pSPIx->CR2 |= (1 << CR_TXEIE); //Enable the interrupt for transfer buffer
	}
	return state;
}

void SPI_ReceiveData(SPI_RegDef_t* pSPIx, uint8_t *pRxBuffer, uint32_t Len){
	while(Len > 0){
		while(!checkStatusFlag(pSPIx,SR_RXNE));
		if(pSPIx->CR1 & (0x1 << CR_DFF)){
			*((uint16_t*)pRxBuffer) = pSPIx->DR;
			(uint16_t*)pRxBuffer++;
			Len--;
			Len--;
		}else{
			*pRxBuffer = pSPIx->DR;
			pRxBuffer++;
			Len--;
		}
	}
}
uint8_t SPI_ReceiveDataIT(SPI_Handle_t* pSPIHandle, uint8_t *pRxBuffer, uint32_t Len){
	uint8_t state = pSPIHandle->RxState;
	if(state != SPI_BUSY_IN_RX){
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;

		pSPIHandle->RxState = SPI_BUSY_IN_RX;
		pSPIHandle->pSPIx->CR2 |= (1 << CR_RXNEIE);
	}
	return state;
}
void setSSI(SPI_RegDef_t* pSPIx, uint8_t en_di){
	if(en_di == ENABLE){
		pSPIx->CR1 |= (0x1 << CR_SSI);
	}else
		pSPIx->CR1 &= ~(0x1 << CR_SSI);
}
void setSSOE(SPI_RegDef_t* pSPIx, uint8_t en_di){
	if(en_di == ENABLE){
		pSPIx->CR2 |= (0x1 << CR_SSOE);
	}else
		pSPIx->CR2 &= ~(0x1 << CR_SSOE);
}
void spiEnable(SPI_RegDef_t* pSPIx, uint8_t en_di){
	if(en_di == ENABLE){
		pSPIx->CR1 |= (0x1 << CR_SPE);
	}else{
		while(checkStatusFlag(pSPIx,SR_BSY));
		pSPIx->CR1 &= ~(0x1 << CR_SPE);
	}
}
void SPI_IRQConfig(uint8_t IRQNumber,uint8_t ENorDi){
	if(ENorDi == ENABLE){
		if(IRQNumber < 31){
			*NVIC_ISER0 |= (0x1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64){
			*NVIC_ISER1 |= (0x1 << (IRQNumber % 32));
		}else if(IRQNumber > 63 && IRQNumber < 96){
			*NVIC_ISER2 |= (0x1 << (IRQNumber % 64));
		}else if(IRQNumber > 95 && IRQNumber < 128){
			*NVIC_ISER3 |= (0x1 << (IRQNumber % 96));
		}
	}else{
		if(IRQNumber < 31){
			*NVIC_ICER0 |= (0x1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64){
			*NVIC_ICER1 |= (0x1 << (IRQNumber % 32));
		}else if(IRQNumber > 63 && IRQNumber < 96){
			*NVIC_ICER2 |= (0x1 << (IRQNumber % 64));
		}else if(IRQNumber > 95 && IRQNumber < 128){
			*NVIC_ICER3 |= (0x1 << (IRQNumber % 96));
		}
	}
}
void SPI_IRQPriorityConfig(uint8_t IRQNUmber, uint8_t IRQpriority){
	uint8_t iprx = IRQNUmber / 4;
	uint8_t iprSection = IRQNUmber % 4;
	uint8_t shiftNo = iprSection*8 + 4;
	*(NVIC_IPR0 + iprx) |= IRQpriority << shiftNo;
}

void SPI_IRQHandler(SPI_Handle_t* pSPIHandle){
	uint8_t temp1, temp2;
	//first let check for TXE
	temp1 = checkStatusFlag(pSPIHandle->pSPIx,SR_TXE);
	temp2 = pSPIHandle->pSPIx->CR2 && (1 << CR_TXEIE);

	if(temp1 && temp2){
		spi_txe_interrupt_handle(pSPIHandle);
	}

	//Check for RXNE
	temp1 = checkStatusFlag(pSPIHandle->pSPIx,SR_RXNE);
	temp2 = pSPIHandle->pSPIx->CR2 && (1 << CR_RXNEIE);

	if(temp1 && temp2){
		spi_rxne_interrupt_handle(pSPIHandle);
	}

	//check for ovr error
	temp1 = checkStatusFlag(pSPIHandle->pSPIx,SR_OVR);
	temp2 = pSPIHandle->pSPIx->CR2 && (1 << CR_ERRIE);

	if(temp1 && temp2){
		spi_ovr_err_interrupt_handle(pSPIHandle);
	}
}

//Some helper function implementations

static void spi_txe_interrupt_handle(SPI_Handle_t* pSPIHandle){
	if(pSPIHandle->pSPIx->CR1 & (0x1 << CR_DFF)){
		pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		(uint16_t*)pSPIHandle->pTxBuffer++;
		pSPIHandle->TxLen--;
	}else{
		pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
		pSPIHandle->pTxBuffer++;
		pSPIHandle->TxLen--;
	}
	if(!pSPIHandle->TxLen){
		SPI_CloseTransmission(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_TX_CMPLT);
	}
}
static void spi_rxne_interrupt_handle(SPI_Handle_t* pSPIHandle){
	if(pSPIHandle->pSPIx->CR1 & (0x1 << CR_DFF)){
		*((uint16_t*)pSPIHandle->pRxBuffer) = pSPIHandle->pSPIx->DR;
		(uint16_t*)pSPIHandle->pRxBuffer++;
		pSPIHandle->RxLen--;
		pSPIHandle->RxLen--;
	}else{
		*pSPIHandle->pRxBuffer = pSPIHandle->pSPIx->DR;
		pSPIHandle->pRxBuffer++;
		pSPIHandle->RxLen--;
	}
	if(!pSPIHandle->RxLen){
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
	}
}
static void spi_ovr_err_interrupt_handle(SPI_Handle_t* pSPIHandle){
	//1.Clear the ovr flag
	//2. Inform the application
	uint8_t temp;
	if(pSPIHandle->TxState != SPI_BUSY_IN_RX){
		temp = pSPIHandle->pSPIx->DR;
		temp = pSPIHandle->pSPIx->SR;
	}
	(void)temp;

	SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_OVRERR);
}

void SPI_CloseTransmission(SPI_Handle_t* pSPIHandle){
	pSPIHandle->pSPIx->CR2 &= ~(0x1 << CR_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;
}
void SPI_CloseReception(SPI_Handle_t* pSPIHandle){
	pSPIHandle->pSPIx->CR2 &= ~(0x1 << CR_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;
}
void I2C_ClearOVRFlag(SPI_RegDef_t* pSPIx){
	uint8_t temp;
	temp = pSPIx->DR;
	temp = pSPIx->SR;
	(void)temp;
}
__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t* pSPIHandle, uint8_t AppEv){

}
