/**
 ******************************************************************************
 * @file           : stm32f446xx_usart_driver.c
 * @brief          : Refactored USART Driver implementation matching standard ecosystem API
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#include "stm32f446xx_usart_driver.h"

/******************************************************************************/
/* Peripheral Clock Control                                                    */
/******************************************************************************/

void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE)
    {
        if (pUSARTx == USART1)      { USART1_PCLK_EN(); }
        else if (pUSARTx == USART2) { USART2_PCLK_EN(); }
        else if (pUSARTx == USART3) { USART3_PCLK_EN(); }
        else if (pUSARTx == USART6) { USART6_PCLK_EN(); }
    }
    else
    {
        if (pUSARTx == USART1)      { USART1_PCLK_DIS(); }
        else if (pUSARTx == USART2) { USART2_PCLK_DIS(); }
        else if (pUSARTx == USART3) { USART3_PCLK_DIS(); }
        else if (pUSARTx == USART6) { USART6_PCLK_DIS(); }
    }
}

/******************************************************************************/
/* Peripheral Initialization & De-Initialization                              */
/******************************************************************************/

void USART_Init(USART_Handle_t *pUSARTHandle)
{
    uint32_t tempreg = 0;

    /* 1. Enable the Peripheral Clock Gateway */
    USART_PeriClockControl(pUSARTHandle->pUSARTx, ENABLE);

    /* 2. Configure TX / RX Hardware Subsystem Engines */
    if (pUSARTHandle->USART_Config.mode == USART_MODE_ONLY_RX)
    {
        tempreg |= (1 << USART_CR1_RE);
    }
    else if (pUSARTHandle->USART_Config.mode == USART_MODE_ONLY_TX)
    {
        tempreg |= (1 << USART_CR1_TE);
    }
    else if (pUSARTHandle->USART_Config.mode == USART_MODE_TXRX)
    {
        tempreg |= ((1 << USART_CR1_RE) | (1 << USART_CR1_TE));
    }

    /* 3. Configure Character Frame Word Length Widths */
    tempreg |= (pUSARTHandle->USART_Config.wordLength << USART_CR1_M);

    /* 4. Configure Parity Bit Validation Mechanics */
    if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_EN_EVEN)
    {
        tempreg |= (1 << USART_CR1_PCE);
        /* Hardware defaults to Even Parity options automatically when PCE is active */
    }
    else if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_EN_ODD)
    {
        tempreg |= (1 << USART_CR1_PCE);
        tempreg |= (1 << USART_CR1_PS);
    }

    /* Write out configuration values safely to Control Register 1 */
    pUSARTHandle->pUSARTx->CR1 = tempreg;

    /* 5. Configure Stop Bits Isolation Split Boundaries inside CR2 */
    tempreg = 0;
    tempreg |= (pUSARTHandle->USART_Config.stopBits << USART_CR2_STOP);
    pUSARTHandle->pUSARTx->CR2 = tempreg;

    /* 6. Configure Hardware Flow Control Options inside CR3 */
    tempreg = 0;
    if (pUSARTHandle->USART_Config.hwFlowControl == USART_HW_FLOW_CTRL_CTS)
    {
        tempreg |= (1 << USART_CR3_CTSE);
    }
    else if (pUSARTHandle->USART_Config.hwFlowControl == USART_HW_FLOW_CTRL_RTS)
    {
        tempreg |= (1 << USART_CR3_RTSE);
    }
    else if (pUSARTHandle->USART_Config.hwFlowControl == USART_HW_FLOW_CTRL_RTS_CTS)
    {
        tempreg |= ((1 << USART_CR3_CTSE) | (1 << USART_CR3_RTSE));
    }
    pUSARTHandle->pUSARTx->CR3 = tempreg;

    /* 7. Calculate and Set Clock Fractional Speed Steps inside BRR */
    USART_SetBaudRate(pUSARTHandle->pUSARTx, pUSARTHandle->USART_Config.baudRate);
}

void USART_DeInit(USART_RegDef_t *pUSARTx)
{
    if (pUSARTx == USART1)      { RESET_USART1_REG(); }
    else if (pUSARTx == USART2) { RESET_USART2_REG(); }
    else if (pUSARTx == USART3) { RESET_USART3_REG(); }
    else if (pUSARTx == USART6) { RESET_USART6_REG(); }
}

/******************************************************************************/
/* Data Send and Receive (Blocking / Synchronous)                             */
/******************************************************************************/

void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t len)
{
    uint16_t *pdata;

    for (uint32_t i = 0; i < len; i++)
    {
        /* Spin-wait until Transmit Data Register Empty (TXE) flag drops up */
        while (!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_TXE));

        if (pUSARTHandle->USART_Config.wordLength == USART_WORDLEN_9BITS)
        {
            pdata = (uint16_t*)pTxBuffer;
            pUSARTHandle->pUSARTx->DR = (*pdata & (uint16_t)0x01FF);

            if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_DISABLE)
            {
                /* 9 full bits are raw user payload data - requires 2-byte memory steps */
                pTxBuffer++;
                pTxBuffer++;
            }
            else
            {
                /* 8 bits are user data, the 9th bit is automatically assigned by hardware */
                pTxBuffer++;
            }
        }
        else
        {
            /* Standard 8-bit characters tracking sequence */
            pUSARTHandle->pUSARTx->DR = (*pTxBuffer & (uint8_t)0xFF);
            pTxBuffer++;
        }
    }

    /* Wait for Transmission Complete (TC) to assure physical wire lines settle */
    while (!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_TC));
}

void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        /* Spin-wait until Receive Data Register Not Empty (RXNE) flag goes high */
        while (!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_RXNE));

        if (pUSARTHandle->USART_Config.wordLength == USART_WORDLEN_9BITS)
        {
            if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_DISABLE)
            {
                /* Read full 9 bits of valid user payload data into local buffer pointer */
                *((uint16_t*)pRxBuffer) = (pUSARTHandle->pUSARTx->DR & (uint16_t)0x01FF);
                pRxBuffer++;
                pRxBuffer++;
            }
            else
            {
                /* Parity mask strips off the hardware bit wrapper, isolating raw byte payload */
                *pRxBuffer = (pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
                pRxBuffer++;
            }
        }
        else
        {
            if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_DISABLE)
            {
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
            }
            else
            {
                /* 7 bits are payload data, strip out the MSB parity bit */
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0x7F);
            }
            pRxBuffer++;
        }
    }
}

/******************************************************************************/
/* Data Send and Receive (Non-Blocking / Interrupt-Driven)                    */
/******************************************************************************/

uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t len)
{
    uint8_t txstate = pUSARTHandle->txBusyState;

    if (txstate != USART_BUSY_IN_TX)
    {
        pUSARTHandle->txLen = len;
        pUSARTHandle->pTxBuffer = pTxBuffer;
        pUSARTHandle->txBusyState = USART_BUSY_IN_TX;

        /* Enable Interrupt local generation mask gates for TXE and TC transitions */
        pUSARTHandle->pUSARTx->CR1 |= (1 << USART_CR1_TXEIE);
        pUSARTHandle->pUSARTx->CR1 |= (1 << USART_CR1_TCIE);
    }
    return txstate;
}

uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t len)
{
    uint8_t rxstate = pUSARTHandle->rxBusyState;

    if (rxstate != USART_BUSY_IN_RX)
    {
        pUSARTHandle->rxLen = len;
        pUSARTHandle->pRxBuffer = pRxBuffer;
        pUSARTHandle->rxBusyState = USART_BUSY_IN_RX;

        /* Enable RXNE register exception generation lines */
        pUSARTHandle->pUSARTx->CR1 |= (1 << USART_CR1_RXNEIE);
    }
    return rxstate;
}

void USART_CloseTransmission(USART_Handle_t *pUSARTHandle)
{
    /* Mask out hardware context triggers within control line flags */
    pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_TXEIE);
    pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_TCIE);

    pUSARTHandle->pTxBuffer = NULL;
    pUSARTHandle->txLen = 0;
    pUSARTHandle->txBusyState = USART_READY;
}

void USART_CloseReception(USART_Handle_t *pUSARTHandle)
{
    pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_RXNEIE);

    pUSARTHandle->pRxBuffer = NULL;
    pUSARTHandle->rxLen = 0;
    pUSARTHandle->rxBusyState = USART_READY;
}

/******************************************************************************/
/* Peripheral Control Utility Controls                                        */
/******************************************************************************/

void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE)
    {
        pUSARTx->CR1 |= (1 << USART_CR1_UE);
    }
    else
    {
        pUSARTx->CR1 &= ~(1 << USART_CR1_UE);
    }
}

uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t flagName)
{
    if (pUSARTx->SR & flagName)
    {
        return TRUE;
    }
    return FALSE;
}

void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t statusFlagName)
{
    pUSARTx->SR &= ~statusFlagName;
}

void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t baudRate)
{
    uint32_t PCLKx;
    uint32_t usartdiv;
    uint32_t M_part, F_part;
    uint32_t tempreg = 0;

    /* Identify internal peripheral bus architecture routing clock frequencies */
    if (pUSARTx == USART1 || pUSARTx == USART6)
    {
        PCLKx = RCC_GetPCLK2Value(); /* Hanging off high-speed APB2 Bus */
    }
    else
    {
        PCLKx = RCC_GetPCLK1Value(); /* Hanging off low-speed APB1 Bus */
    }

    /* Scaling calculations modified to protect floating metrics inside integer fields */
    if (pUSARTx->CR1 & (1 << USART_CR1_OVER8))
    {
        usartdiv = ((25 * PCLKx) / (2 * baudRate));
    }
    else
    {
        usartdiv = ((25 * PCLKx) / (4 * baudRate));
    }

    M_part = usartdiv / 100;
    tempreg |= (M_part << 4);

    F_part = (usartdiv - (M_part * 100));

    if (pUSARTx->CR1 & (1 << USART_CR1_OVER8))
    {
        F_part = (((F_part * 8) + 50) / 100) & (uint8_t)0x07;
    }
    else
    {
        F_part = (((F_part * 16) + 50) / 100) & (uint8_t)0x0F;
    }

    tempreg |= F_part;
    pUSARTx->BRR = tempreg;
}

/******************************************************************************/
/* IRQ Vector Management & NVIC Configurations                                */
/******************************************************************************/

void USART_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi)
{
    if (enOrDi == ENABLE)
    {
        if (irqNumber < 32)
        {
            *NVIC_ISER0 |= (1 << irqNumber);
        }
        else if (irqNumber >= 32 && irqNumber < 64)
        {
            *NVIC_ISER1 |= (1 << (irqNumber % 32));
        }
        else if (irqNumber >= 64 && irqNumber < 96)
        {
            *NVIC_ISER2 |= (1 << (irqNumber % 64));
        }
    }
    else
    {
        if (irqNumber < 32)
        {
            *NVIC_ICER0 |= (1 << irqNumber);
        }
        else if (irqNumber >= 32 && irqNumber < 64)
        {
            *NVIC_ICER1 |= (1 << (irqNumber % 32));
        }
        else if (irqNumber >= 64 && irqNumber < 96)
        {
            *NVIC_ICER2 |= (1 << (irqNumber % 64));
        }
    }
}

void USART_IRQPriorityConfig(uint8_t irqNumber, uint32_t irqPriority)
{
    uint8_t iprx = irqNumber / 4;
    uint8_t iprSection = irqNumber % 4;
    uint8_t shiftNo = (iprSection * 8) + 4; /* Lower 4 bits of each section are not implemented on STM32 MCU */
    *(NVIC_IPR0 + iprx) |= (irqPriority << shiftNo);
}

/******************************************************************************/
/* Core ISR Handler Logic                                                      */
/******************************************************************************/

void USART_IRQHandling(USART_Handle_t *pUSARTHandle)
{
    uint32_t temp1, temp2, temp3;
    uint16_t pdata;

    /************************* Check for TC flag ************************************/
    temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_TC;
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_TCIE);

    if (temp1 && temp2)
    {
        if (pUSARTHandle->txBusyState == USART_BUSY_IN_TX)
        {
            if (pUSARTHandle->txLen == 0)
            {
                /* Clear the interrupt controls and clean up active operational handle context */
                pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_TCIE);

                pUSARTHandle->txBusyState = USART_READY;
                pUSARTHandle->pTxBuffer = NULL;
                pUSARTHandle->txLen = 0;

                USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_TX_CMPLT);
            }
        }
    }

    /************************* Check for TXE flag ***********************************/
    temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_TXE;
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_TXEIE);

    if (temp1 && temp2)
    {
        if (pUSARTHandle->txBusyState == USART_BUSY_IN_TX)
        {
            if (pUSARTHandle->txLen > 0)
            {
                if (pUSARTHandle->USART_Config.wordLength == USART_WORDLEN_9BITS)
                {
                    pdata = *((uint16_t*)pUSARTHandle->pTxBuffer);
                    pUSARTHandle->pUSARTx->DR = (pdata & (uint16_t)0x01FF);

                    if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_DISABLE)
                    {
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->txLen -= 2;
                    }
                    else
                    {
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->txLen--;
                    }
                }
                else
                {
                    pUSARTHandle->pUSARTx->DR = (*pUSARTHandle->pTxBuffer & (uint8_t)0xFF);
                    pUSARTHandle->pTxBuffer++;
                    pUSARTHandle->txLen--;
                }
            }

            if (pUSARTHandle->txLen == 0)
            {
                /* Disable TXEIE, clear interrupt pathway, and let final byte settle inside TC step */
                pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_TXEIE);
            }
        }
    }

    /************************* Check for RXNE flag **********************************/
    temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_RXNE;
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_RXNEIE);

    if (temp1 && temp2)
    {
        if (pUSARTHandle->rxBusyState == USART_BUSY_IN_RX)
        {
            if (pUSARTHandle->rxLen > 0)
            {
                if (pUSARTHandle->USART_Config.wordLength == USART_WORDLEN_9BITS)
                {
                    if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_DISABLE)
                    {
                        *((uint16_t*)pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR & (uint16_t)0x01FF);
                        pUSARTHandle->pRxBuffer++;
                        pUSARTHandle->pRxBuffer++;
                        pUSARTHandle->rxLen -= 2;
                    }
                    else
                    {
                        *(pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
                        pUSARTHandle->pRxBuffer++;
                        pUSARTHandle->rxLen--;
                    }
                }
                else
                {
                    if (pUSARTHandle->USART_Config.parityControl == USART_PARITY_DISABLE)
                    {
                        *(pUSARTHandle->pRxBuffer) = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
                    }
                    else
                    {
                        *(pUSARTHandle->pRxBuffer) = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0x7F);
                    }
                    pUSARTHandle->pRxBuffer++;
                    pUSARTHandle->rxLen--;
                }
            }

            if (pUSARTHandle->rxLen == 0)
            {
                USART_CloseReception(pUSARTHandle);
                USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_RX_CMPLT);
            }
        }
    }

    /************************* Check for CTS flag ***********************************/
    temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_CTS;
    temp2 = pUSARTHandle->pUSARTx->CR3 & (1 << USART_CR3_CTSE);
    temp3 = pUSARTHandle->pUSARTx->CR3 & (1 << USART_CR3_CTSIE);

    if (temp1 && temp2 && temp3)
    {
        pUSARTHandle->pUSARTx->SR &= ~USART_SR_CTS;
        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_CTS);
    }

    /************************* Check for IDLE Line flag *****************************/
    temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_IDLE;
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_IDLEIE);

    if (temp1 && temp2)
    {
        /* Clear sequence for IDLE flag: Read SR followed by a read to DR register */
        uint32_t dummyRead = pUSARTHandle->pUSARTx->SR;
        dummyRead = pUSARTHandle->pUSARTx->DR;
        (void)dummyRead;

        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_IDLE);
    }

    /************************* Check for Overrun Error (ORE) flag *******************/
    temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_ORE;
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_RXNEIE);

    if (temp1 && temp2)
    {
        USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_ORE);
    }

    /************************* Check for Error Status (Multibuffer Mode) ************/
    temp2 = pUSARTHandle->pUSARTx->CR3 & (1 << USART_CR3_EIE);
    if (temp2)
    {
        temp1 = pUSARTHandle->pUSARTx->SR;
        if (temp1 & USART_SR_FE)
        {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_FE);
        }
        if (temp1 & USART_SR_NF)
        {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_NE);
        }
    }
}

/******************************************************************************/
/* Application Event Callback Template                                         */
/******************************************************************************/

__attribute__((weak)) void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t appEv)
{
    /* Overridable application layer callback hook */
}
