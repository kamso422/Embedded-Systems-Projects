/*
 * STM32F44xxgpio__driver.c
 *
 *  Created on: Jun 1, 2025
 *      Author: chika
 */
#include <stdint.h>
#include "STM32F44xx__driver.h"

void GPIO_PeriClockControl(GPIO_RegDef_t* pGPIOx,uint8_t EnorDi){
	if(EnorDi){
		if(pGPIOx == GPIOA){
			GPIOA_PCLK_EN();
		}else if(pGPIOx == GPIOB){
			GPIOB_PCLK_EN();
		}else if(pGPIOx == GPIOC){
			GPIOC_PCLK_EN();
		}else if(pGPIOx == GPIOD){
			GPIOD_PCLK_EN();
		}else if(pGPIOx == GPIOE){
			GPIOE_PCLK_EN();
		}else if(pGPIOx == GPIOF){
			GPIOF_PCLK_EN();
		}else if(pGPIOx == GPIOG){
			GPIOG_PCLK_EN();
		}else if(pGPIOx == GPIOH){
			GPIOH_PCLK_EN();
		}
	}else{
		if(pGPIOx == GPIOA){
			GPIOA_PCLK_DIS();
		}else if(pGPIOx == GPIOB){
			GPIOB_PCLK_DIS();
		}else if(pGPIOx == GPIOC){
			GPIOC_PCLK_DIS();
		}else if(pGPIOx == GPIOD){
			GPIOD_PCLK_DIS();
		}else if(pGPIOx == GPIOE){
			GPIOE_PCLK_DIS();
		}else if(pGPIOx == GPIOF){
			GPIOF_PCLK_DIS();
		}else if(pGPIOx == GPIOG){
			GPIOG_PCLK_DIS();
		}else if(pGPIOx == GPIOH){
			GPIOH_PCLK_DIS();
		}
	}
}

void gpioInit(gpio_handle_t* handle){
	GPIO_PeriClockControl(handle->gpioReg, ENABLE);
	uint32_t temp = 0;
	//Setting mode
	if(handle->gpioConfigure.MODE <= GPIO_ANALOG_MODE){
		temp |= (handle->gpioConfigure.MODE << (2 * handle->gpioConfigure.pinNumber));
		handle->gpioReg->MODER &= ~(0x3 << (2 * handle->gpioConfigure.pinNumber)); //Clearing register
		handle->gpioReg->MODER |= temp;
	}
	//setting pull up pull down resistors
	temp = 0;
	temp = (handle->gpioConfigure.PUPD << (2 * handle->gpioConfigure.pinNumber));
	handle->gpioReg->PUPDR &= ~(0x3 << (2 * handle->gpioConfigure.pinNumber)); //Clearing register
	handle->gpioReg->PUPDR |= temp;
	//Setting the Output type for the pin
	temp = 0;
	temp |= (handle->gpioConfigure.OTYPE << handle->gpioConfigure.pinNumber);
	handle->gpioReg->OTYPER &= ~(0x1 << handle->gpioConfigure.pinNumber); //Clearing register
	handle->gpioReg->OTYPER |= temp;
	//Setting the speed of the gpio pin
	temp = 0;
	temp |= (handle->gpioConfigure.OSPEED << (2 * handle->gpioConfigure.pinNumber));
	handle->gpioReg->OSPEEDER &= ~(0x3 << (2 * handle->gpioConfigure.pinNumber)); //Clearing register
	handle->gpioReg->OSPEEDER |= temp;
	if(handle->gpioConfigure.MODE == GPIO_MODE_ALFN){
		temp = 0;
		if(handle->gpioConfigure.pinNumber <= 7){
			temp |= (handle->gpioConfigure.ALTF << (4 * handle->gpioConfigure.pinNumber));
			handle->gpioReg->AFRL |= temp;
		}else{
			temp |= (handle->gpioConfigure.ALTF << ((handle->gpioConfigure.pinNumber % 8) * 4));
			handle->gpioReg->AFRH |= temp;
		}
	}else if(handle->gpioConfigure.MODE >= GPIO_IT_MODE_RT){
		//Setup the interrupt lines
		int num = 0;
		if(handle->gpioReg == GPIOA)
			num = 0;
		else if(handle->gpioReg == GPIOB)
			num = 1;
		else if(handle->gpioReg == GPIOC)
			num = 3;
		else if(handle->gpioReg == GPIOD)
			num = 4;
		else if(handle->gpioReg == GPIOE)
			num = 5;
		else if(handle->gpioReg == GPIOF)
			num = 6;
		else if(handle->gpioReg == GPIOG)
			num = 7;
		temp = 0;
		temp |= 0x1 << handle->gpioConfigure.pinNumber;//remove mask from interrupt line
		EXTI->IMR |= temp;
		temp = 0;
		if(handle->gpioConfigure.MODE == GPIO_IT_MODE_RT){
			temp = (0x1 << handle->gpioConfigure.pinNumber);
			EXTI->RTSR |= temp;
		}else if(handle->gpioConfigure.MODE == GPIO_IT_MODE_FT){
			temp = (0x1 << handle->gpioConfigure.pinNumber);
			EXTI->FTSR |= temp;
		}else if(handle->gpioConfigure.MODE == GPIO_IT_MODE_RTFT){
			temp = (0x1 << handle->gpioConfigure.pinNumber);
			EXTI->RTSR |= temp;
			EXTI->FTSR |= temp;
		}
		//Enable the clock for the SYSCFG peripheral
		SYSCFG_PCLK_EN();
		//Setting up the exact GPIO port
		if(handle->gpioConfigure.pinNumber <= 3){
			temp = 0;
			//temp |= num << (4 * handle->gpioConfigure.pinNumber);
			SYSCFG->EXTICR[0] &= ~(0xFU) << (4 * handle->gpioConfigure.pinNumber);//clearing register
			SYSCFG->EXTICR[0] |= num << (4 * handle->gpioConfigure.pinNumber);
		}else if(handle->gpioConfigure.pinNumber > 3 && handle->gpioConfigure.pinNumber <= 7){
			temp = 0;
			temp |= num << (4 * (handle->gpioConfigure.pinNumber % 4));
			SYSCFG->EXTICR[1] &= ~(0xFU) << (4 * (handle->gpioConfigure.pinNumber % 4));//clearing register
			SYSCFG->EXTICR[1] |= temp;
		}else if(handle->gpioConfigure.pinNumber > 7 && handle->gpioConfigure.pinNumber <= 11){
			temp = 0;
			temp |= num << (4 * (handle->gpioConfigure.pinNumber % 8));
			SYSCFG->EXTICR[2] &= ~(0xFU) << (4 * (handle->gpioConfigure.pinNumber % 8));//clearing register
			SYSCFG->EXTICR[2] |= temp;
		}else if(handle->gpioConfigure.pinNumber > 11 && handle->gpioConfigure.pinNumber <= 15){
			temp = 0;
			temp |= num << (4 * (handle->gpioConfigure.pinNumber % 12));
			SYSCFG->EXTICR[3] &= ~(0xFU) << (4 * (handle->gpioConfigure.pinNumber % 12));
			SYSCFG->EXTICR[3] |= temp;
		}
	}
}

void gpioDeInit(GPIO_RegDef_t* pGPIOx){
	if(pGPIOx == GPIOA){
		RESET_GPIOA_REG();
	}else if(pGPIOx == GPIOB){
		RESET_GPIOB_REG();
	}else if(pGPIOx == GPIOC){
		RESET_GPIOC_REG();
	}else if(pGPIOx == GPIOD){
		RESET_GPIOD_REG();
	}else if(pGPIOx == GPIOE){
		RESET_GPIOE_REG();
	}else if(pGPIOx == GPIOF){
		RESET_GPIOF_REG();
	}else if(pGPIOx == GPIOG){
		RESET_GPIOG_REG();
	}else if(pGPIOx == GPIOH){
		RESET_GPIOH_REG();
	}
}
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t* pGPIOx,uint8_t pinNumber){
	uint8_t num = pGPIOx->IDR >> pinNumber;
	return num &= 0x1;
}
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t* pGPIOx){
	return pGPIOx->IDR;
}
void GPIO_WriteToOutputPin(GPIO_RegDef_t* pGPIOx,uint8_t pinNumber,uint8_t value){
	pGPIOx->ODR |= value << pinNumber;
}
void GPIO_WriteToOutputPort(GPIO_RegDef_t* pGPIOx, uint16_t value){
	pGPIOx->ODR = value;
}
void GPIO_ToggleOutputPin(GPIO_RegDef_t* pGPIOx,uint8_t pinNumber){
	pGPIOx->ODR ^= 0x1 << pinNumber;
}

void GPIO_IRQConfig(uint8_t IRQNumber,uint8_t ENorDi){
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
void GPIO_IRQHandling(uint8_t PinNumber){
	if(EXTI->PR & (0x1) << PinNumber){
		EXTI->PR |= (0x1) << PinNumber;
	}
}
void GPIO_IRQPriorityConfig(uint8_t IRQNUmber, uint8_t IRQpriority){
	uint8_t iprx = IRQNUmber / 4;
	uint8_t iprSection = IRQNUmber % 4;
	uint8_t shiftNo = iprSection*8 + 4;
	*(NVIC_IPR0 + iprx) |= IRQpriority << shiftNo;
}
