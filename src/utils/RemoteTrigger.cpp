/*
 *  Created on: Aug 10, 2018
 *      Author: Luca Erbetta
 */

#include "RemoteTrigger.h"

#include <chrono>
#include <thread>

using std::chrono::milliseconds;
using namespace std::this_thread;

void initTrigger()
{
    wiringPiSetup();
    pinMode(PIN_TRIGGER, OUTPUT);
    digitalWrite(PIN_TRIGGER, HIGH);
}

void trigger()
{
    digitalWrite(PIN_TRIGGER, LOW);
    sleep_for(milliseconds(100));
    digitalWrite(PIN_TRIGGER, HIGH);
}
