/*
 *  Created on: Aug 10, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_UTILS_REMOTETRIGGER_H
#define SRC_UTILS_REMOTETRIGGER_H

#include <wiringPi.h>

static const int PIN_TRIGGER = 25;

void initTrigger();
void trigger();

#endif /* SRC_UTILS_REMOTETRIGGER_H */
