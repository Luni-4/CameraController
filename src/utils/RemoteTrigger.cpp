/*
 *  Created on: Aug 10, 2018
 *      Author: Luca Erbetta
 */

#include "RemoteTrigger.h"

#include <chrono>
#include <thread>

using std::chrono::milliseconds;
using namespace std::this_thread;

void initRemote()
{
    wiringPiSetup();
    pinMode(PIN_TRIGGER, OUTPUT);
    digitalWrite(PIN_TRIGGER, HIGH);
}

void remoteTrigger()
{
    digitalWrite(PIN_TRIGGER, LOW);
    sleep_for(milliseconds(100));
    digitalWrite(PIN_TRIGGER, HIGH);
}

void initTrigger()
{
    wiringPiSetup();
    pinMode(PIN_TRIGGER, OUTPUT);
    digitalWrite(PIN_TRIGGER, LOW);
}

void trigger(unsigned int repetitions)
{
    for (unsigned int r = 0; r < repetitions; r++)
    {
        for (int j = 0; j < 2; j++)
        {
            int i;
            for (i = 0; i < 76; i++)
            {
                digitalWrite(PIN_TRIGGER, HIGH);
                delayMicroseconds(7);
                digitalWrite(PIN_TRIGGER, LOW);
                delayMicroseconds(7);
            }
            delay(27);
            delayMicroseconds(810);

            for (i = 0; i < 16; i++)
            {
                digitalWrite(PIN_TRIGGER, HIGH);
                delayMicroseconds(7);
                digitalWrite(PIN_TRIGGER, LOW);
                delayMicroseconds(7);
            }
            delayMicroseconds(1540);

            for (i = 0; i < 16; i++)
            {
                digitalWrite(PIN_TRIGGER, HIGH);
                delayMicroseconds(7);
                digitalWrite(PIN_TRIGGER, LOW);
                delayMicroseconds(7);
            }

            delayMicroseconds(3545);

            for (i = 0; i < 16; i++)
            {
                digitalWrite(PIN_TRIGGER, HIGH);
                delayMicroseconds(7);
                digitalWrite(PIN_TRIGGER, LOW);
                delayMicroseconds(7);
            }
            if (j == 0)
            {
                delay(63);
                delayMicroseconds(200);
            }
        }
        if(r < repetitions - 1)
        {
            sleep_for(milliseconds(80));
        }
    }
}