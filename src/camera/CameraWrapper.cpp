/*
 *  Created on: Jul 12, 2018
 *      Author: Luca Erbetta
 */

#include "CameraWrapper.h"
#include <gphoto2/gphoto2.h>
#include <cmath>
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

bool CameraWrapper::capture()
{
    CameraFilePath path;
    int result = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &path, context);

    if (result == GP_OK)
    {
        D(printf("Captured to: %s/%s\n", path.folder, path.name));
        return true;
    }
    return false;
}

string CameraWrapper::getSerialNumber()
{
    return getTextConfigValue(CONFIG_SERIAL_NUMBER);
}

string CameraWrapper::getTextConfigValue(string config_name)
{
    string value_str = "";
    CameraWidget* widget;

    int result = gp_camera_get_single_config(camera, config_name.c_str(),
                                             &widget, context);
    if (result != GP_OK)
    {
        D(printf("Couldn't get single config (%s): %d\n", config_name.c_str(),
                 result));
        goto out;
    }

    const char* value;
    result = gp_widget_get_value(widget, &value);

    if (result != GP_OK)
    {
        D(printf("Couldn't get widget value (%s): %d\n", config_name.c_str(),
                 result));
        goto out;
    }
    value_str = string(value);

out:
    gp_widget_free(widget);
    return value_str;
}

bool CameraWrapper::setConfigValue(string config_name, string value)
{
    bool success = false;

    CameraWidget* widget;
    int result = gp_camera_get_single_config(camera, config_name.c_str(),
                                             &widget, context);
    if (result != GP_OK)
    {
        D(printf("Couldn't get single config (%s): %d\n", config_name.c_str(),
                 result));
        goto out;
    }
    CameraWidgetType type;
    result = gp_widget_get_type(widget, &type);
    if (result != GP_OK)
    {
        D(printf("Couldn't get config type (%s): %d\n", config_name.c_str(),
                 result));
        goto out;
    }

    switch (type)
    {
        case GP_WIDGET_MENU:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_TEXT:
        {
            result = gp_widget_set_value(widget, value.c_str());
            if (result != GP_OK)
            {
                D(printf("Couldn't set config value (%s): %d\n",
                         config_name.c_str(), result));
                goto out;
            }
            // Finally set the config on the camera
            result = gp_camera_set_single_config(camera, config_name.c_str(),
                                                 widget, context);
            if (result != GP_OK)
            {
                D(printf("Couldn't set config on camera (%s): %d\n",
                         config_name.c_str(), result));
                goto out;
            }
            success = true;
            goto out;
            break;
        }

        default:
            D(printf("Bad widget type (%s): %d\n", config_name.c_str(), type));
            goto out;
    }

out:
    gp_widget_free(widget);
    return success;
}

int CameraWrapper::getCurrentExposureTime()
{
    string exp = getTextConfigValue(CONFIG_EXPOSURE_TIME);
    if (exp != "")
    {
        return exposureTimeFromString(exp);
    }
    return -1;
}

bool CameraWrapper::setExposureTime(int index)
{
    vector<string> exposures = listConfigChoices(CONFIG_EXPOSURE_TIME);
    if (index >= 0 && (unsigned)index < exposures.size())
    {
        return setConfigValue(CONFIG_EXPOSURE_TIME, exposures[index]);
    }
    return false;
}

vector<string> CameraWrapper::listConfigChoices(string config_name)
{
    vector<string> choices;
    CameraWidget* widget;
    int result = gp_camera_get_single_config(camera, config_name.c_str(),
                                             &widget, context);
    int n;

    if (result != GP_OK)
    {
        D(printf("Couldn't get single config (%s): %d", config_name.c_str(),
                 result));
        goto out;
    }

    n = gp_widget_count_choices(widget);

    const char* ch;
    for (int i = 0; i < n; i++)
    {
        int result = gp_widget_get_choice(widget, i, &ch);
        if (result != GP_OK)
        {
            D(printf("Couldn't get choice (%s): %d", config_name.c_str(),
                     result));
            goto out;
        }
        choices.push_back(string(ch));
    }

out:
    gp_widget_free(widget);
    return choices;
}

vector<int> CameraWrapper::listAvailableExposureTimes()
{
    vector<int> out;
    vector<string> choices = listConfigChoices(CONFIG_EXPOSURE_TIME);

    for (auto it = choices.begin(); it != choices.end(); it++)
    {
        out.push_back(exposureTimeFromString(*it));
    }

    return out;
}

int CameraWrapper::exposureTimeFromString(string exposure_time)
{
    int exp_time = stoi(exposure_time) * 100;
    return exp_time != -100 ? exp_time : 0;  // Change BULB from -100 to 0}
}
