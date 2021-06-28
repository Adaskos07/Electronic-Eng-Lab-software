/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>


/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* FreeRTOS TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"


/* FreeRTOS coreMQTT-Agent  includes. */
// #include "core_mqtt_config.h"
// #include "core_mqtt_agent.h"

/* source includes */
#include "console.h"
#include "types.h"

#include "scheduler_task.h"
#include "resolver_task.h"
#include "redline_crossed_task.h"

StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
UBaseType_t uxRand( void );
void prvMiscInitialisation( void );
void prvSRand( UBaseType_t ulSeed );

/* The default IP and MAC address used by the demo.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
static const uint8_t ucIPAddress[ 4 ] = {
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};
static const uint8_t ucNetMask[ 4 ] = {
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};
static const uint8_t ucGatewayAddress[ 4 ] = { 
    configGATEWAY_ADDR0,
    configGATEWAY_ADDR1,
    configGATEWAY_ADDR2,
    configGATEWAY_ADDR3
};
static const uint8_t ucDNSServerAddress[ 4 ] = {
    configDNS_SERVER_ADDR0,
    configDNS_SERVER_ADDR1,
    configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3
};

/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition for information on how to configure
 * the real network connection to use. */
const uint8_t ucMACAddress[ 6 ] = {
    configMAC_ADDR0,
    configMAC_ADDR1,
    configMAC_ADDR2,
    configMAC_ADDR3,
    configMAC_ADDR4,
    configMAC_ADDR5
};

static UBaseType_t ulNextRand;

QueueHandle_t xUnscheduledCarsQueue;
QueueHandle_t xScheduledCarsQueue;


void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize );


int main( void )
{
    srand((unsigned int)time(NULL));

    rc_settings_t s1 = {3050, 0};
    rc_settings_t s2 = {3150, 2};
    rc_settings_t s3 = {3250, 3};
    rc_settings_t s4 = {3250, 4};

    console_init();
    // prvMiscInitialisation();
    // FreeRTOS_IPInit( ucIPAddress,
    //                 ucNetMask,
    //                 ucGatewayAddress,
    //                 ucDNSServerAddress,
    //                 ucMACAddress );


    xUnscheduledCarsQueue = xQueueCreate(10, sizeof(Car_t));
    xScheduledCarsQueue = xQueueCreate(10, sizeof(Car_t));
    // xQueue = xQueueCreate(10, sizeof(Car_t));

    xTaskCreate(vRedlineCrossedTask, "Redline_N", 1000, (void *)&s1, 1, NULL);
    xTaskCreate(vRedlineCrossedTask, "Redline_W", 1000, (void *)&s2, 1, NULL);
    xTaskCreate(vRedlineCrossedTask, "Redline_S", 1000, (void *)&s3, 1, NULL);
    xTaskCreate(vRedlineCrossedTask, "Redline_E", 1000, (void *)&s4, 1, NULL);

    xTaskCreate(vSchedulerTask, "Scheduler", 1000, NULL, 2, NULL);
    xTaskCreate(vResolverTask, "Resolver", 1000, NULL, 2, NULL);


    vTaskStartScheduler();

    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
    for( ;; );
    return 0;
}


/* ----------------------------------------------------- */
UBaseType_t uxRand( void )
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    /* Utility function to generate a pseudo random number. */

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

void prvSRand( UBaseType_t ulSeed )
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}

void prvMiscInitialisation( void )
{
    time_t xTimeNow;

    /* Seed the random number generator. */
    time( &xTimeNow );
    prvSRand( ( uint32_t ) xTimeNow );
}

extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    return uxRand();
}

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *( pulNumber ) = uxRand();
    return pdTRUE;
}

/* ----------------------------------------------- */
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

void vLoggingPrintf( const char *pcFormat,
                     ... )
{
    va_list arg;
    va_start( arg, pcFormat );
    vprintf( pcFormat, arg );
    va_end( arg );
}