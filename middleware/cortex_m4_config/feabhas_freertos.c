#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_device.h"

#ifdef DEBUG
#include <stdio.h>
#endif


// Hooks for better diagnostics

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
    (void) xTask;
#ifdef DEBUG
    printf("stack overflow in task %s\n", pcTaskName);
#endif
    configASSERT(0);
}

void vApplicationMallocFailedHook( void )
{
#ifdef DEBUG
    printf("malloc failed\n");
#endif
    configASSERT(0);
}


void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
    (void) ulLine;
    (void) pcFileName;
#ifdef DEBUG
    printf("assert failed %s:%ld\n", pcFileName, ulLine);
#else
    for(;;) {__WFE(void);}
#endif
}
