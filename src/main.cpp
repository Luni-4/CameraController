#include "CameraWrapper.h"

#include <stdlib.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>
#include "circular_buffer.h"
#include "commands/Commands.h"
#include "communication/MessageDecoder.h"
#include "communication/TCPStream.h"
#include "functions/camerafunction.h"
#include "functions/intervalometer.h"
#include "functions/sequencer.h"
#include "logger.h"
#include "utils/RemoteTrigger.h"

using namespace std::chrono;
using namespace std::this_thread;

using std::chrono::milliseconds;
using std::chrono::seconds;

Logger Log;

CameraWrapper* camera;

TCPServer* server;
MessageHandler* msghandler;
MessageDecoder* decoder;
MessageEncoder* encoder;

NetStream* netstream;
std::ofstream ofs("cameracontroller_log.txt", std::ofstream::out);

CameraFunction* activeFunction = nullptr;

class CommandHandler : public OnMessageReceivedListener
{
    void onCommandReceived(const Command& command) override
    {
        switch (command.cmd_id)
        {
            case CMD_ID_REBOOT:
                Log.i("Received reboot command");
                sleep_for(seconds(5));
                system("sudo reboot");
                break;
            case CMD_ID_SHUTDOWN:
                Log.i("Received shutdown command");
                sleep_for(seconds(5));
                system("sudo halt");
                break;
            case CMD_ID_CAMERA_TEST_CONNECTION:
                if (camera->isConnected())
                {
                    Log.i("Camera connected.");
                }
                else
                {
                    Log.w("Camera not connected.");
                    break;
                }
                if (camera->isResponsive())
                {
                    Log.i("Camera responsive.");
                }
                else
                {
                    Log.w("Camera not responsive.");
                    break;
                }
                break;
            case CMD_ID_CAMERA_RECONNECT:
                Log.i("Disconnecting...");
                camera->disconnect();
                sleep_for(seconds(5));
                Log.i("Reconnecting...");
                camera->connect();
                break;
            case CMD_ID_SEQUENCERSETUP:
            {
                // Cast
                const SequencerSetupCommand& cmd =
                    reinterpret_cast<const SequencerSetupCommand&>(command);

                if (activeFunction != nullptr)
                {
                    if (!activeFunction->isOperating())
                    {
                        // If finished, delete old sequencer
                        delete activeFunction;
                        // Create a new sequencer
                        activeFunction = new Sequencer(
                            cmd.num_exposures, cmd.exp_time, cmd.download);
                    }
                    else
                    {
                        Log.e(
                                "Cannot configure sequencer: Function "
                                "already running.");
                    }
                }
                else
                {
                    // No function configured
                    activeFunction = new Sequencer(
                        cmd.num_exposures, cmd.exp_time, cmd.download);
                }

                break;
            }
            case CMD_ID_INTERVALOMETERSETUP:
            {
                Log.d("Received intervalometer config");
                // Cast
                const IntervalometerSetupCommand& cmd =
                    reinterpret_cast<const IntervalometerSetupCommand&>(command);

                if (activeFunction != nullptr)
                {
                    if (!activeFunction->isOperating())
                    {
                        // If finished, delete old sequencer
                        delete activeFunction;
                        // Create a new sequencer
                        activeFunction = new Intervalometer(
                            cmd.num_exposures, cmd.interval, cmd.exp_time, cmd.download);
                    }
                    else
                    {
                        Log.e(
                                "Cannot configure intervalometer: Function "
                                "already running.");
                    }
                }
                else
                {
                    // No function configured
                    activeFunction = new Intervalometer(
                        cmd.num_exposures, cmd.interval, cmd.exp_time, cmd.download);
                }

                break;
            }
            case CMD_ID_DOWNLOAD_AFTER_EXPOSURE:
            {
                const DownloadAfterExposureCommand& cmd =
                    reinterpret_cast<const DownloadAfterExposureCommand&>(
                        command);
                if (activeFunction != nullptr)
                {
                    activeFunction->downloadAfterExposure(cmd.download);
                    Log.i("Downloading after exposure: %s",
                          cmd.download ? "yes" : "no");
                }
                else
                {
                    Log.w("No function configured.");
                }
                break;
            }
            case CMD_ID_FUNCTION_TEST_CAPTURE:
            {
                if (activeFunction != nullptr)
                {
                    activeFunction->testCapture();
                }
                else if (activeFunction == nullptr)
                {
                    Log.w("No function configured.");
                }
                break;
            }
            case CMD_ID_FUNCTIONSTART:
            {
                if (activeFunction != nullptr)
                {
                    activeFunction->start();
                }
                else if (activeFunction == nullptr)
                {
                    Log.w("No function configured.");
                }
                break;
            }

            case CMD_ID_FUNCTIONSTOP:
            {
                if (activeFunction != nullptr && activeFunction->isStarted())
                {
                    activeFunction->abort();
                }
                else if (activeFunction == nullptr)
                {
                    Log.w("No function configured.");
                }
            }
        }
    }
};

CommandHandler* cmdhandler;

void init()
{
    camera     = &CameraWrapper::getInstance();
    cmdhandler = new CommandHandler();
    msghandler = new MessageHandler(*cmdhandler);
    decoder    = new MessageDecoder(*msghandler);
    server     = new TCPServer(*decoder);

    encoder   = new MessageEncoder(server);
    netstream = new NetStream(encoder);

    // camera->connect();
}

int main()
{
    piHiPri(20);
    Log.addStream(&ofs, LOG_DEBUG);
    initTrigger();
    init();

    Log.addStream(netstream, LOG_INFO);

    server->start();
    sleep_for(seconds(1));

    while (true)
    {
        Log.i("Heartbeat");
        sleep_for(seconds(60));
    }

    ofs.close();

    return 0;
}
