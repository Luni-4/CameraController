/*
 *  Created on: Aug 8, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_FUNCTIONS_CAMERAFUNCTION_H
#define SRC_FUNCTIONS_CAMERAFUNCTION_H

#include "logger.h"

#include <thread>
using std::thread;

static const char* DEFAULT_DOWNLOAD_FOLDER = "/home/pi/CCCaptures/";

enum class FunctionID
{
    SEQUENCER,
    INTERVALOMETER
};

class CameraFunction
{
public:
    CameraFunction() : camera(CameraWrapper::getInstance()) {}

    virtual ~CameraFunction() {}

    virtual FunctionID getID() = 0;

    virtual bool start() = 0;
    virtual void abort() = 0;

    virtual void downloadAfterExposure(bool value) = 0;
    virtual bool downloadAfterExposure()           = 0;

    virtual bool isStarted()  = 0;
    virtual bool isFinished() = 0;

    virtual void testCapture()
    {
        if (!isOperating())
        {
            if (!connectCamera())
            {
                Log.e("Cannot test capture");
                return;
            }
            Log.i("Test capture...");
            thread t(&CameraFunction::doTestCapture, this);
            t.detach();
        }
    }

    bool connectCamera()
    {
        if (!camera.isConnected())
        {
            Log.w("Camera not connected!");
            if (camera.connect())
            {
                Log.i("Succesfully connected to camera");
            }
            else
            {
                Log.e("Failed connecting to camera!");
                return false;
            }
        }
        else if (!camera.isResponsive())
        {
            Log.e("Camera is not responsive!");
            return false;
        }
        return true;
    }

    bool isOperating() { return (isStarted() && !isFinished()) || testing; }

protected:
    virtual bool capture() = 0;
    bool isTesting() { return testing; }

    CameraWrapper& camera;

private:
    void doTestCapture()
    {
        testing = true;
        if (capture())
        {
            Log.i("Test capture completed successfully");
        }
        else
        {
            Log.i("Test capture finished with errors.");
        }
        testing = false;
    }
    bool testing = false;
};

#endif /* SRC_FUNCTIONS_CAMERAFUNCTION_H */
