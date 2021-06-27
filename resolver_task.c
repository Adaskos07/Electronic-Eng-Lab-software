#include <stdlib.h>

#include "types.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "console.h"


extern QueueHandle_t xScheduledCarsQueue;


void vResolverTask(void *pvParameters) {

    Car_t xReadBuffer[2];
    BaseType_t xStatus;
    bool last_stoped = false;

    for ( ;; ) {
        if (last_stoped) {
            xReadBuffer[0] = xReadBuffer[1];
            last_stoped = false;
        }
        else {
            xStatus = xQueueReceive(xScheduledCarsQueue, &xReadBuffer[0], portMAX_DELAY);
        }

        xStatus = xQueueReceive(xScheduledCarsQueue, &xReadBuffer[1], pdMS_TO_TICKS(50));
        if (xStatus == pdFALSE) {
            console_print("Car id: %d let through\n", xReadBuffer[0].id);
        }
        else {
            const int ms_threshold = 300;
            if (abs(xReadBuffer[0].poll_time - xReadBuffer[1].poll_time) < ms_threshold) {
                console_print("Car id: %d let through\n", xReadBuffer[0].id);
                console_print("Car id: %d stoped\n", xReadBuffer[1].id);
                last_stoped = true;
            }
            else {
                console_print(
                    "Car id: %d and car id: %d let through\n",
                    xReadBuffer[0].id, xReadBuffer[1].id
                    );
            }
        }
    }
}