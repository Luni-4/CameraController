#include "CameraWrapper.h"

#include <chrono>
#include <thread>

using namespace std::chrono;
using namespace std::this_thread;

int main()
{
    CameraWrapper& w = CameraWrapper::getInstance();

    w.connect();

    printf("%s\n", w.getTextConfigValue("shutterspeed").c_str());

    bool success = w.setExposureTime(0);
    if (success)
    {
        printf("Kmaronn\n");
    }
    printf("%s\n", w.getTextConfigValue("shutterspeed").c_str());

    return 0;
}
