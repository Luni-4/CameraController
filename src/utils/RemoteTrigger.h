/*
 *  Created on: Aug 10, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_UTILS_REMOTETRIGGER_H
#define SRC_UTILS_REMOTETRIGGER_H

#include <wiringPi.h>

static const int PIN_TRIGGER = 25;

// Using an IR led
void initTrigger();
void trigger(unsigned int repetitions = 1);

// Using a pre build remote
void initRemote();
void remoteTrigger();


#endif /* SRC_UTILS_REMOTETRIGGER_H */
