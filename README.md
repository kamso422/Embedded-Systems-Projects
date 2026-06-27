# STM32F446xx Custom Bare-Metal Peripheral Driver Suite

A high-performance, modular, zero-dependency C driver library built entirely from scratch for the STM32F446xx (ARM Cortex-M4) microcontroller.

This repository houses custom peripheral drivers designed without relying on vendor abstraction layers (such as STMicroelectronics HAL or LL libraries). The goal of this project is to achieve absolute mastery over silicon-level operations, register maps, hardware bus topologies, and exact data-frame timings.

## 🚀 Key Architectural Pillars
* **True Bare-Metal**: Direct manipulation of memory-mapped peripheral registers using bitwise masks and operations.
* **Unified API Design**: Strict, predictable camelCase object handle-to-peripheral architecture across all communication protocols (`GPIO`, `SPI`, `I2C`, `USART`).
* **Non-Blocking Execution Async Architecture**: Implements fully developed Interrupt Service Routines (ISRs) handling complex hardware flags, transaction lifecycles, and application callback hooks.

---

## 📂 Repository Structure

```text
├── drivers/
│   ├── Inc/
│   │   ├── stm32f446xx.h                  # Core MCU definitions, register addresses, and vector maps
│   │   ├── stm32f446xx_gpio_driver.h      # GPIO Controller structures, speed configurations & APIs
│   │   ├── stm32f446xx_spi_driver.h       # SPI Master/Slave serial bus configuration handles
│   │   ├── stm32f446xx_i2c_driver.h       # I2C Serial Engine profiles, SCL timings & state flags
│   │   └── stm32f446xx_usart_driver.h     # USART/UART Asynchronous engine structures & frame limits
│   └── Src/
│       ├── stm32f446xx_gpio_driver.h       # GPIO Pin Multiplexing & Input/Output operations
│       ├── stm32f446xx_spi_driver.h        # SPI Sync/Async multi-byte stream transactions
│       ├── stm32f446xx_i2c_driver.c       # I2C Protocol state machinery (Address phase, ACK, Stop)
│       └── stm32f446xx_usart_driver.c     # USART Non-blocking state tracker & Baud Rate fraction sets
└── README.md
