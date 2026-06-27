/**
 ******************************************************************************
 * @file           : stm32f446xx_gpio_driver.h
 * @brief          : General Purpose I/O (GPIO) Peripheral Driver API Header
 * @author         : Chikamso Okeru
 * @processor      : ARM Cortex-M4F
 ******************************************************************************
 * @attention
 *
 * This file contains structural configurations, handle wrappers, bitfield 
 * configuration macro specifications, and explicit function declarations 
 * for interacting with the STM32F446xx GPIO matrix.
 *
 ******************************************************************************
 */

#ifndef STM32F446XX_GPIO_DRIVER_H_
#define STM32F446XX_GPIO_DRIVER_H_

#include "stm32f446xx.h"

/**
 * @brief GPIO Pin Configuration Structural Definition
 */
typedef struct {
    uint8_t pinNumber;      /*!< Target GPIO Pin Number. @ref GPIO_PIN_NUMBERS */
    uint8_t pinMode;        /*!< Input/Output/Alternate Function Mode. @ref GPIO_MODES */
    uint8_t pinOtype;       /*!< Output Configuration (Push-Pull/Open-Drain). @ref GPIO_OUTPUT_TYPES */
    uint8_t pinOspeed;      /*!< Electrical Speed. @ref GPIO_SPEEDS */
    uint8_t pinPuPd;        /*!< Internal Resistor Topology (Pull-up/Pull-down). @ref GPIO_PUPD_MODES */
    uint8_t pinAltFun;      /*!< Alternate Function Multiplexing Matrix Mapping. @ref GPIO_ALT_FUNCTIONS */
} GPIO_PinConfig_t;

/**
 * @brief GPIO Peripheral Operational Handle Structure
 */
typedef struct {
    GPIO_RegDef_t *pGPIOx;         /*!< Base Address Pointer targeting the active GPIO peripheral port instance */
    GPIO_PinConfig_t GPIO_PinConfig; /*!< Underlying specific pin layout configurations */
} GPIO_Handle_t;


/** @defgroup GPIO_PIN_NUMBERS GPIO Pin Numbers Definition Specifiers
  * @{
  */
#define GPIO_PIN_NO_0                   0
#define GPIO_PIN_NO_1                   1
#define GPIO_PIN_NO_2                   2
#define GPIO_PIN_NO_3                   3
#define GPIO_PIN_NO_4                   4
#define GPIO_PIN_NO_5                   5
#define GPIO_PIN_NO_6                   6
#define GPIO_PIN_NO_7                   7
#define GPIO_PIN_NO_8                   8
#define GPIO_PIN_NO_9                   9
#define GPIO_PIN_NO_10                  10
#define GPIO_PIN_NO_11                  11
#define GPIO_PIN_NO_12                  12
#define GPIO_PIN_NO_13                  13
#define GPIO_PIN_NO_14                  14
#define GPIO_PIN_NO_15                  15


/** @defgroup GPIO_MODES GPIO Pin Operational Modes Configuration Parameters
  * @{
  */
#define GPIO_MODE_INPUT                 0   /*!< Digital Input Operation */
#define GPIO_MODE_OUTPUT                1   /*!< Digital Output Operation */
#define GPIO_MODE_ALTFN                 2   /*!< Peripheral Alternate Function Multiplexing */
#define GPIO_MODE_ANALOG                3   /*!< High-Impedance Analog Input Channel */
#define GPIO_MODE_IT_RT                 4   /*!< External Interrupt Trigger: Rising Edge */
#define GPIO_MODE_IT_FT                 5   /*!< External Interrupt Trigger: Falling Edge */
#define GPIO_MODE_IT_RFT                6   /*!< External Interrupt Trigger: Rising and Falling Edge */


/** @defgroup GPIO_OUTPUT_TYPES GPIO Output Physical Driver Configurations
  * @{
  */
#define GPIO_OP_TYPE_PP                 0   /*!< Active Push-Pull Output Circuitry */
#define GPIO_OP_TYPE_OD                 1   /*!< Open-Drain Sinking Configuration */


/** @defgroup GPIO_SPEEDS GPIO Output Slew-Rate Operational Speed Thresholds
  * @{
  */
#define GPIO_SPEED_LOW                  0   /*!< Max operational target speed threshold 2MHz */
#define GPIO_SPEED_MEDIUM               1   /*!< Max operational target speed threshold 25MHz */
#define GPIO_SPEED_FAST                 2   /*!< Max operational target speed threshold 50MHz */
#define GPIO_SPEED_HIGH                 3   /*!< Max operational target speed threshold 100MHz */


/** @defgroup GPIO_PUPD_MODES GPIO Internal Resistor Biasing Topology Options
  * @{
  */
#define GPIO_NO_PUPD                    0   /*!< Floating High-Impedance Pin State */
#define GPIO_PIN_PU                     1   /*!< Internal Pull-Up Resistor Engagement */
#define GPIO_PIN_PD                     2   /*!< Internal Pull-Down Resistor Engagement */


/** @defgroup GPIO_ALT_FUNCTIONS GPIO Alternate Function Routing Matrix Identifiers
  * @{
  */
#define GPIO_AF_0                       0
#define GPIO_AF_1                       1
#define GPIO_AF_2                       2
#define GPIO_AF_3                       3
#define GPIO_AF_4                       4
#define GPIO_AF_5                       5
#define GPIO_AF_6                       6
#define GPIO_AF_7                       7
#define GPIO_AF_8                       8
#define GPIO_AF_9                       9
#define GPIO_AF_10                      10
#define GPIO_AF_11                      11
#define GPIO_AF_12                      12
#define GPIO_AF_13                      13
#define GPIO_AF_14                      14
#define GPIO_AF_15                      15


/******************************************************************************/
/* */
/* Driver API Function Prototypes                                  */
/* */
/******************************************************************************/

/**
 * @brief  Turns the peripheral clock (power switch) on or off for a specific GPIO port bank.
 * @param  pGPIOx: The base address of the port bank you want to modify (e.g., GPIOA, GPIOB).
 * @param  enOrDi: Set to 'ENABLE' to power it up, or 'DISABLE' to shut it down and save energy.
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t enOrDi);

/**
 * @brief  Applies your custom settings (mode, speed, resistors) to configure a pin's behavior.
 * @param  pGPIOHandle: Pointer to the struct holding the target port and configuration settings.
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);

/**
 * @brief  Resets a full port bank and all its pins back to their out-of-the-box factory state.
 * @param  pGPIOx: The base address of the port bank you want to wipe clean.
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

/**
 * @brief  Checks if a specific pin is sensing high voltage (3.3V) or low voltage (0V).
 * @param  pGPIOx: The port bank containing the pin you want to check.
 * @param  pinNumber: The specific pin number (0 to 15) you want to read from.
 * @retval Returns '1' if sensing high voltage, or '0' if sensing low voltage.
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber);

/**
 * @brief  Reads the status of all 16 pins on a port bank at the same time.
 * @param  pGPIOx: The port bank you want to read entirely.
 * @retval A 16-bit number representing the state of every pin on that port.
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);

/**
 * @brief  Sends an electrical signal out of a pin, forcing it High or Low.
 * @param  pGPIOx: The port bank containing the pin you want to control.
 * @param  pinNumber: The pin number (0 to 15) you want to write to.
 * @param  value: Set to 'SET' (1) to output 3.3V, or 'RESET' (0) to output 0V.
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber, uint8_t value);

/**
 * @brief  Writes data to all 16 pins on a port bank simultaneously.
 * @param  pGPIOx: The port bank you want to write to.
 * @param  value: A 16-bit layout mask specifying the state of every output pin.
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t value);

/**
 * @brief  Flips the current output state of a pin (turns an LED on if off, or off if on).
 * @param  pGPIOx: The port bank containing the pin you want to invert.
 * @param  pinNumber: The pin number (0 to 15) you want to toggle.
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber);

/**
 * @brief  Enables or disables a pin interrupt vector inside the processor's main interrupt controller (NVIC).
 * @param  irqNumber: The internal interrupt position number assigned to the pin.
 * @param  enOrDi: Set to 'ENABLE' to turn on the interrupt listener, or 'DISABLE' to turn it off.
 */
void GPIO_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi);

/**
 * @brief  Sets a rank priority level for an interrupt in case multiple interrupts trigger at the same time.
 * @param  irqNumber: The internal interrupt position number being ranked.
 * @param  irqPriority: The priority rank number (lower numbers mean more important/urgent).
 */
void GPIO_IRQPriorityConfig(uint8_t irqNumber, uint32_t irqPriority);

/**
 * @brief  Clears the internal 'alert flag' inside the EXTI peripheral after an interrupt routine executes.
 * @param  pinNumber: The pin number whose pending interrupt needs to be reset.
 */
void GPIO_IRQHandling(uint8_t pinNumber);

#endif /* STM32F446XX_GPIO_DRIVER_H_ */
