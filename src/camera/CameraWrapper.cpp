/*
 *  Created on: Jul 12, 2018
 *      Author: Luca Erbetta
 */

#include "CameraWrapper.h"
#include <gphoto2/gphoto2.h>
#include "debug.h"

CameraWrapper::CameraWrapper() : context(gp_context_new()) {}

CameraWrapper::~CameraWrapper() { freeCamera(); }

void CameraWrapper::freeCamera()
{
    if (camera != nullptr)
    {
        gp_camera_exit(camera, context);
        gp_camera_free(camera);
    }
}

bool CameraWrapper::connect()
{
    gp_camera_new(&camera);
    int result = gp_camera_init(camera, context);
    if (result == GP_OK)
    {
        serial = getSerialNumber();
        D(printf("Connected to camera: %s\n", serial.c_str()));
        return true;
    }
    return false;
}

bool CameraWrapper::isConnected()
{
    if (getSerialNumber() == serial)
    {
        return true;
    }

    if (camera != nullptr)
    {
        freeCamera();
        serial = NOT_A_GOOD_SERIAL;
    }
    return false;
}
string CameraWrapper::getSerialNumber()
{
    return getTextConfig("serialnumber");
}

string CameraWrapper::getTextConfig(string config_name)
{
    if (camera == nullptr)
    {
        return "";
    }
    CameraWidget* widget;
    int result = gp_camera_get_single_config(camera, config_name.c_str(),
                                             &widget, context);
    if (result != GP_OK)
    {
        return "";
    }

    const char* value;
    result = gp_widget_get_value(widget, &value);

    if (result != GP_OK)
    {
        return "";
    }

    return string(value);
}
