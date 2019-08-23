/*
 *  Created on: Aug 8, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_FUNCTIONS_CAMERAFUNCTION_H
#define SRC_FUNCTIONS_CAMERAFUNCTION_H

#include "logger.h"

#include <atomic>
#include <thread>
#include <future>

using std::atomic_bool;
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
    CameraFunction(string download_folder)
        : camera(CameraWrapper::getInstance()), download_folder(download_folder)
    {
    }

    virtual ~CameraFunction() {}

    virtual FunctionID getID() = 0;

    virtual bool start() = 0;
    virtual void abort() = 0;

    virtual void downloadAfterExposure(bool value)
    {
        download_after_exposure = value;
    };

    virtual bool downloadAfterExposure() { return download_after_exposure; };

    virtual bool isStarted()  = 0;
    virtual bool isFinished() = 0;

    virtual void testCapture()
    {
        if (!isOperating())
        {
            if (!connectCamera())
            {
                Log.e("Cannot test capture, camera not connected.");
                return;
            }
            Log.i("Test capture...");
            std::async(&CameraFunction::doTestCapture, this);
        }
        else
        {
            Log.w("Cannot test while operating.");
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
    bool isTesting() { return testing; }

    CameraWrapper& camera;

    string download_folder;

    virtual void doTestCapture() = 0;
    bool testing = false;
private:

    atomic_bool download_after_exposure{};
};

#endif /* SRC_FUNCTIONS_CAMERAFUNCTION_H */
