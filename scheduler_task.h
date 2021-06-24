#ifndef SCHEDULER_TASK_
#define SCHEDULER_TASK_


#include "types.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern QueueHandle_t xUnscheduledCarsQueue;
extern QueueHandle_t xScheduledCarsQueue;

int car_comparator(const void* c1, const void* c2);
void  vSchedulerTask(void *pvParameters);


#endif