/*
 *******************************************************************************
 * NOTE: Linux will not be running the FreeRTOS demo threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Linux port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Linux
 * port for further information:
 * https://freertos.org/FreeRTOS-simulator-for-Linux.html
 *
 *******************************************************************************
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "console.h"

#include "scheduler_task.h"

volatile uint32_t ulIdleCycleCount = 0UL;

StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

QueueHandle_t xQueue;

void vContinousTask( void *pvParameters );
void vPeriodicTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );
static void vSenderTask( void *pvParameters );

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize );

int main( void )
{
    /* Perform any hardware setup necessary. */
    // prvSetupHardware();
    // /* --- APPLICATION TASKS CAN BE CREATED HERE --- */
    // /* Start the created tasks running. */
    console_init();

    xQueue = xQueueCreate(5, sizeof(int32_t));

    // xTaskCreate(vContinousTask, "Task_1", 1000, "CONTINOUS TASK 1", 1, NULL);
    // xTaskCreate(vContinousTask, "Task_2", 1000, "CONTINOUS TASK 2", 1, NULL);
    xTaskCreate(vSenderTask, "Sender_1", 1000, (void *)100, 1, NULL);
    xTaskCreate(vSenderTask, "Sender_2", 1000, (void*)200, 1, NULL);
    xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, 2, NULL);
    // xTaskCreate(vPeriodicTask, "Task_3", 1000, "PERIODIC TASK 3", 10, NULL);

    vTaskStartScheduler();
    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
    for( ;; );
    return 0;
}

static void vSenderTask( void *pvParameters )
{
    int32_t lValueToSend;
    BaseType_t xStatus;
    
    lValueToSend = ( int32_t ) pvParameters;
    // UBaseType_t uxPriority;

    for( ;; )
    {
        xStatus = xQueueSendToBack( xQueue, &lValueToSend, 0 );

        if (xStatus != pdPASS) {
            console_print("COULD NOT SEND");
        }
        /* Delay for a period. */
        // vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
}

static void vReceiverTask( void *pvParameters )
{
    int32_t lReceivedValue;
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    // volatile uint32_t ul; /* volatile to ensure ul is not optimized away. */
    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        if (uxQueueMessagesWaiting(xQueue) != 0)
        {
            console_print("Queue should have been empty\r\n");
        }

        xStatus = xQueueReceive(xQueue, &lReceivedValue, xTicksToWait);

        if (xStatus == pdPASS)
        {
            console_print("Received = %d\r\n", lReceivedValue);
        }
        else
        {
            console_print("Could not receive from the queue.\r\n");
        }
    }
}

void vPeriodicTask( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xDelay = pdMS_TO_TICKS(5);
    const char *pcTaskName = (char *)pvParameters;
    UBaseType_t uxPriority;
    // volatile uint32_t ul; /* volatile to ensure ul is not optimized away. */
    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        uxPriority = uxTaskPriorityGet( NULL );

        console_print( "%s  %d\n", pcTaskName, uxPriority);
        /* Delay for a period. */
        // vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
}

void vContinousTask( void *pvParameters )
{
    // TickType_t xLastWakeTime = xTaskGetTickCount();
    const char *pcTaskName = (char *)pvParameters;
    // UBaseType_t uxPriority;
    // // volatile uint32_t ul;
    // uxPriority = uxTaskPriorityGet( NULL );
    for( ;; )
    {
    /* Print out the name of this task. */
        // vPrintString( pcTaskName );
        // vprintf("%s\n", pcTaskName );
        console_print( "%s\n", pcTaskName);
        // vTaskPrioritySet( NULL, ( uxPriority + 1 ) );
        /* Delay for a period. */
        // vTaskDelayUntil(&xLastWakeTime ,pdMS_TO_TICKS(2000));
        // vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

// void vApplicationIdleHook(void)
// {
//     ulIdleCycleCount++;
// }

