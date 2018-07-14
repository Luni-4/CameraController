#include "CameraWrapper.h"

#include <chrono>
#include <cstdio>
#include <thread>
#include "functions/intervalometer.h"

using namespace std::chrono;
using namespace std::this_thread;

using std::chrono::milliseconds;

int main()
{
    // CameraWrapper& w = CameraWrapper::getInstance();

    Intervalometer i(5, 5 * 1000 * 1000);

    printf("Starting...\n");
    i.start();

    sleep_for(milliseconds(30 * 1000));
    printf("Aborting...\n");
    i.abort();

    sleep_for(milliseconds(5 * 1000));
    return 0;
}
