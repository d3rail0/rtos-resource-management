#pragma once

#include "Error.hpp"

// Defining all errors for later use with error Look Up Table (LUT)
#define DEF_ERR static inline const Error
DEF_ERR E_OK("Success.");
DEF_ERR E_GENERIC("Something went wrong.");
DEF_ERR E_NO_MORE_RESOURCES("All resources have been exhausted. Please wait...");
DEF_ERR E_INVALID_RES_SEM("Resource semaphore identifier is invalid.");