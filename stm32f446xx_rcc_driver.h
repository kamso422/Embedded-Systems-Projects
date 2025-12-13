/*
 * stm32f446xx_rcc_driver.h
 *
 *  Created on: Nov 24, 2025
 *      Author: chikamso
 */

#ifndef STM32F446XX_RCC_DRIVER_H_
#define STM32F446XX_RCC_DRIVER_H_

uint32_t RCC_GetPCLK1Value(void);
uint32_t RCC_GetPCLK2Value(void);
uint32_t RCC_GetPLLOutputClock(void);

#endif /* STM32F446XX_RCC_DRIVER_H_ */
