# Resource Management System with RTOS and Temperature Monitoring

I developed an embedded systems project utilizing the STM32 platform and FreeRTOS, with C and C++ as the programming languages. The build system was generated using **CMake** and the build environment was set up using **Make**. The hardware consisted of an STM32F103R6Tx MCU, 3 LEDs representing resources, 1 Alarm LED for error indication, 1 temperature sensor, and 1 pushbutton that consumed 1 resource for 10 seconds. I created 3 threads for the management of the 3 LEDs and implemented an interrupt service routine (ISR) triggered by the pushbutton to release a binary semaphore for a resource thread that was in a blocked state. The resource thread acquired a counting semaphore with a maximum count of 3. The Alarm LED would start blinking if the pushbutton was pressed more than 3 times, indicating that all resources were in use. I also utilized UART to print relevant and descriptive error messages in case of any issues. Additionally, I implemented a high-priority producer-consumer pattern with tasks to measure temperature and convert readings to temperature values. If the temperature was outside a specified range, the consumer task would suspend all resource threads to prevent damage to the MCU.

This was as a fun learning opportunity for several reasons:
1. **Utilizing RTOS**: By using FreeRTOS, I demonstrated the ability to implement a real-time operating system, which is crucial for developing real-world embedded systems projects.
2. **Managing resources**: The use of binary and counting semaphores in this project showed an understanding of resource management, a critical aspect of embedded systems development.
3. **Implementing Interrupt Service Routines (ISRs)**: This project involved implementing an ISR triggered by a pushbutton, demonstrating knowledge of interrupt handling, which is essential for developing embedded systems with user interfaces.
4. **Implementing threads**: The creation of three threads for the management of the LEDs showed an understanding of multi-threading, which is necessary for complex embedded systems that need to manage multiple tasks simultaneously.
5. **High-priority task management**: The implementation of the producer-consumer pattern with a high-priority task demonstrated an understanding of task prioritization, a crucial aspect of real-time systems.

Pinout configuration used for the project:
![](./code/PinoutConfiguration.png)

Circuit design:
![](./proteus/stm32f103r6_sem.SVG)