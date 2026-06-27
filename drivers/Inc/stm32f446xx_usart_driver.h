/**
 ******************************************************************************
 * @file           : stm32f446xx_usart_driver.h
 * @brief          : Concise, beginner-friendly USART Peripheral Driver Header
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#ifndef STM32F446XX_USART_DRIVER_H_
#define STM32F446XX_USART_DRIVER_H_

#include "stm32f44xx__driver.h"

typedef struct {
    uint8_t mode;             /*!< Choose mode configuration: Only TX, Only RX, or combined TX/RX. @ref USART_MODES */
    uint32_t baudRate;        /*!< Standard speed rates (e.g., 9600, 115200, etc.). @ref USART_BAUD_RATES */
    uint8_t stopBits;         /*!< Configure framing stop splits: 0.5, 1, 1.5, or 2 bits. @ref USART_STOP_BITS */
    uint8_t wordLength;       /*!< Select payload package configuration width: 8 bits or 9 bits. @ref USART_WORD_LENGTHS */
    uint8_t parityControl;    /*!< Select framing checking rules: None, Even, or Odd parity. @ref USART_PARITY_OPTIONS */
    uint8_t hwFlowControl;    /*!< Enable/Disable hardware flow lines (RTS/CTS). @ref USART_FLOW_CONTROL_OPTIONS */
} USART_Config_t;

typedef struct {
    USART_RegDef_t *pUSARTx;   /*!< Base register memory address block of the chosen instance */
    USART_Config_t USART_Config; /*!< Your custom operational configuration checklist profiles */
    uint8_t *pTxBuffer;        /*!< Address pointer to user array payload awaiting serial transit */
    uint8_t *pRxBuffer;        /*!< Address pointer to user collection window storing arriving packets */
    uint32_t txLen;            /*!< Track remaining data counter index volumes inside current TX cycle */
    uint32_t rxLen;            /*!< Track remaining data counter index volumes inside current RX cycle */
    uint8_t txBusyState;       /*!< Driver transmission operational line lifecycle status state flag */
    uint8_t rxBusyState;       /*!< Driver reception operational line lifecycle status state flag */
} USART_Handle_t;

/** @defgroup USART_MODES
  * @{ */
#define USART_MODE_ONLY_TX              0       /*!< Enable transmitter subsystem circuits only */
#define USART_MODE_ONLY_RX              1       /*!< Enable receiver subsystem circuits only */
#define USART_MODE_TXRX                 2       /*!< Enable both transmitter and receiver pipelines */
/** @} */

/** @defgroup USART_BAUD_RATES
  * @{ */
#define USART_STD_BAUD_1200             1200
#define USART_STD_BAUD_2400             2400
#define USART_STD_BAUD_9600             9600
#define USART_STD_BAUD_19200            19200
#define USART_STD_BAUD_38400            38400
#define USART_STD_BAUD_57600            57600
#define USART_STD_BAUD_115200           115200
#define USART_STD_BAUD_230400           230400
#define USART_STD_BAUD_460800           460800
#define USART_STD_BAUD_921600           921600
#define USART_STD_BAUD_2M               2000000
#define USART_STD_BAUD_3M               3000000
/** @} */

/** @defgroup USART_STOP_BITS
  * @{ */
#define USART_STOPBITS_1                0       /*!< Standard framing edge closure constraint rule */
#define USART_STOPBITS_0_5              1       /*!< Half bit frame closure timing element fraction */
#define USART_STOPBITS_2                2       /*!< Two bit extended validation framework spacing */
#define USART_STOPBITS_1_5              3       /*!< Custom split boundary specification fraction */
/** @} */

/** @defgroup USART_WORD_LENGTHS
  * @{ */
#define USART_WORDLEN_8BITS             0       /*!< Standard 8-bit characters payload width */
#define USART_WORDLEN_9BITS             1       /*!< 9-bit characters (often utilized when parity is enabled) */
/** @} */

/** @defgroup USART_PARITY_OPTIONS
  * @{ */
#define USART_PARITY_DISABLE            0       /*!< No parity bit attached or evaluated */
#define USART_PARITY_EN_EVEN            1       /*!< Even parity calculation check added to frame packages */
#define USART_PARITY_EN_ODD             2       /*!< Odd parity calculation check added to frame packages */
/** @} */

/** @defgroup USART_FLOW_CONTROL_OPTIONS
  * @{ */
#define USART_HW_FLOW_CTRL_NONE         0       /*!< Disable flow control lines completely */
#define USART_HW_FLOW_CTRL_RTS          1       /*!< Enable Request To Send hardware pin line control only */
#define USART_HW_FLOW_CTRL_CTS          2       /*!< Enable Clear To Send hardware pin line control only */
#define USART_HW_FLOW_CTRL_RTS_CTS      3       /*!< Enable both RTS and CTS automated handshake tracking */
/** @} */

/* USART Driver Communication Line Lifecycle Flags */
#define USART_READY                     0
#define USART_BUSY_IN_RX                1
#define USART_BUSY_IN_TX                2

/* Interrupt Event Flags passed back to Application Callback */
#define USART_EVENT_TX_CMPLT            0
#define USART_EVENT_RX_CMPLT            1
#define USART_EVENT_IDLE                2
#define USART_EVENT_CTS                 3
#define USART_EVENT_PE                  4       /*!< Parity Error validation failure flag detected */
#define USART_ERR_FE                    5       /*!< Framing Error validation failure flag detected */
#define USART_ERR_NE                    6       /*!< Noise Error confirmation status flag generated */
#define USART_ERR_ORE                   7       /*!< Overrun Data line overflow processing bottleneck constraint */

/******************************************************************************/
/* Driver API Functions                                                       */
/******************************************************************************/

/**
 * @brief  Turns the power (peripheral clock) on or off for a USART peripheral channel.
 * @param  pUSARTx: Base address of the USART peripheral instance (USART1, USART2, etc.).
 * @param  enOrDi: Set to ENABLE to turn power on, or DISABLE to turn it off.
 */
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t enOrDi);

/**
 * @brief  Applies your custom configuration settings to initialize a USART hardware block.
 * @param  pUSARTHandle: Pointer to the driver operational profile wrapper.
 */
void USART_Init(USART_Handle_t *pUSARTHandle);

/**
 * @brief  Resets a USART peripheral bank back to its out-of-the-box factory defaults.
 * @param  pUSARTx: Base address of the target USART register bank.
 */
void USART_DeInit(USART_RegDef_t *pUSARTx);

/**
 * @brief  Transmits an array stream over the line character by character (Blocking Mode).
 * @param  pUSARTHandle: Pointer to your active operational handler configuration.
 * @param  pTxBuffer: Pointer to data array payload block layout.
 * @param  len: Total number of bytes elements queued for transit.
 */
void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t len);

/**
 * @brief  Collects incoming line data streams sequentially inside a user-defined block array (Blocking Mode).
 * @param  pUSARTHandle: Pointer to your active operational handler configuration.
 * @param  pRxBuffer: Pointer to the container where data should be stored.
 * @param  len: Total number of byte packages expected for incoming line capture.
 */
void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t len);

/**
 * @brief  Starts a non-blocking transmission using peripheral interrupt lines.
 * @param  pUSARTHandle: Pointer to your active operational handler configuration.
 * @param  pTxBuffer: Pointer to data array payload block layout.
 * @param  len: Total number of bytes elements queued for transit.
 * @retval Returns USART_READY if transmission setup succeeded, or state codes if busy.
 */
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t len);

/**
 * @brief  Starts a non-blocking reception capture sequence using peripheral interrupt lines.
 * @param  pUSARTHandle: Pointer to your active operational handler configuration.
 * @param  pRxBuffer: Pointer to the container where data should be stored.
 * @param  len: Total number of byte packages expected for incoming line capture.
 * @retval Returns USART_READY if reception setup succeeded, or state codes if busy.
 */
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t len);

/**
 * @brief  Safely terminates a running interrupt-driven data transmission tracking state.
 * @param  pUSARTHandle: Pointer to your active operational handler configuration.
 */
void USART_CloseTransmission(USART_Handle_t *pUSARTHandle);

/**
 * @brief  Safely terminates a running interrupt-driven data reception tracking state.
 * @param  pUSARTHandle: Pointer to your active operational handler configuration.
 */
void USART_CloseReception(USART_Handle_t *pUSARTHandle);

/**
 * @brief  Enables or Disables the main USART hardware gate block inside Control Register 1.
 * @param  pUSARTx: Base address of the target USART peripheral block.
 * @param  enOrDi: Set to ENABLE to power the gateway active, or DISABLE to close it.
 */
void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t enOrDi);

/**
 * @brief  Checks if a specific flag inside the USART Status Register (SR) is set.
 * @param  pUSARTx: Base address of the target USART peripheral block.
 * @param  flagName: Bitwise macro target index field selection mask being checked.
 * @retval Returns 1 if flag is up, or 0 if flag is down.
 */
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t flagName);

/**
 * @brief  Clears a specific status flag inside the USART hardware architecture blocks.
 * @param  pUSARTx: Base address of the target USART peripheral block.
 * @param  statusFlagName: Targeting bit field mask identifying index elements to wipe.
 */
void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t statusFlagName);

/**
 * @brief  Enables or disables a USART interrupt line within the main CPU NVIC controller.
 * @param  irqNumber: Internal exception vector coordination line index number.
 * @param  enOrDi: Set to ENABLE to process exceptions, or DISABLE to mask them out.
 */
void USART_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi);

/**
 * @brief  Sets a rank priority execution weight scale factor for a USART interrupt line.
 * @param  irqNumber: Internal exception vector coordination line index number being ranked.
 * @param  irqPriority: Execution weight tier metric numbers (0 to 15 scaling ranges).
 */
void USART_IRQPriorityConfig(uint8_t irqNumber, uint8_t irqPriority);

/**
 * @brief  The core routing handler processing standard operational events, transmission empty and error calls.
 * @param  pUSARTHandle: Pointer to your active operational handler tracking state elements.
 */
void USART_IRQHandling(USART_Handle_t *pUSARTHandle);

/**
 * @brief  An application callback function that notifications jump to when USART events complete.
 * @param  pUSARTHandle: Pointer to your active operational handler tracking state elements.
 * @param  appEv: Event identity flags matching transaction milestones (e.g., USART_EVENT_TX_CMPLT).
 */
void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t appEv);

#endif /* STM32F446XX_USART_DRIVER_H_ */
