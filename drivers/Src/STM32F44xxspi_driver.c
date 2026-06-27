/**
 ******************************************************************************
 * @file           : stm32f446xx_spi_driver.c
 * @brief          : Concise, beginner-friendly SPI Driver Implementation
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#include "stm32f446xx_spi_driver.h"

/* Private helper functions for handling interrupts */
static void spi_txe_interrupt_handle(SPI_Handle_t* pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t* pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t* pSPIHandle);

/**
 * @brief  Helper function to check if a specific Status Register flag is set.
 * @param  pSPIx: Base address of the SPI instance.
 * @param  flagName: The status register mask bit to check (e.g., SPI_SR_TXE).
 * @retval FLAG_SET (1) if the bit is up, or FLAG_RESET (0) if down.
 */
uint8_t checkStatusFlag(SPI_RegDef_t* pSPIx, uint32_t flagName)
{
    if (pSPIx->SR & flagName) {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

/**
 * @brief  Turns the power (peripheral clock) on or off for an SPI channel.
 * @param  pSPIx: Base address of the SPI instance (SPI1, SPI2, etc.).
 * @param  enOrDi: ENABLE to turn power on, DISABLE to turn it off.
 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE) {
        if      (pSPIx == SPI1) SPI1_PCLK_EN();
        else if (pSPIx == SPI2) SPI2_PCLK_EN();
        else if (pSPIx == SPI3) SPI3_PCLK_EN();
        else if (pSPIx == SPI4) SPI4_PCLK_EN();
    } else {
        if      (pSPIx == SPI1) SPI1_PCLK_DIS();
        else if (pSPIx == SPI2) SPI2_PCLK_DIS();
        else if (pSPIx == SPI3) SPI3_PCLK_DIS();
        else if (pSPIx == SPI4) SPI4_PCLK_DIS();
    }
}

/**
 * @brief  Applies custom settings checklist to configure the SPI hardware peripheral.
 * @param  pSPIHandle: Pointer to the handle holding target settings.
 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
    uint32_t temp = 0;

    // 1. Turn on the operational peripheral clock
    SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

    // 2. Configure Device Mode (Master/Slave)
    if (pSPIHandle->SPIConfig.spiDeviceMode == SPI_DEVICE_MODE_MASTER) {
        temp |= (1 << CR_MSTR);
    } else {
        temp &= ~(1 << CR_MSTR);
    }

    // 3. Configure Bus Wiring Layout Architecture
    if (pSPIHandle->SPIConfig.spiBusConfig == SPI_BUS_CONFIG_FD) {
        temp &= ~(1 << CR_BIDIMODE);
    } else if (pSPIHandle->SPIConfig.spiBusConfig == SPI_BUS_CONFIG_HD) {
        temp |= (1 << CR_BIDIMODE);
    } else if (pSPIHandle->SPIConfig.spiBusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY) {
        temp &= ~(1 << CR_BIDIMODE);
        temp |= (1 << CR_RXONLY);
    } else if (pSPIHandle->SPIConfig.spiBusConfig == SPI_BUS_CONFIG_SIMPLEX_TXONLY) {
        temp |= (1 << CR_BIDIMODE);
        temp |= (1 << CR_BIDIOE);
    }

    // 4. Configure Data Frame Format (8-bit or 16-bit packets)
    if (pSPIHandle->SPIConfig.spiDFF == SPI_DFF_16BITS) {
        temp |= (1 << CR_DFF);
    } else {
        temp &= ~(1 << CR_DFF);
    }

    // 5. Configure Clock Phase (CPHA)
    if (pSPIHandle->SPIConfig.spiCPHA == SPI_CPHA_HIGH) {
        temp |= (1 << CR_CPHA);
    } else {
        temp &= ~(1 << CR_CPHA);
    }

    // 6. Configure Clock Polarity (CPOL)
    if (pSPIHandle->SPIConfig.spiCPOL == SPI_CPOL_HIGH) {
        temp |= (1 << CR_CPOL);
    } else {
        temp &= ~(1 << CR_CPOL);
    }

    // 7. Configure Slave Select Management (Hardware or Software controlled)
    if (pSPIHandle->SPIConfig.spiSSM == SPI_SSM_EN) {
        temp |= (1 << CR_SSM);
    } else {
        temp &= ~(1 << CR_SSM);
    }

    // 8. Configure Clock Speed Prescaler Divider
    temp |= (pSPIHandle->SPIConfig.spiSclkSpeed << CR_BR);

    // Write all combined custom register values to CR1
    pSPIHandle->pSPIx->CR1 = temp;
}

/**
 * @brief  Resets an SPI peripheral bank back to its factory default states.
 * @param  pSPIx: Base address of the target SPI instance.
 */
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
    if      (pSPIx == SPI1) RESET_SPI1_REG();
    else if (pSPIx == SPI2) RESET_SPI2_REG();
    else if (pSPIx == SPI3) RESET_SPI3_REG();
    else if (pSPIx == SPI4) RESET_SPI4_REG();
}

/**
 * @brief  Sends out a buffer array of data over the SPI line (Blocking Mode).
 * @param  pSPIx: Base address of the target SPI peripheral.
 * @param  pTxBuffer: Pointer to data array you want to send.
 * @param  len: Total number of bytes to send.
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len)
{
    while (len > 0) {
        // Wait until the internal Transmit Buffer is completely empty
        while (checkStatusFlag(pSPIx, SR_TXE) == FLAG_RESET);

        if (pSPIx->CR1 & (1 << CR_DFF)) {
            // 16-bit data frame mode
            pSPIx->DR = *((uint16_t*)pTxBuffer);
            pTxBuffer += 2;
            len -= 2;
        } else {
            // 8-bit data frame mode
            pSPIx->DR = *pTxBuffer;
            pTxBuffer++;
            len--;
        }
    }
}

/**
 * @brief  Gathers incoming data from the SPI line and saves it (Blocking Mode).
 * @param  pSPIx: Base address of the target SPI peripheral.
 * @param  pRxBuffer: Pointer to data array where received bits will go.
 * @param  len: Total number of bytes to expect.
 */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len)
{
    while (len > 0) {
        // Wait until the Receive Buffer has data ready to read
        while (checkStatusFlag(pSPIx, SR_RXNE) == FLAG_RESET);

        if (pSPIx->CR1 & (1 << CR_DFF)) {
            // 16-bit data frame mode
            *((uint16_t*)pRxBuffer) = (uint16_t)pSPIx->DR;
            pRxBuffer += 2;
            len -= 2;
        } else {
            // 8-bit data frame mode
            *pRxBuffer = (uint8_t)pSPIx->DR;
            pRxBuffer++;
            len--;
        }
    }
}

/**
 * @brief  Sets up data transmission buffers and activates interrupts (Non-blocking Mode).
 * @param  pSPIHandle: Pointer to handle wrapper organizing variables.
 * @retval Returns current execution state of transmission line.
 */
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t len)
{
    uint8_t state = pSPIHandle->txState;

    if (state != SPI_BUSY_IN_TX) {
        pSPIHandle->pTxBuffer = pTxBuffer;
        pSPIHandle->txLen = len;
        pSPIHandle->txState = SPI_BUSY_IN_TX;

        // Turn on the Transmit Buffer Empty Interrupt bit
        pSPIHandle->pSPIx->CR2 |= (1 << CR_TXEIE);
    }
    return state;
}

/**
 * @brief  Sets up data reception buffers and activates interrupts (Non-blocking Mode).
 * @param  pSPIHandle: Pointer to handle wrapper organizing variables.
 * @retval Returns current execution state of reception line.
 */
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t len)
{
    uint8_t state = pSPIHandle->rxState;

    if (state != SPI_BUSY_IN_RX) {
        pSPIHandle->pRxBuffer = pRxBuffer;
        pSPIHandle->rxLen = len;
        pSPIHandle->rxState = SPI_BUSY_IN_RX;

        // Turn on the Receive Buffer Not Empty Interrupt bit
        pSPIHandle->pSPIx->CR2 |= (1 << CR_RXNEIE);
    }
    return state;
}

/**
 * @brief  Flips the Software Internal Slave Select line high or low manually.
 * @param  pSPIx: Base address of the target SPI hardware.
 * @param  enOrDi: ENABLE to pull line High, DISABLE to ground it Low.
 */
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE) {
        pSPIx->CR1 |= (1 << CR_SSI);
    } else {
        pSPIx->CR1 &= ~(1 << CR_SSI);
    }
}

/**
 * @brief  Turns on the Slave Select Output Enable bit to let hardware drive NSS automatically.
 * @param  pSPIx: Base address of the target SPI hardware.
 * @param  enOrDi: ENABLE to open automatic master output driving, DISABLE to block it.
 */
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE) {
        pSPIx->CR2 |= (1 << CR_SSOE);
    } else {
        pSPIx->CR2 &= ~(1 << CR_SSOE);
    }
}

/**
 * @brief  Enables or Disables the main SPI Peripheral Gate block to allow data stream flow.
 * @param  pSPIx: Base address of the target SPI instance.
 * @param  enOrDi: ENABLE to turn the SPI peripheral on, DISABLE to safely shut it down.
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE) {
        pSPIx->CR1 |= (1 << CR_SPE);
    } else {
        // Wait until SPI completes any active, running communication exchanges before killing power
        while (checkStatusFlag(pSPIx, SR_BSY) == FLAG_SET);
        pSPIx->CR1 &= ~(1 << CR_SPE);
    }
}

/**
 * @brief  Enables or disables an SPI interrupt channel line within the central processor NVIC.
 * @param  irqNumber: Internal positional exception index coordinate.
 * @param  enOrDi: ENABLE to listen to this exception line, DISABLE to ignore it.
 */
void SPI_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi)
{
    if (enOrDi == ENABLE) {
        if      (irqNumber < 32) *NVIC_ISER0 |= (1 << irqNumber);
        else if (irqNumber < 64) *NVIC_ISER1 |= (1 << (irqNumber % 32));
        else if (irqNumber < 96) *NVIC_ISER2 |= (1 << (irqNumber % 64));
        else if (irqNumber < 128)*NVIC_ISER3 |= (1 << (irqNumber % 96));
    } else {
        if      (irqNumber < 32) *NVIC_ICER0 |= (1 << irqNumber);
        else if (irqNumber < 64) *NVIC_ICER1 |= (1 << (irqNumber % 32));
        else if (irqNumber < 96) *NVIC_ICER2 |= (1 << (irqNumber % 64));
        else if (irqNumber < 128)*NVIC_ICER3 |= (1 << (irqNumber % 96));
    }
}

/**
 * @brief  Sets a rank priority execution weight number for an SPI interrupt channel line.
 * @param  irqNumber: Internal positional exception index code being ranked.
 * @param  irqPriority: Rank assignment priority scale number (0 to 15).
 */
void SPI_IRQPriorityConfig(uint8_t irqNumber, uint8_t irqPriority)
{
    uint8_t iprx = irqNumber / 4;
    uint8_t iprSection = irqNumber % 4;
    uint8_t shiftNo = (iprSection * 8) + (8 - 4); // Top 4 bits of each priority byte are used

    *(NVIC_IPR0 + iprx) &= ~(0xF << shiftNo);     // Clear old ranking field
    *(NVIC_IPR0 + iprx) |= (irqPriority << shiftNo);
}

/**
 * @brief  The primary central interrupt routing handler called when an SPI event happens.
 * @param  pSPIHandle: Pointer to the active handle controlling operational tracking state variables.
 */
void SPI_IRQHandler(SPI_Handle_t *pSPIHandle)
{
    uint8_t flagState;
    uint8_t controlState;

    // 1. Check if Transmit Buffer Empty flag triggered and if its interrupt listener is open
    flagState = checkStatusFlag(pSPIHandle->pSPIx, SR_TXE);
    controlState = (pSPIHandle->pSPIx->CR2 & (1 << CR_TXEIE)) ? 1 : 0; // Fixed: Swapped bad logical '&&' to bitwise '&'
    if (flagState && controlState) {
        spi_txe_interrupt_handle(pSPIHandle);
    }

    // 2. Check if Receive Buffer Not Empty flag triggered and if its interrupt listener is open
    flagState = checkStatusFlag(pSPIHandle->pSPIx, SR_RXNE);
    controlState = (pSPIHandle->pSPIx->CR2 & (1 << CR_RXNEIE)) ? 1 : 0;
    if (flagState && controlState) {
        spi_rxne_interrupt_handle(pSPIHandle);
    }

    // 3. Check if an Overrun Error condition triggered and if its error handler listener is open
    flagState = checkStatusFlag(pSPIHandle->pSPIx, SR_OVR);
    controlState = (pSPIHandle->pSPIx->CR2 & (1 << CR_ERRIE)) ? 1 : 0;
    if (flagState && controlState) {
        spi_ovr_err_interrupt_handle(pSPIHandle);
    }
}

/* ============================================================================
 * Helper Private Function Implementations
 * ============================================================================ */

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if (pSPIHandle->pSPIx->CR1 & (1 << CR_DFF)) {
        pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
        pSPIHandle->pTxBuffer += 2;
        pSPIHandle->txLen -= 2; // Fixed: Decrement by 2 for 16-bit frame tracking
    } else {
        pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
        pSPIHandle->pTxBuffer++;
        pSPIHandle->txLen--;
    }

    if (pSPIHandle->txLen == 0) {
        SPI_CloseTransmission(pSPIHandle);
        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
    }
}

static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if (pSPIHandle->pSPIx->CR1 & (1 << CR_DFF)) {
        *((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->pRxBuffer += 2;
        pSPIHandle->rxLen -= 2;
    } else {
        *pSPIHandle->pRxBuffer = (uint8_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->pRxBuffer++;
        pSPIHandle->rxLen--;
    }

    if (pSPIHandle->rxLen == 0) {
        SPI_CloseReception(pSPIHandle);
        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
    }
}

static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    // Clear the overrun hardware flag by reading DR and SR sequentially
    if (pSPIHandle->txState != SPI_BUSY_IN_TX) {
        uint32_t temp = pSPIHandle->pSPIx->DR;
        temp = pSPIHandle->pSPIx->SR;
        (void)temp;
    }/*
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

    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1 << CR_TXEIE); // Close the interrupt gate line
    pSPIHandle->pTxBuffer = NULL;
    pSPIHandle->txLen = 0;
    pSPIHandle->txState = SPI_READY;
}

void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1 << CR_RXNEIE); // Close the interrupt gate line
    pSPIHandle->pRxBuffer = NULL;
    pSPIHandle->rxLen = 0;
    pSPIHandle->rxState = SPI_READY;
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
    uint32_t temp = pSPIx->DR;
    temp = pSPIx->SR;
    (void)temp;
}

__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t appEv)
{
    // Weak definition for custom main implementation hooks
}
