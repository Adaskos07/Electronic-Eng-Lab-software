#include <stdlib.h>
#include "console.h"

#include "types.h"
#include "FreeRTOS.h"
#include "semphr.h"


extern QueueHandle_t xUnscheduledCarsQueue;

void  vRedlineCrossedTask(void *pvParameters) {

    BaseType_t xStatus;
    TickType_t delay = ((rc_settings_t*)pvParameters)->delay;
    int from_lane = ((rc_settings_t*)pvParameters)->from_lane;

    for ( ;; ) {
        volatile TickType_t call_time = pdMS_TO_TICKS(xTaskGetTickCount());
        Car_t car = {
            .id = (rand() % 10000) + 1,
            .direction =  3*from_lane + (rand() % 3),
            .poll_time = call_time,
            .is_scheduled = false
        };
        xStatus = xQueueSendToBack( xUnscheduledCarsQueue, &car, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}