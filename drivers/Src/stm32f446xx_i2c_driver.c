/**
 ******************************************************************************
 * @file           : stm32f446xx_i2c_driver.c
 * @brief          : Concise, beginner-friendly I2C Driver Implementation
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#include "stm32f446xx_i2c_driver.h"

/* Private helper functions for managing the I2C protocol hardware phases */
static void I2C_GenerateStartCondition(I2C_RegDef_t* pI2Cx);
static void I2C_GenerateStopCondition(I2C_RegDef_t* pI2Cx);
static uint8_t I2C_checkStatusFlag(I2C_RegDef_t* pI2Cx, uint32_t flagName);
static void I2C_ExecuteAddressPhase(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);
static void I2C_ManageAcking(I2C_RegDef_t* pI2Cx, uint8_t state);

/**
 * @brief  Helper function to check if an internal I2C Status flag is set.
 */
static uint8_t I2C_checkStatusFlag(I2C_RegDef_t* pI2Cx, uint32_t flagName)
{
    if (pI2Cx->SR1 & flagName) {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

static void I2C_GenerateStartCondition(I2C_RegDef_t* pI2Cx)
{
    pI2Cx->CR1 |= (1 << CR1_START);
}

static void I2C_GenerateStopCondition(I2C_RegDef_t* pI2Cx)
{
    pI2Cx->CR1 |= (1 << CR1_STOP);
}

static void I2C_ExecuteAddressPhase(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr)
{
    slaveAddr = slaveAddr << 1;
    slaveAddr &= ~(1); // Clear bit 0 for Write operation
    pI2Cx->DR = slaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr)
{
    slaveAddr = slaveAddr << 1;
    slaveAddr |= 1;    // Set bit 0 for Read operation
    pI2Cx->DR = slaveAddr;
}

static void I2C_ManageAcking(I2C_RegDef_t* pI2Cx, uint8_t state)
{
    if (state == ENABLE) {
        pI2Cx->CR1 |= (1 << CR1_ACK);
    } else {
        pI2Cx->CR1 &= ~(1 << CR1_ACK);
    }
}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
    uint8_t dummy_read;
    if (pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL) {
        // Master Mode
        if (pI2CHandle->txRxState == I2C_BUSY_IN_RX) {
            if (pI2CHandle->rxSize == 1) {
                // For a 1-byte reception, ACK must be dropped before clearing ADDR
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
            }
        }
    }
    // Standard hardware clear sequence: Read SR1 followed by reading SR2
    dummy_read = pI2CHandle->pI2Cx->SR1;
    dummy_read = pI2CHandle->pI2Cx->SR2;
    (void)dummy_read;
}

/**
 * @brief  Turns peripheral clock power on or off for an I2C channel instance.
 */
void I2C_PeriClockControl(I2C_RegDef_t* pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == ENABLE) {
        if      (pI2Cx == I2C1) I2C1_PCLK_EN();
        else if (pI2Cx == I2C2) I2C2_PCLK_EN();
        else if (pI2Cx == I2C3) I2C3_PCLK_EN();
    } else {
        if      (pI2Cx == I2C1) I2C1_PCLK_DIS();
        else if (pI2Cx == I2C2) I2C2_PCLK_DIS();
        else if (pI2Cx == I2C3) I2C3_PCLK_DIS();
    }
}

/**
 * @brief  Configures speed metrics, rise times, ownership addresses, and enables the I2C block.
 */
void I2C_Init(I2C_Handle_t* pI2CHandle)
{
    uint32_t tempreg = 0;

    // 1. Turn on peripheral clock power
    I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

    // 2. Configure the FREQ field inside CR2 (matches internal PCLK1 clock speed)
    tempreg = RCC_GetPCLK1Value() / 1000000;
    pI2CHandle->pI2Cx->CR2 |= (tempreg & 0x3F);

    // 3. Program the target device's custom own address (for slave mode operational responses)
    tempreg = 0;
    tempreg = pI2CHandle->I2C_Config.deviceAddress << 1;
    tempreg |= (1 << 14); // Hardware requirement bit state
    pI2CHandle->pI2Cx->OAR1 |= tempreg;

    // 4. CCR Timing Clock Speed configurations
    uint16_t ccr_value = 0;
    tempreg = 0;
    if (pI2CHandle->I2C_Config.sclSpeed <= I2C_SCL_SPEED_SM) {
        // Standard Mode configuration timing formula
        ccr_value = (RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.sclSpeed));
        tempreg |= (ccr_value & 0xFFF);
    } else {
        // Fast Mode configuration timing formula
        tempreg |= (1 << 15); // Turn on Fast Mode bit selector
        tempreg |= (pI2CHandle->I2C_Config.fmDutyCycle << 14);
        if (pI2CHandle->I2C_Config.fmDutyCycle == I2C_FM_DUTY_2) {
            ccr_value = (RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.sclSpeed));
        } else {
            ccr_value = (RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.sclSpeed));
        }
        tempreg |= (ccr_value & 0xFFF);
    }
    pI2CHandle->pI2Cx->CCR = tempreg;

    // 5. TRISE Maximum Rise Time configuration boundaries
    tempreg = 0;
    if (pI2CHandle->I2C_Config.sclSpeed <= I2C_SCL_SPEED_SM) {
        tempreg |= (RCC_GetPCLK1Value() / 1000000U) + 1;
    } else {
        tempreg |= ((RCC_GetPCLK1Value() * 300) / 1000000000) + 1;
    }
    pI2CHandle->pI2Cx->TRISE = (tempreg & 0x3F);

    // 6. Enable the peripheral gate block
    pI2CHandle->pI2Cx->CR1 |= (1 << CR1_PE);

    // 7. Apply the customized user choice ACK control response rule (Fixed bug)
    I2 ManageAcking(pI2CHandle->pI2Cx, pI2CHandle->I2C_Config.ackControl);
}

/**
 * @brief  Sends data stream blocks to an external destination node (Blocking Mode).
 */
void I2C_MasterSendData(I2C_Handle_t* pI2CHandle, uint8_t *pTxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr)
{
    // Start communication frame setup
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    // Wait until start sequence completes (SB flag gets hoisted)
    while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_SB));

    // Ship the target address coordinates out
    I2C_ExecuteAddressPhase(pI2CHandle->pI2Cx, slaveAddr);

    // Wait for validation that the destination node matched and claimed the request
    while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_ADDR)) {
        if (I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_AF)) {
            // Error catch: Destination missing/rejected (NACK failure)
            pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_AF;
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
            return;
        }
    }

    I2C_ClearADDRFlag(pI2CHandle);

    // Stream the array elements down the pipeline buffer sequential channels
    while (len > 0) {
        while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_TXE)); // Wait for empty internal buffer slot
        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        len--;
    }

    // Secure execution closures before letting go
    while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_TXE));
    while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_BTF));

    if (sr == I2C_SR_DISABLE) {
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
    }
}

/**
 * @brief  Requests and reads an array block from an external slave device (Blocking Mode).
 */
void I2C_MasterReceiveData(I2C_Handle_t* pI2CHandle, uint8_t *pRxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr)
{
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
    while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_SB));

    I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, slaveAddr);

    while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_ADDR)) {
        if (I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_AF)) {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_AF;
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
            return;
        }
    }

    if (len == 1) {
        I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
        I2C_ClearADDRFlag(pI2CHandle);

        while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_RXNE));
        if (sr == I2C_SR_DISABLE) {
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
        }

        *pRxBuffer = pI2CHandle->pI2Cx->DR;
        return;
    }

    if (len > 1) {
        I2C_ClearADDRFlag(pI2CHandle);
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);

        for (uint32_t i = len; i > 0; i--) {
            while (!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_RXNE));

            if (i == 2) {
                // Drop ACK generation before accepting the absolute final byte packet
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
            }
            if (i == 1) {
                if (sr == I2C_SR_DISABLE) {
                    I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                }
            }
            *pRxBuffer = pI2CHandle->pI2Cx->DR;
            pRxBuffer++;
        }
    }

    // Restore standard default state tracking
    if (pI2CHandle->I2C_Config.ackControl == I2C_ACK_EN) {
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
    }
}

/**
 * @brief  Sets up non-blocking interrupt parameters to transmit data arrays (Interrupt Mode).
 */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t* pI2CHandle, uint8_t *pTxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr)
{
    uint8_t busystate = pI2CHandle->txRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX)) {
        pI2CHandle->pTxBuffer = pTxBuffer;
        pI2CHandle->txLen = len;
        pI2CHandle->txRxState = I2C_BUSY_IN_TX;
        pI2CHandle->devAddr = slaveAddr;
        pI2CHandle->sr = sr;

        I2C_EnableInterrupts(pI2CHandle, ENABLE);
        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
    }
    return busystate;
}

/**
 * @brief  Sets up non-blocking interrupt parameters to receive data arrays (Interrupt Mode).
 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t* pI2CHandle, uint8_t *pRxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr)
{
    uint8_t busystate = pI2CHandle->txRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX)) {
        pI2CHandle->pRxBuffer = pRxBuffer;
        pI2CHandle->rxLen = len;
        pI2CHandle->txRxState = I2C_BUSY_IN_RX;
        pI2CHandle->rxSize = len;
        pI2CHandle->devAddr = slaveAddr;
        pI2CHandle->sr = sr;

        I2C_EnableInterrupts(pI2CHandle, ENABLE);
        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
    }
    return busystate;
}

void I2C_EnableInterrupts(I2C_Handle_t *pI2CHandle, uint8_t EnDi)
{
    if (EnDi == ENABLE) {
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    } else {
        pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
        pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);
    }
}

void I2C_CloseSendData(I2C_Handle_t* pI2CHandle)
{
    I2C_EnableInterrupts(pI2CHandle, DISABLE);
    // Fixed: Peripheral Control PE bit stays enabled here now to prevent driver death
    pI2CHandle->txRxState = I2C_READY;
    pI2CHandle->pTxBuffer = NULL;
    pI2CHandle->txLen = 0;

    if (pI2CHandle->I2C_Config.ackControl == I2C_ACK_EN) {
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
    }
}

void I2C_CloseReceiveData(I2C_Handle_t* pI2CHandle)
{
    I2C_EnableInterrupts(pI2CHandle, DISABLE);
    pI2CHandle->txRxState = I2C_READY;
    pI2CHandle->pRxBuffer = NULL;
    pI2CHandle->rxLen = 0;
    pI2CHandle->rxSize = 0;

    if (pI2CHandle->I2C_Config.ackControl == I2C_ACK_EN) {
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
    }
}

void I2C_SlaveSendData(I2C_RegDef_t* pI2C, uint8_t data)
{
    pI2C->DR = data;
}

uint8_t I2C_SlaveReceiveData(I2C_RegDef_t* pI2C)
{
    return pI2C->DR;
}

void I2C_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi)
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

void I2C_IRQPriorityConfig(uint8_t irqNumber, uint8_t irqPriority)
{
    uint8_t iprx = irqNumber / 4;
    uint8_t iprSection = irqNumber % 4;
    uint8_t shiftNo = (iprSection * 8) + 4;

    *(NVIC_IPR0 + iprx) &= ~(0xF << shiftNo); // Fixed bug: Clean old bit fields first
    *(NVIC_IPR0 + iprx) |= (irqPriority << shiftNo);
}

/**
 * @brief  Core execution routing system processing standard operational event callbacks.
 */
void I2C_EV_IRQHandling(I2C_Handle_t* pI2CHandle)
{
    uint32_t temp1, temp3, temp_bufen;
    temp1 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);
    temp_bufen = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);

    // 1. Handle Start Bit Generation Event (SB Flag) - Only active in Master mode
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_SR1_SB;
    if (temp1 && temp3) {
        if (pI2CHandle->txRxState == I2C_BUSY_IN_RX) {
            I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->devAddr);
        } else if (pI2CHandle->txRxState == I2C_BUSY_IN_TX) {
            I2C_ExecuteAddressPhase(pI2CHandle->pI2Cx, pI2CHandle->devAddr);
        }
    }

    // 2. Handle Address Phase Sent/Matched Event (ADDR Flag)
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_SR1_ADDR;
    if (temp1 && temp3) {
        I2C_ClearADDRFlag(pI2CHandle);
        if ((pI2CHandle->txRxState == I2C_BUSY_IN_RX) && (pI2CHandle->rxSize > 1)) {
            I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
        }
    }

    // 3. Handle Byte Transfer Finished Event (BTF Flag)
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_SR1_BTF;
    if (temp1 && temp3) {
        if (pI2CHandle->txRxState == I2C_BUSY_IN_TX) {
            if (pI2CHandle->pI2Cx->SR1 & I2C_SR1_TXE) {
                if (pI2CHandle->txLen == 0) {
                    if (pI2CHandle->sr == I2C_SR_DISABLE) {
                        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                    }
                    I2C_CloseSendData(pI2CHandle);
                    I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
                }
            }
        }
    }

    // 4. Handle Slave Stop Detection Flag Event (STOPF Flag)
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_SR1_STOPF;
    if (temp1 && temp3) {
        // Clear STOPF by reading SR1 and then writing to CR1
        pI2CHandle->pI2Cx->CR1 |= 0x0000;
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOPF);
    }

    // 5. Handle Transmit Data Register Empty Event (TXE Flag)
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_SR1_TXE;
    if (temp1 && temp_bufen && temp3) {
        if (pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL) {
            if (pI2CHandle->txRxState == I2C_BUSY_IN_TX) {
                if (pI2CHandle->txLen > 0) {
                    pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);
                    pI2CHandle->txLen--;
                    pI2CHandle->pTxBuffer++;
                }
            }
        } else {
            // Slave Mode
            if (pI2CHandle->pI2Cx->SR2 & I2C_SR2_TRA) {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
            }
        }
    }

    // 6. Handle Receive Data Register Not Empty Event (RXNE Flag)
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_SR1_RXNE;
    if (temp1 && temp_bufen && temp3) {
        if (pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL) {
            if (pI2CHandle->txRxState == I2C_BUSY_IN_RX) {
                if (pI2CHandle->rxSize == 1) {
                    *pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
                    pI2CHandle->rxLen--;
                } else if (pI2CHandle->rxSize > 1) {
                    if (pI2CHandle->rxLen == 2) {
                        I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
                    }
                    *pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
                    pI2CHandle->pRxBuffer++;
                    pI2CHandle->rxLen--;
                    pI2CHandle->rxSize--;
                }
                if (pI2CHandle->rxLen == 0) {
                    if (pI2CHandle->sr == I2C_SR_DISABLE) {
                        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                    }
                    I2C_CloseReceiveData(pI2CHandle);
                    I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
                }
            }
        } else {
            // Slave Mode
            if (!(pI2CHandle->pI2Cx->SR2 & I2C_SR2_TRA)) {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCVD);
            }
        }
    }
}

/**
 * @brief  Core execution routing system processing physical error flags and exception failures.
 */
void I2C_ER_IRQHandling(I2C_Handle_t* pI2CHandle)
{
    uint32_t temp1, temp2;
    temp2 = (pI2CHandle->pI2Cx->CR2) & I2C_CR2_ITERREN;

    // 1. Bus Error (BERR Flag)
    temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_BERR;
    if (temp1 && temp2) {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_BERR;
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
    }

    // 2. Arbitration Lost Error (ARLO Flag)
    temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_ARLO;
    if (temp1 && temp2) {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_ARLO;
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
    }

    // 3. Acknowledge Failure Error (AF Flag)
    temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_AF;
    if (temp1 && temp2) {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_AF;
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
        I2C_CloseSendData(pI2CHandle);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);
    }

    // 4. Overrun/Underrun Error (OVR Flag)
    temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_OVR;
    if (temp1 && temp2) {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_OVR;
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
    }

    // 5. Timeout Line Error (TIMEOUT Flag)
    temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_TIMEOUT;
    if (temp1 && temp2) {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_TIMEOUT;
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
    }
}

__attribute__((weak)) void I2C_ApplicationEventCallback(I2C_Handle_t* pI2CHandle, uint8_t appEv)
{
    // Weak definition hook for main application logic overrides
}
