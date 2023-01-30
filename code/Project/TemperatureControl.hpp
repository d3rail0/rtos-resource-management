#pragma once

#include "cmsis_os2.h"
#include "components/Led.hpp"
#include "ErrorDefs.hpp"
#include "main.h"
#include "stdio.h"
#include "math.h"
#include <cstring>

#ifdef __cplusplus
extern "C"
{
#endif

    void TempControllerMain();

    // All resource threads which cause
    // the device temperature to rise
    extern osThreadId_t resourceThreads[3];
    extern void signalForError(Error error);

#ifdef __cplusplus
}
#endif