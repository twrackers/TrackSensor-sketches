#ifndef _TRACK_SENSOR__CONTROLLER__H_
#define _TRACK_SENSOR__CONTROLLER__H_

#include <Arduino.h>

typedef void (*CALLBACK0)();
typedef void (*CALLBACK1)(int);

const byte LOOK_AT_ME = 2;  // interrupt line to I2C central

#endif
