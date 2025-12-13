/*
 * STM32F44xxgpio__driver.h
 *
 *  Created on: Jun 1, 2025
 *      Author: chika
 */

#ifndef STM32F44XXGPIO__DRIVER_H_
#define STM32F44XXGPIO__DRIVER_H_

#include "STM32F44xx__driver.h"

/*
 * Defining the configuration structure for the gpio peripherals
 */
typedef struct{
	uint8_t pinNumber;
	uint8_t MODE;
	uint8_t OTYPE;
	uint8_t OSPEED;
	uint8_t PUPD;
	uint8_t ALTF;
}gpio_config_t;

/*
 * Defining the handle structure for the gpio peripherals
 */
typedef struct{
	GPIO_RegDef_t* gpioReg;
	gpio_config_t gpioConfigure;
}gpio_handle_t;

#define GPIO_PIN_NO_0			0
#define GPIO_PIN_NO_1			1
#define GPIO_PIN_NO_2			2
#define GPIO_PIN_NO_3			3
#define GPIO_PIN_NO_4			4
#define GPIO_PIN_NO_5			5
#define GPIO_PIN_NO_6			6
#define GPIO_PIN_NO_7			7
#define GPIO_PIN_NO_8			8
#define GPIO_PIN_NO_9			9
#define GPIO_PIN_NO_10			10
#define GPIO_PIN_NO_11			11
#define GPIO_PIN_NO_12			12
#define GPIO_PIN_NO_13			13
#define GPIO_PIN_NO_14			14
#define GPIO_PIN_NO_15			15

#define GPIO_MODE_INPUT			0
#define GPIO_MODE_OUTPUT		1
#define GPIO_MODE_ALFN			2
#define GPIO_ANALOG_MODE		3
#define GPIO_IT_MODE_RT			4
#define GPIO_IT_MODE_FT			5
#define GPIO_IT_MODE_RTFT		6

#define GPIO_LOW_SPEED			0
#define GPIO_MEDIUM_SPEED		1
#define GPIO_FAST_SPEED			2
#define GPIO_HIGH_SPEED			3

#define GPIO_NO_PUPD			0
#define GPIO_PU					1
#define GPIO_PD					2
#define GPIO_REVERSERD			3

#define GPIO_PP					0
#define GPIO_OD					1

#define AF0						0
#define AF1						1
#define AF2						2
#define AF3						3
#define AF4						4
#define AF5						5
#define AF6						6
#define AF7						7
#define AF8						8
#define AF9						9
#define AF10					10
#define AF11					11
#define AF12					12
#define AF13					13
#define AF14					14
#define AF15					15


void GPIO_PeriClockControl(GPIO_RegDef_t* pGPIOx,uint8_t EnorDi);
void gpioInit(gpio_handle_t* handle);
void gpioDeInit(GPIO_RegDef_t* pGPIOx);

uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t* pGPIOx,uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t* pGPIOx);
void GPIO_WriteToOutputPin(GPIO_RegDef_t* pGPIOx,uint8_t PinNumber,uint8_t Value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t* pGPIOx, uint16_t Value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t* pGPIOx,uint8_t PinNumber);

/*
 * IRQ Configuration and ISR handling
 */
void GPIO_IRQConfig(uint8_t IRQNumber,uint8_t ENorDi);
void GPIO_IRQHandling(uint8_t PinNumber);
void GPIO_IRQPriorityConfig(uint8_t IRQNUmber, uint8_t IRQpriority);


#endif /* STM32F44XXGPIO__DRIVER_H_ */
