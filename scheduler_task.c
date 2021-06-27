#include <stdlib.h>
#include "console.h"

#include "types.h"
#include "FreeRTOS.h"
#include "semphr.h"


extern QueueHandle_t xUnscheduledCarsQueue;
extern QueueHandle_t xScheduledCarsQueue;

static int car_comparator(const void* c1, const void* c2) {
    Car_t* car_1 = (Car_t*)c1;
    Car_t* car_2 = (Car_t*)c2;

    return car_1->poll_time - car_2->poll_time;
}

void  vSchedulerTask(void *pvParameters) {

    Car_t xReadBuffer[4];
    BaseType_t xStatus;

    for( ;; ) {
        xStatus = xQueueReceive(xUnscheduledCarsQueue, &xReadBuffer[0], portMAX_DELAY);

        int buffer_fill = 1;
        for (int i = 1; i < 4; i++) {
            vTaskDelay(pdMS_TO_TICKS(50));

            if (uxQueueMessagesWaiting(xUnscheduledCarsQueue) == 0) {
                break;
            }
            else {
                xStatus = xQueueReceive(xUnscheduledCarsQueue, &xReadBuffer[i], 0);
                buffer_fill++;
            }
        }

        if (buffer_fill == 1) {
            console_print("Single car received, pushed immediately...\r\n");
            xReadBuffer[0].is_scheduled = true;
            xStatus = xQueueSendToBack( xScheduledCarsQueue, &xReadBuffer[0], 0);
        }
        else {
            qsort(xReadBuffer, buffer_fill, sizeof(Car_t), car_comparator);

            for (int i = 0; i < buffer_fill; i++) {
                console_print("Car %d schduled\r\n", xReadBuffer[i].poll_time);
                xReadBuffer[i].is_scheduled = true;
                xStatus = xQueueSendToBack( xScheduledCarsQueue, &xReadBuffer[i], 0);
            }
            console_print("Many cars received, all pushed...\r\n");
        }
    }
}