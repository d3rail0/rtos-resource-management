#pragma once

#include "Error.hpp"

// Defining all errors for later use with error Look Up Table (LUT)
#define DEF_ERR static inline const Error
DEF_ERR E_OK("Success.");
DEF_ERR E_GENERIC("Something went wrong.");
DEF_ERR E_WRITE_FAILED("UART write failed the deadline.");
DEF_ERR E_NO_MORE_RESOURCES("All resources have been exhausted. Please wait...");
DEF_ERR E_INVALID_RES_SEM("Resource semaphore identifier is invalid.");
DEF_ERR E_TEMP_TOO_HIGH("Temperature is too high.");
DEF_ERR E_TEMP_TOO_LOW("Temperature is too low.");
DEF_ERR E_TEMP_CTRL_FAILED_START("Temperature controller failed to start.");
DEF_ERR E_TEMP_NOT_READ_ON_TIME("[CRITICAL] Failed to sample temperature on time.");
DEF_ERR E_TEMP_MQ_FULL("Temperature message queue is full.");