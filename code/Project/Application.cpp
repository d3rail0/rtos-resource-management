#include <Project/application.hpp>

#define EV_FLAGS_ALARM_MASK 0xFFFFU

static void pushButtonISR();
__NO_RETURN void resourceTask(void* argument);
__NO_RETURN void alarmTask(void* argument);
void blinkAlarmLED(uint16_t blinkCount, const uint16_t blinkDuration);
void signalForError(Error error);

// Total count of available resources (in this case LEDs)
const uint8_t  RESOURCES_COUNT = 3;
const uint16_t RESOURCES[RESOURCES_COUNT] = {
    LED_1_Pin, LED_2_Pin, LED_3_Pin
};

osThreadId_t resourceThreads[3];

osSemaphoreId_t  semTaskEvent_id; 
osSemaphoreId_t  semFreeResources_id; 
osEventFlagsId_t evt_id;

void AppMain()
{
     /* Init scheduler */
    osKernelInitialize();

    evt_id = osEventFlagsNew(NULL);
    if (evt_id == NULL) 
        signalForError(E_GENERIC);

    // binary semaphore initial count set to 0
    // so that no resource task can start before
    // ISR relases it
    semTaskEvent_id = osSemaphoreNew(1U, 0U, NULL);

    // Counting semaphore that tracks currently available 
    // resources to mitigate a problem with ISR being able
    // to "buffer" a single additional resource task before 
    // it starts ignoring consequent resource utilization requests.
    semFreeResources_id = osSemaphoreNew(RESOURCES_COUNT, RESOURCES_COUNT, NULL);

    if(semTaskEvent_id == NULL || semFreeResources_id == NULL)
        signalForError(E_GENERIC);

    osThreadAttr_t ledTaskAttrs = {};
    ledTaskAttrs.name       = "LED_TASK";
    ledTaskAttrs.stack_size = 64 * 4;
    ledTaskAttrs.priority   = osPriorityNormal;

    // Start all the threads 
    for(uint8_t i = 0; i < RESOURCES_COUNT; ++i) {
        resourceThreads[i] = osThreadNew(resourceTask, (void*)&RESOURCES[i], &ledTaskAttrs);
    }

    osThreadAttr_t alarmTaskAttrs = {};
    alarmTaskAttrs.name       = "ALARM_TASK";
    alarmTaskAttrs.stack_size = 64 * 4;
    alarmTaskAttrs.priority   = osPriorityHigh;
    osThreadNew(alarmTask, NULL, &alarmTaskAttrs);

    // Init temperature controller
    TempControllerMain();
    
    /* Start scheduler */
    osKernelStart();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PushButton_1_Pin)
    {
        pushButtonISR();
    }
}

// Error signal event generator
void signalForError(Error error) {
    osEventFlagsSet(evt_id, uint32_t(error));
}

/**
 * @brief Utilizes resources for processing. 
 * 
 */
void pushButtonISR() {

    uint32_t freeResourceCount = osSemaphoreGetCount(semFreeResources_id);

    if (freeResourceCount == 0) {
        signalForError(E_NO_MORE_RESOURCES);
        // Return here to prevent buffering of another resource
        // utilization when all resources are exhausted.
        return;
    }

    // Release 1 token that resource tasks are waiting for.
    osStatus_t semStatus = osSemaphoreRelease(semTaskEvent_id);

    if(semStatus == osErrorParameter) {
        // semTaskEvent id is not valid.
        signalForError(E_INVALID_RES_SEM);
    }
}   

/**
 * @brief 
 * Blinks an alarm LED to signal that something might have 
 * went wrong.
 * @param blinkCount Total LED blink count.
 * @param blinkDuration Time ticks to wait between the full blink (LED toggle on and off).  
 */
void blinkAlarmLED(uint16_t blinkCount, const uint16_t blinkDuration) {
    static Led led(Alarm_1_GPIO_Port, Alarm_1_Pin);
    for(;blinkCount > 0; --blinkCount) {
        led.toggle();
        osDelay(blinkDuration);
        led.toggle();
        osDelay(blinkDuration);
    } 
}

/**
 * @brief High priority task which waits for error event signals
 * to process different error types by notifying user in two ways:
 *  1. Print descriptive error message via UART.
 *  2. Blink an Alaram LED for certain types of errors.
 * 
 * @note TODO: Change this function to support handling more than 1
 * error at a time. If during the blinking of LED two errors occur 
 * elsewhere, the event flags will be picked up for only one error
 * (the one that occured the last). Use message queue. 
 * @param argument Unused
 */
void alarmTask(void* argument) {
    uint32_t errorFlag;
    char*    msg;
    uint8_t  dataBuffer[3] = "\r\n";

    for(;;) {
        errorFlag = osEventFlagsWait(evt_id, EV_FLAGS_ALARM_MASK, osFlagsWaitAny, osWaitForever);

        msg = const_cast<char*>(Error::getErrorMessage(errorFlag));

        // Write error message
        if(HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(msg), static_cast<uint16_t>(std::strlen(msg)), 500) != HAL_OK) {
            signalForError(E_WRITE_FAILED);
        }
        
        // Write newline
        if(HAL_UART_Transmit(&huart1, dataBuffer, sizeof(dataBuffer), 500) != HAL_OK) {
            signalForError(E_WRITE_FAILED);
        }

        if(errorFlag == uint32_t(E_NO_MORE_RESOURCES)) 
            blinkAlarmLED(3, 250);
        else if (errorFlag == uint32_t(E_GENERIC))
            blinkAlarmLED(5, 100);
    }

}

void resourceTask(void* argument) {
    if(argument == NULL) {
        osThreadExit();
    }

    uint16_t pin = *(static_cast<uint16_t*>(argument));

    if(pin == 0) {
        osThreadExit();
    }

    Led led(LED_1_GPIO_Port, pin);

    for(;;) {
        // Wait for PushButton ISR to release a semaphore
        osSemaphoreAcquire(semTaskEvent_id, osWaitForever);
        osSemaphoreAcquire(semFreeResources_id, osWaitForever);

        // Execute for 5 seconds
        for(uint8_t i = 0; i < 10; ++i) {
            osDelay(500);
            led.toggle();
        }

        osSemaphoreRelease(semFreeResources_id);
    }
    
}