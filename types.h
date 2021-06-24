#ifndef TYPES_
#define TYPES_


typedef enum Direction {
    NW = 0,
    NS = 1,
    NE = 2,
    WS = 3,
    WE = 4,
    WN = 5,
    SE = 6,
    SN = 7,
    SW = 8,
    EN = 9,
    EW = 10,
    ES = 11
} Direction_t;

typedef struct Car {
    int id;
    Direction_t direction;
    double poll_time;
} Car_t;

#endif