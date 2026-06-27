/**
 ******************************************************************************
 * @file           : stm32f446xx_gpio_driver.c
 * @brief          : Concise, beginner-friendly GPIO Driver Implementation
 * @author         : Chikamso Okeru
 ******************************************************************************
 */

#include "stm32f446xx_gpio_driver.h"

/**
 * @brief  Turns the peripheral clock (power switch) on or off for a port bank.
 * @param  pGPIOx: Port address (GPIOA, GPIOB, etc.)
 * @param  enOrDi: ENABLE to turn power on, DISABLE to turn it off
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t enOrDi)
{
    if (enOrDi == ENABLE) {
        if      (pGPIOx == GPIOA) GPIOA_PCLK_EN();
        else if (pGPIOx == GPIOB) GPIOB_PCLK_EN();
        else if (pGPIOx == GPIOC) GPIOC_PCLK_EN();
        else if (pGPIOx == GPIOD) GPIOD_PCLK_EN();
        else if (pGPIOx == GPIOE) GPIOE_PCLK_EN();
        else if (pGPIOx == GPIOF) GPIOF_PCLK_EN();
        else if (pGPIOx == GPIOG) GPIOG_PCLK_EN();
        else if (pGPIOx == GPIOH) GPIOH_PCLK_EN();
    } else {
        if      (pGPIOx == GPIOA) GPIOA_PCLK_DIS();
        else if (pGPIOx == GPIOB) GPIOB_PCLK_DIS();
        else if (pGPIOx == GPIOC) GPIOC_PCLK_DIS();
        else if (pGPIOx == GPIOD) GPIOD_PCLK_DIS();
        else if (pGPIOx == GPIOE) GPIOE_PCLK_DIS();
        else if (pGPIOx == GPIOF) GPIOF_PCLK_DIS();
        else if (pGPIOx == GPIOG) GPIOG_PCLK_DIS();
        else if (pGPIOx == GPIOH) GPIOH_PCLK_DIS();
    }
}

/**
 * @brief  Applies custom settings (mode, speed, resistors) to configure a pin.
 * @param  pGPIOHandle: Pointer to handle holding target port and settings checklist.
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{
    uint32_t temp = 0;

    // 1. Turn on the peripheral clock for this port bank
    GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);

    // 2. Configure the Pin Mode (Input, Output, Alternate Function, or Interrupt)
    if (pGPIOHandle->GPIO_PinConfig.pinMode <= GPIO_MODE_ANALOG) {
        // Regular non-interrupt modes (Each pin takes 2 bits)
        temp = (pGPIOHandle->GPIO_PinConfig.pinMode << (2 * pGPIOHandle->GPIO_PinConfig.pinNumber));
        pGPIOHandle->pGPIOx->MODER &= ~(0x3 << (2 * pGPIOHandle->GPIO_PinConfig.pinNumber)); // Clear old bits
        pGPIOHandle->pGPIOx->MODER |= temp;
    } else {
        // Interrupt mode configurations
        if (pGPIOHandle->GPIO_PinConfig.pinMode == GPIO_MODE_IT_FT) {
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
            EXTI->RTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
        } else if (pGPIOHandle->GPIO_PinConfig.pinMode == GPIO_MODE_IT_RT) {
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
            EXTI->FTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
        } else if (pGPIOHandle->GPIO_PinConfig.pinMode == GPIO_MODE_IT_RFT) {
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
        }

        // Connect the GPIO port to the EXTI system line using SYSCFG
        uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.pinNumber / 4;
        uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.pinNumber % 4;
        uint8_t portCode = 0;

        if      (pGPIOHandle->pGPIOx == GPIOA) portCode = 0;
        else if (pGPIOHandle->pGPIOx == GPIOB) portCode = 1;
        else if (pGPIOHandle->pGPIOx == GPIOC) portCode = 2;
        else if (pGPIOHandle->pGPIOx == GPIOD) portCode = 3;
        else if (pGPIOHandle->pGPIOx == GPIOE) portCode = 4;
        else if (pGPIOHandle->pGPIOx == GPIOF) portCode = 5;
        else if (pGPIOHandle->pGPIOx == GPIOG) portCode = 6;
        else if (pGPIOHandle->pGPIOx == GPIOH) portCode = 7;

        SYSCFG_PCLK_EN();
        SYSCFG->EXTICR[temp1] &= ~(0xF << (temp2 * 4)); // Clear old configuration
        SYSCFG->EXTICR[temp1] |= (portCode << (temp2 * 4));

        // Enable the interrupt delivery mask line
        EXTI->IMR |= (1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
    }

    // 3. Configure the Output Speed (Each pin takes 2 bits)
    temp = (pGPIOHandle->GPIO_PinConfig.pinOspeed << (2 * pGPIOHandle->GPIO_PinConfig.pinNumber));
    pGPIOHandle->pGPIOx->OSPEEDER &= ~(0x3 << (2 * pGPIOHandle->GPIO_PinConfig.pinNumber));
    pGPIOHandle->pGPIOx->OSPEEDER |= temp;

    // 4. Configure the Pull-Up/Pull-Down Resistors (Each pin takes 2 bits)
    temp = (pGPIOHandle->GPIO_PinConfig.pinPuPd << (2 * pGPIOHandle->GPIO_PinConfig.pinNumber));
    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3 << (2 * pGPIOHandle->GPIO_PinConfig.pinNumber));
    pGPIOHandle->pGPIOx->PUPDR |= temp;

    // 5. Configure the Output Electrical Driving Type (Each pin takes 1 bit)
    temp = (pGPIOHandle->GPIO_PinConfig.pinOtype << pGPIOHandle->GPIO_PinConfig.pinNumber);
    pGPIOHandle->pGPIOx->OTYPER &= ~(0x1 << pGPIOHandle->GPIO_PinConfig.pinNumber);
    pGPIOHandle->pGPIOx->OTYPER |= temp;

    // 6. Configure the Alternate Function Multiplexing (Each pin takes 4 bits)
    if (pGPIOHandle->GPIO_PinConfig.pinMode == GPIO_MODE_ALTFN) {
        uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.pinNumber / 8;
        uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.pinNumber % 8;

        if (temp1 == 0) {
            pGPIOHandle->pGPIOx->AFRL &= ~(0xF << (4 * temp2));
            pGPIOHandle->pGPIOx->AFRL |= (pGPIOHandle->GPIO_PinConfig.pinAltFun << (4 * temp2));
        } else {
            pGPIOHandle->pGPIOx->AFRH &= ~(0xF << (4 * temp2));
            pGPIOHandle->pGPIOx->AFRH |= (pGPIOHandle->GPIO_PinConfig.pinAltFun << (4 * temp2));
        }
    }
}

/**
 * @brief  Resets a full port bank and all its pins back to their factory state.
 * @param  pGPIOx: Base address of the port bank to wipe clean.
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx)
{
    if      (pGPIOx == GPIOA) RESET_GPIOA_REG();
    else if (pGPIOx == GPIOB) RESET_GPIOB_REG();
    else if (pGPIOx == GPIOC) RESET_GPIOC_REG();
    else if (pGPIOx == GPIOD) RESET_GPIOD_REG();
    else if (pGPIOx == GPIOE) RESET_GPIOE_REG();
    else if (pGPIOx == GPIOF) RESET_GPIOF_REG();
    else if (pGPIOx == GPIOG) RESET_GPIOG_REG();
    else if (pGPIOx == GPIOH) RESET_GPIOH_REG();
}

/**
 * @brief  Checks if a pin is sensing high voltage (3.3V) or low voltage (0V).
 * @param  pGPIOx: Port bank containing the target pin.
 * @param  pinNumber: The pin number (0 to 15) to read from.
 * @retval 1 if sensing high voltage, or 0 if sensing low voltage.
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber)
{
    return (uint8_t)((pGPIOx->IDR >> pinNumber) & 0x00000001U);
}

/**
 * @brief  Reads the status of all 16 pins on a port bank at the same time.
 * @param  pGPIOx: Port bank to read entirely.
 * @retval A 16-bit number showing the state of every pin on that port.
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx)
{
    return (uint16_t)pGPIOx->IDR;
}

/**
 * @brief  Sends an electrical signal out of a pin, forcing it High or Low.
 * @param  pGPIOx: Port bank containing the target pin.
 * @param  pinNumber: The pin number (0 to 15) to write to.
 * @param  value: SET (1) to output 3.3V, or RESET (0) to output 0V.
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber, uint8_t value)
{
    if (value == SET) {
        pGPIOx->ODR |= (1 << pinNumber);
    } else {
        pGPIOx->ODR &= ~(1 << pinNumber); // Fixed: Properly masks off and clears the pin bit
    }
}

/**
 * @brief  Writes data to all 16 pins on a port bank simultaneously.
 * @param  pGPIOx: Port bank to write to.
 * @param  value: A 16-bit layout mask specifying the state of every output pin.
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t value)
{
    pGPIOx->ODR = value;
}

/**
 * @brief  Flips the current output state of a pin (LED on if off, or off if on).
 * @param  pGPIOx: Port bank containing the target pin.
 * @param  pinNumber: The pin number (0 to 15) to toggle.
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber)
{
    pGPIOx->ODR ^= (1 << pinNumber);
}

/**
 * @brief  Enables or disables a pin interrupt inside the processor's main NVIC controller.
 * @param  irqNumber: Internal interrupt position number assigned to the pin.
 * @param  enOrDi: ENABLE to turn on the interrupt listener, DISABLE to turn it off.
 */
void GPIO_IRQInterruptConfig(uint8_t irqNumber, uint8_t enOrDi)
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
 * @brief  Sets a rank priority level for an interrupt.
 * @param  irqNumber: Internal interrupt position number being ranked.
 * @param  irqPriority: Priority rank number (0 to 15; lower means more urgent).
 */
void GPIO_IRQPriorityConfig(uint8_t irqNumber, uint32_t irqPriority)
{
    // The top 4 bits of each priority byte are used for ranking on the STM32F4
    uint8_t iprx = irqNumber / 4;
    uint8_t iprx_section = irqNumber % 4;
    uint8_t shift_amount = (8 * iprx_section) + (8 - 4);

    *(NVIC_IPR0 + iprx) &= ~(0xF << shift_amount); // Clear old priority field
    *(NVIC_IPR0 + iprx) |= (irqPriority << shift_amount);
}

/**
 * @brief  Clears the internal 'alert flag' inside the EXTI peripheral after an interrupt runs.
 * @param  pinNumber: The pin number whose pending interrupt needs to be reset.
 */
void GPIO_IRQHandling(uint8_t pinNumber)
{
    // If the flag is set, clear it by writing a '1' to it
    if (EXTI->PR & (1 << pinNumber)) {
        EXTI->PR |= (1 << pinNumber);
    }
}
