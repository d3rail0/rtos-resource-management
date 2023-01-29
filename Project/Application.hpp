#pragma once

#include <cmsis_os2.h>
#include "components/Led.hpp"
#include "main.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdio.h"
#include <cstring>

#include "Error.hpp"
#include "ErrorDefs.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
    void AppMain();
    extern void TempControllerMain();

    void signalForError(Error error);
#ifdef __cplusplus
}
#endif

