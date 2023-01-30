#include "TemperatureControl.hpp"

// Max elements that can be buffered in the message queue
const uint8_t MQ_BUFFER_SIZE = 8;
const float MAX_VOLTAGE      = 5.0f;
const float ADC_RESOLUTION   = 4096.f; 

// Minimal and maximal temperatures allowed for the system
const float MIN_TEMP = 12.0f;
const float MAX_TEMP = 25.0f;

osMessageQueueId_t msgQueue_id;
osThreadAttr_t tempTaskAttrs  = {};
bool isResourceTasksSuspended = false;

__NO_RETURN void tempReaderTask(void*);
__NO_RETURN void tempProcessorTask(void*);

void TempControllerMain() {

    msgQueue_id = osMessageQueueNew(MQ_BUFFER_SIZE, sizeof(float), NULL);
    if (msgQueue_id == NULL) {
        signalForError(E_TEMP_CTRL_FAILED_START);
        return;
    }
    
    tempTaskAttrs.name       = "TEMP_TASK";
    tempTaskAttrs.stack_size = 128 * 4;
    tempTaskAttrs.priority   = osPriorityHigh;

    // Start producer and consumer threads
    osThreadNew(tempReaderTask, NULL, &tempTaskAttrs);
    osThreadNew(tempProcessorTask, NULL, &tempTaskAttrs);

}

void stopAllResourceTasks() {
    if (isResourceTasksSuspended)
        // Already suspended.
        return;

    for(size_t i = sizeof(resourceThreads)/sizeof(osThreadId_t); i > 0; --i)
        osThreadSuspend(resourceThreads[i-1]);

    isResourceTasksSuspended = true;
}

void resumeAllResourcesTasks() {
    if (!isResourceTasksSuspended)
        // Tasks were not suspended.
        return;

    for(size_t i = sizeof(resourceThreads)/sizeof(osThreadId_t); i > 0; --i)
        osThreadResume(resourceThreads[i-1]);

    isResourceTasksSuspended = false;
}

uint32_t getConvertedADCValue() {
    static uint32_t adcReading { 0 };

    HAL_ADC_Start(&hadc1); // Start conversion

    // Wait for end of the conversion and get value
    if (HAL_ADC_PollForConversion(&hadc1, 250) == HAL_OK) {
        adcReading = HAL_ADC_GetValue(&hadc1);
    } else {
        signalForError(E_TEMP_NOT_READ_ON_TIME);
    }
    
    HAL_ADC_Stop(&hadc1); // Stop conversion

    return adcReading;
}

void tempReaderTask(void*) {

    uint32_t bufferedReading[MQ_BUFFER_SIZE];
    uint16_t currElement { 0 };
    
    // Call to avoid wrong reading at the start of simulation.
    osDelay(1500);

    for(;;) {
        
        bufferedReading[currElement] = getConvertedADCValue();

        // Push the reading to a queue
        if(osMessageQueuePut(msgQueue_id, &bufferedReading[currElement], 0U, 0U) != osOK) {
            signalForError(E_TEMP_MQ_FULL);
        }

        currElement = (currElement+1) % MQ_BUFFER_SIZE;

        osDelay(250);
    }

}

uint64_t GetTick(void) {
    static uint32_t tick_h {0U};
    static uint32_t tick_l {0U};
    uint32_t tick {osKernelGetSysTimerCount()};

    if (tick < tick_l) {
        tick_h++;
    }

    tick_l = tick;
    return ((static_cast<uint64_t>(tick_h) << 32) | tick_l);
}

void tempProcessorTask(void*) {
    char     readingMessage[32];
    float    temp;
    uint32_t adcReading;

    // Since compiling this program with printf() floating-point support
    // would end up increasing the FLASH memory size and overflowing, 
    // printing float will require a different approach
    char    tempSign[2] = " "; // can be ' ' or '-'
    int32_t tempInt     { 0 };
    int32_t fractInt    { 0 };
    float   tempFrac    { 0.f };
    float   tmpTemp     { 0.f };

    // Wait for producer to send at least a single reading
    // before starting the main consumer loop.
    // Note: This will dismiss the first reading completely
    // which in practice shouldn't cause problems.
    osMessageQueueGet(msgQueue_id, &adcReading, NULL, osWaitForever);

    uint64_t tick {0};
  
    // Calculating 3s timeout in system timer ticks    
    const uint64_t timeout { 3U * osKernelGetSysTimerFreq() };

    for(;;) {
        if(osMessageQueueGet(msgQueue_id, &adcReading, NULL, 250U)) {
            signalForError(E_TEMP_NOT_READ_ON_TIME);
            osDelay(250);
            continue;
        }

        temp = MAX_VOLTAGE * adcReading * 100 / ADC_RESOLUTION;

        // Decompose float value
        tempSign[0] = (temp < 0) ? '-' : ' ';        // Resolve sign
        tmpTemp     = (temp < 0) ? -temp : temp;     // Make tmpTemp positive
        tempInt     = static_cast<int32_t>(tmpTemp); // Extract integer part from float
        tempFrac    = tmpTemp - tempInt;             // Extract fractional part from float
        fractInt    = static_cast<int32_t>(trunc(tempFrac*10000)); // Turn fractional part into an integer (with 4-digit precision)

        sprintf(readingMessage, "Temperature: %s%ld.%04ld *C\r\n", tempSign, tempInt, fractInt);
        if(HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(readingMessage), static_cast<uint16_t>(std::strlen(readingMessage)), 50U) != HAL_OK) {
            signalForError(E_WRITE_FAILED);
        }

        if(temp < MIN_TEMP) {
            signalForError(E_TEMP_TOO_LOW);
            tick = GetTick();
            stopAllResourceTasks();
        } else if (temp > MAX_TEMP) {
            signalForError(E_TEMP_TOO_HIGH);
            tick = GetTick();
            stopAllResourceTasks();
        } else if (isResourceTasksSuspended) {
            
            // Wait for 
            if(GetTick() - tick > timeout) {
                sprintf(readingMessage, "Resuming all threads...\r\n");

                if(HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(readingMessage), static_cast<uint16_t>(std::strlen(readingMessage)), 250U) != HAL_OK) {
                    signalForError(E_WRITE_FAILED);
                }

                resumeAllResourcesTasks();
            }

        }    

        osDelay(250);
    }

}