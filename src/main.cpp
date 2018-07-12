#include "CameraWrapper.h"

int main()
{
    CameraWrapper& w = CameraWrapper::getInstance();

    bool c = w.isConnected();
    printf("Connected: %s\n", c ? "true" : "false");
    w.connect();

    c = w.isConnected();
    printf("Connected: %s\n", c ? "true" : "false");
    return 0;
}
