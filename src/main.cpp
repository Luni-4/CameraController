#include "CameraWrapper.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>
#include "circular_buffer.h"
#include "communication/TCPServer.h"
#include "functions/intervalometer.h"
#include "functions/sequencer.h"
#include "logger.h"

using namespace std::chrono;
using namespace std::this_thread;

using std::chrono::milliseconds;
using std::chrono::seconds;

Logger log;

std::ofstream ofs("cameracontroller_log.txt", std::ofstream::out);

void printBuf(uint8_t* buf, size_t s)
{
    for (size_t i = 0; i < s; i++)
    {
        printf(" %d ", buf[i]);
    }

    printf("\n");
}
int main()
{
    TCPServer s{};
    TCPStream tcps{&s};

    log.addStream(&tcps, LOG_INFO);

    s.start();

    sleep_for(seconds(3));

    while (true)
    {
        // tcps.flush();
        string str = "I'm awake!\n";

        s.sendData((const uint8_t*)str.c_str(), str.length());
        sleep_for(seconds(5));
    }

    ofs.close();
    return 0;
}
