/**
 ******************************************************************************
 * @file           : stm32f446xx_i2c_driver.h
 * @brief          : Concise, beginner-friendly I2C Peripheral Driver Header
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#ifndef STM32F446XX_I2C_DRIVER_H_
#define STM32F446XX_I2C_DRIVER_H_

#include "stm32f446xx.h"

typedef struct {
    uint32_t sclSpeed;       /*!< Choose I2C bus speed: Standard (100kHz) or Fast Mode (up to 400kHz). @ref I2C_SCL_SPEEDS */
    uint8_t deviceAddress;   /*!< Your own custom 7-bit device address (only needed if acting as a slave) */
    uint8_t ackControl;      /*!< Enable or disable automatic ACK bit generation. @ref I2C_ACK_OPTIONS */
    uint8_t fmDutyCycle;     /*!< Choose the Fast Mode clock timing ratio (high/low splits). @ref I2C_FM_DUTY_CYCLES */
} I2C_Config_t;

typedef struct {
    I2C_RegDef_t *pI2Cx;     /*!< Base address of the I2C instance (I2C1, I2C2, I2C3) */
    I2C_Config_t I2C_Config; /*!< Your custom I2C settings checklist */
    uint8_t *pTxBuffer;      /*!< Pointer to the data array you want to send */
    uint8_t *pRxBuffer;      /*!< Pointer to store incoming received data */
    uint32_t txLen;          /*!< Number of bytes left to transmit */
    uint32_t rxLen;          /*!< Number of bytes left to receive */
    uint8_t txRxState;       /*!< Current communication state: Idle, Busy TX, or Busy RX */
    uint8_t devAddr;         /*!< Target address of the external slave device */
    uint32_t rxSize;         /*!< Total size of bytes expected to read */
    uint8_t sr;              /*!< Keeps track of Repeated Start configuration settings */
} I2C_Handle_t;

/** @defgroup I2C_SCL_SPEEDS
  * @{ */
#define I2C_SCL_SPEED_SM                100000  /*!< Standard Mode: Speed up to 100 kHz */
#define I2C_SCL_SPEED_FM2K              200000  /*!< Fast Mode: Speed up to 200 kHz */
#define I2C_SCL_SPEED_FM4K              400000  /*!< Fast Mode: Speed up to 400 kHz */
/** @} */

/** @defgroup I2C_ACK_OPTIONS
  * @{ */
#define I2C_ACK_DIS                     0       /*!< Don't return an ACK pulse (ends a read stream) */
#define I2C_ACK_EN                      1       /*!< Send an ACK pulse back after receiving a byte */
/** @} */

/** @defgroup I2C_FM_DUTY_CYCLES
  * @{ */
#define I2C_FM_DUTY_2                   0       /*!< Clock timing ratio: T(Low) / T(High) = 2 */
#define I2C_FM_DUTY_16_9                1       /*!< Clock timing ratio: T(Low) / T(High) = 16/9 */
/** @} */

/* I2C Repeated Start Control Options */
#define I2C_SR_DISABLE                  0       /*!< Generate a STOP condition after data transfer finishes */
#define I2C_SR_ENABLE                   1       /*!< Keep control of the bus by omitting the STOP condition */

/* I2C Communication Application State Flags */
#define I2C_READY                       0
#define I2C_BUSY_IN_RX                  1
#define I2C_BUSY_IN_TX                  2

/* Interrupt Event Status Indicators passed to Application Callback */
#define I2C_EV_TX_CMPLT                 1
#define I2C_EV_RX_CMPLT                 2
#define I2C_EV_DATA_REQ                 3
#define I2C_EV_DATA_RCVD                4
#define I2C_EV_STOPF                    5

/* Interrupt Error Codes passed to Application Callback */
#define I2C_ERROR_BERR                  3       /*!< Bus Error: Misplaced Start/Stop pulses detected */
#define I2C_ERROR_ARLO                  4       /*!< Arbitration Lost: Master lost priority battle to another master */
#define I2C_ERROR_AF                    5       /*!< Acknowledge Failure: No ACK returned from receiver */
#define I2C_ERROR_OVR                   6       /*!< Overrun/Underrun Error: New data arrived before old was read */
#define I2C_ERROR_TIMEOUT               7       /*!< Timeout: Lines stayed Low too long */

/******************************************************************************/
/* Driver API Functions                                                       */
/******************************************************************************/

/**
 * @brief  Turns the power (peripheral clock) on or off for an I2C peripheral channel.
 * @param  pI2Cx: Base address of the I2C peripheral instance (I2C1, I2C2, I2C3).
 * @param  enOrDi: Set to ENABLE to turn power on, or DISABLE to turn it off.
 */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t enOrDi);

/**
 * @brief  Applies your custom settings (speed, address, ACK rules) to configure I2C hardware registers.
 * @param  pI2CHandle: Pointer to the handle holding the custom checklist.
 */
void I2C_Init(I2C_Handle_t *pI2CHandle);

/**
 * @brief  Resets an I2C peripheral bank back to its out-of-the-box factory defaults.
 * @param  pI2Cx: Base address of the target I2C register bank.
 */
void I2C_DeInit(I2C_RegDef_t *pI2Cx);

/**
 * @brief  Sends data over the I2C bus as a Master device (Blocking Mode).
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 * @param  pTxBuffer: Pointer to data array you want to transmit.
 * @param  len: Number of total bytes to transmit.
 * @param  slaveAddr: The 7-bit target device address you want to communicate with.
 * @param  sr: Pass I2C_SR_ENABLE for Repeated Start, or I2C_SR_DISABLE to let go of the bus.
 */
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr);

/**
 * @brief  Requests and reads data from a Slave device as a Master (Blocking Mode).
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 * @param  pRxBuffer: Pointer to data buffer where incoming bytes will go.
 * @param  len: Number of total bytes to request.
 * @param  slaveAddr: The 7-bit device address of the target slave.
 * @param  sr: Pass I2C_SR_ENABLE for Repeated Start, or I2C_SR_DISABLE for standard stop.
 */
void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr);

/**
 * @brief  Starts a non-blocking transmission using I2C event interrupts.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 * @param  pTxBuffer: Pointer to data array you want to send out.
 * @param  len: Total number of bytes to send.
 * @param  slaveAddr: The 7-bit address of the target slave device.
 * @param  sr: Pass I2C_SR_ENABLE for Repeated Start, or I2C_SR_DISABLE for a stop pulse.
 * @retval Returns I2C_READY if transmission setup succeeded, or state codes if busy.
 */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr);

/**
 * @brief  Starts a non-blocking reception request using I2C event interrupts.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 * @param  pRxBuffer: Pointer to data buffer where incoming bytes will be saved.
 * @param  len: Total number of bytes to request from the slave.
 * @param  slaveAddr: The 7-bit address of the target slave device.
 * @param  sr: Pass I2C_SR_ENABLE for Repeated Start, or I2C_SR_DISABLE for standard stop.
 * @retval Returns I2C_READY if reception setup succeeded, or state codes if busy.
 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t len, uint8_t slaveAddr, uint8_t sr);

/**
 * @brief  Sends a single byte of data out when acting as a Slave device.
 * @param  pI2Cx: Base address of the I2C instance.
 * @param  data: The explicit data byte value to load into the data register.
 */
void I2C_SlaveSendData(I2C_RegDef_t *pI2Cx, uint8_t data);

/**
 * @brief  Reads a single byte of data from the data register when acting as a Slave device.
 * @param  pI2Cx: Base address of the I2C instance.
 * @retval The data byte received from the external master device.
 */
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2Cx);

/**
 * @brief  Safely terminates a running interrupt-driven data transmission.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 */
void I2C_CloseSendData(I2C_Handle_t *pI2CHandle);

/**
 * @brief  Safely terminates a running interrupt-driven data reception request.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 */
void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle);

/**
 * @brief  Enables or Disables the entire I2C hardware block inside the control register.
 * @param  pI2Cx: Base address of the I2C instance.
 * @param  enOrDi: Set to ENABLE to allow communication, or DISABLE to shut the block down.
 */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t enOrDi);

/**
 * @brief  Checks if a specific flag inside the I2C Status Register (SR1 or SR2) is set.
 * @param  pI2Cx: Base address of the I2C instance.
 * @param  flagName: The register mask flag you want to check.
 * @retval Returns 1 if flag is up, or 0 if flag is down.
 */
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t flagName);

/**
 * @brief  Enables or disables control bits for handling standard I2C interrupts (buffer, event, error).
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper.
 * @param  enOrDi: Set to ENABLE to unmask interrupts, or DISABLE to mask them out.
 */
void I2C_EnableInterrupts(I2C_Handle_t *pI2CHandle, uint8_t enOrDi);

/**
 * @brief  Enables or disables an I2C interrupt line within the main CPU NVIC controller.
 * @param  irqNumber: Internal index coordination number of the exception vector line.
 * @param  enOrDi: Set to ENABLE to listen to this exception, or DISABLE to mask it out.
 */
void I2C_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi);

/**
 * @brief  Sets a rank priority level execution weight number for an I2C interrupt line.
 * @param  irqNumber: Internal index coordination number being ranked.
 * @param  irqPriority: Rank assignment priority scale number (0 to 15).
 */
void I2C_IRQPriorityConfig(uint8_t irqNumber, uint8_t irqPriority);

/**
 * @brief  The core routing handler called when standard I2C Success/Event interrupts trigger.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper tracking states.
 */
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle);

/**
 * @brief  The core routing handler called when I2C hardware error states trigger.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper tracking states.
 */
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle);

/**
 * @brief  An application callback function that notifications jump to when I2C events complete.
 * @param  pI2CHandle: Pointer to your active I2C operational handle wrapper tracking states.
 * @param  appEv: The explicit event code flag signaling what happened (e.g., I2C_EV_TX_CMPLT).
 */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t appEv);

#endif /* STM32F446XX_I2C_DRIVER_H_ */
