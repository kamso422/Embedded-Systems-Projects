/**
 ******************************************************************************
 * @file           : stm32f446xx_spi_driver.h
 * @brief          : Concise, beginner-friendly SPI Peripheral Driver Header
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#ifndef STM32F446XX_SPI_DRIVER_H_
#define STM32F446XX_SPI_DRIVER_H_

#include "stm32f446xx.h"

typedef struct {
    uint8_t spiDeviceMode;   /*!< Role: Master (controls clock) or Slave. @ref SPI_DEVICE_MODES */
    uint8_t spiBusConfig;    /*!< Wiring: Full-Duplex, Half-Duplex, or Receive-Only. @ref SPI_BUS_CONFIGS */
    uint8_t spiSclkSpeed;    /*!< Clock speed prescaler divider. @ref SPI_SCLK_SPEEDS */
    uint8_t spiDFF;          /*!< Data Frame Format: 8-bit or 16-bit packets. @ref SPI_DFF_OPTIONS */
    uint8_t spiCPOL;         /*!< Clock Polarity: Idle state is High or Low. @ref SPI_CPOL_OPTIONS */
    uint8_t spiCPHA;         /*!< Clock Phase: Capture data on 1st or 2nd edge. @ref SPI_CPHA_OPTIONS */
    uint8_t spiSSM;          /*!< Slave Select Management: Hardware or Software pin control. @ref SPI_SSM_OPTIONS */
} SPI_Config_t;

typedef struct {
    SPI_RegDef_t *pSPIx;     /*!< Base address of the SPI instance (SPI1, SPI2, etc.) */
    SPI_Config_t SPIConfig;  /*!< Your custom SPI settings checklist */
    uint8_t *pTxBuffer;      /*!< Pointer to the data array you want to transmit */
    uint8_t *pRxBuffer;      /*!< Pointer to store incoming received data */
    uint32_t txLen;          /*!< Number of bytes/words left to transmit */
    uint32_t rxLen;          /*!< Number of bytes/words left to receive */
    uint8_t txState;         /*!< Current transmission state: Idle or Busy */
    uint8_t rxState;         /*!< Current reception state: Idle or Busy */
} SPI_Handle_t;

/** @defgroup SPI_DEVICE_MODES
  * @{ */
#define SPI_DEVICE_MODE_SLAVE           0   /*!< Listens to an external master clock */
#define SPI_DEVICE_MODE_MASTER          1   /*!< Generates the serial clock and dictates transfer */
/** @} */

/** @defgroup SPI_BUS_CONFIGS
  * @{ */
#define SPI_BUS_CONFIG_FD               1   /*!< Full-Duplex: Uses 4 wires (MOSI, MISO, SCLK, NSS) for concurrent TX/RX */
#define SPI_BUS_CONFIG_HD               2   /*!< Half-Duplex: Uses 3 wires; data line is shared for TX/RX */
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY   3   /*!< Simplex Receive-Only: Drops the transmit line entirely */
#define SPI_BUS_CONFIG_SIMPLEX_TXONLY   4   /*!< Simplex Transmit-Only: Drops the receive line entirely */
/** @} */

/** @defgroup SPI_SCLK_SPEEDS
  * @{ */
#define SPI_CLK_SPEED_DIV2              0   /*!< SPI Clock = Core Peripheral Clock / 2 */
#define SPI_CLK_SPEED_DIV4              1   /*!< SPI Clock = Core Peripheral Clock / 4 */
#define SPI_CLK_SPEED_DIV8              2   /*!< SPI Clock = Core Peripheral Clock / 8 */
#define SPI_CLK_SPEED_DIV16             3   /*!< SPI Clock = Core Peripheral Clock / 16 */
#define SPI_CLK_SPEED_DIV32             4   /*!< SPI Clock = Core Peripheral Clock / 32 */
#define SPI_CLK_SPEED_DIV64             5   /*!< SPI Clock = Core Peripheral Clock / 64 */
#define SPI_CLK_SPEED_DIV128            6   /*!< SPI Clock = Core Peripheral Clock / 128 */
#define SPI_CLK_SPEED_DIV256            7   /*!< SPI Clock = Core Peripheral Clock / 256 */
/** @} */

/** @defgroup SPI_DFF_OPTIONS
  * @{ */
#define SPI_DFF_8BITS                   0   /*!< Transmits data in 8-bit pieces (standard bytes) */
#define SPI_DFF_16BITS                  1   /*!< Transmits data in 16-bit pieces (half-words) */
/** @} */

/** @defgroup SPI_CPOL_OPTIONS
  * @{ */
#define SPI_CPOL_LOW                    0   /*!< Clock line stays Low (0V) when no data is moving */
#define SPI_CPOL_HIGH                   1   /*!< Clock line stays High (3.3V) when no data is moving */
/** @} */

/** @defgroup SPI_CPHA_OPTIONS
  * @{ */
#define SPI_CPHA_LOW                    0   /*!< Microcontroller samples data on the first clock shift edge */
#define SPI_CPHA_HIGH                   1   /*!< Microcontroller samples data on the second clock shift edge */
/** @} */

/** @defgroup SPI_SSM_OPTIONS
  * @{ */
#define SPI_SSM_DI                      0   /*!< Hardware handles Slave Select pin matching automatically */
#define SPI_SSM_EN                      1   /*!< Software controls the Slave Select pin manually using software bits */
/** @} */

/* SPI Communication Application State Flags */
#define SPI_READY                       0
#define SPI_BUSY_IN_RX                  1
#define SPI_BUSY_IN_TX                  2

/* Interrupt Event Callback Enumerators */
#define SPI_EVENT_TX_CMPLT              1
#define SPI_EVENT_RX_CMPLT              2
#define SPI_EVENT_OVR_ERR               3

/******************************************************************************/
/* Driver API Functions                                                       */
/******************************************************************************/

/**
 * @brief  Turns the power (peripheral clock) on or off for an SPI peripheral.
 * @param  pSPIx: Base address of the SPI instance (e.g., SPI1, SPI2).
 * @param  enOrDi: Set to ENABLE to turn power on, or DISABLE to turn it off.
 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t enOrDi);

/**
 * @brief  Applies your custom settings (mode, bus layout, clock parameters) to configure SPI hardware.
 * @param  pSPIHandle: Pointer to the handle holding the configuration settings.
 */
void SPI_Init(SPI_Handle_t *pSPIHandle);

/**
 * @brief  Resets an SPI peripheral back to its out-of-the-box factory defaults.
 * @param  pSPIx: Base address of the target SPI register bank.
 */
void SPI_DeInit(SPI_RegDef_t *pSPIx);

/**
 * @brief  Sends data out over the SPI bus. (Blocking method).
 * @param  pSPIx: Base address of the target SPI peripheral.
 * @param  pTxBuffer: Pointer to the data array you want to send out.
 * @param  len: Total number of bytes or words to transmit.
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len);

/**
 * @brief  Gathers data coming in from the SPI bus. (Blocking method: waits for data to arrive).
 * @param  pSPIx: Base address of the target SPI peripheral.
 * @param  pRxBuffer: Pointer to the array where incoming data should be saved.
 * @param  len: Total number of bytes or words to expect.
 */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len);

/**
 * @brief  Starts a non-blocking data transmission using system interrupts.
 * @param  pSPIHandle: Pointer to the SPI handle wrapper monitoring application buffers.
 * @retval Returns SPI_READY if transmission started, or state codes if the bus is busy.
 */
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle);

/**
 * @brief  Starts a non-blocking data reception using system interrupts.
 * @param  pSPIHandle: Pointer to the SPI handle wrapper monitoring application buffers.
 * @retval Returns SPI_READY if capture setup succeeded, or state codes if busy.
 */
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle);

/**
 * @brief  Turns the main SPI peripheral block on or off inside the control register.
 * @param  pSPIx: Base address of the target SPI instance.
 * @param  enOrDi: Set to ENABLE to let data flow, or DISABLE to close it.
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t enOrDi);

/**
 * @brief  Configures the internal SSI bit (only used when Software Slave Management is active).
 * @param  pSPIx: Base address of the target SPI instance.
 * @param  enOrDi: Set to ENABLE to pull the internal line High, or DISABLE to push it Low.
 */
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t enOrDi);

/**
 * @brief  Configures the SSOE bit to turn the physical NSS pin into an automatic master output.
 * @param  pSPIx: Base address of the target SPI instance.
 * @param  enOrDi: Set to ENABLE to let master assert the pin, or DISABLE to keep it passive.
 */
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t enOrDi);

/**
 * @brief  Enables or disables an SPI interrupt vector inside the processor's main NVIC controller.
 * @param  irqNumber: Internal interrupt position code assigned to this SPI instance.
 * @param  enOrDi: Set to ENABLE to activate the interrupt listener, or DISABLE to mask it out.
 */
void SPI_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi);

/**
 * @brief  Sets a rank priority level for an SPI interrupt line inside the NVIC register matrix.
 * @param  irqNumber: Internal interrupt position code being ranked.
 * @param  irqPriority: The rank importance number (lower numbers mean higher priority).
 */
void SPI_IRQPriorityConfig(uint8_t irqNumber, uint32_t irqPriority);

/**
 * @brief  The central interrupt handler that processes TX, RX, or error event flags when they trigger.
 * @param  pSPIHandle: Pointer to the application handle managing active buffer transitions.
 */
void SPI_IRQHandling(SPI_Handle_t *pSPIHandle);

/**
 * @brief  An application-side notification function that runs when an interrupt routine finishes up.
 * @param  pSPIHandle: Pointer to the handle tracking the completed transfers.
 * @param  appEv: The event identifier flag (e.g., SPI_EVENT_TX_CMPLT).
 */
__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t appEv);

#endif /* STM32F446XX_SPI_DRIVER_H_ */
