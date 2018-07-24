/*
 *  Created on: Jul 12, 2018
 *      Author: Luca Erbetta
 */

#include "CameraWrapper.h"
#include <gphoto2/gphoto2.h>
#include <cmath>

#include <chrono>
#include <cstdio>
#include <thread>
#include "debug.h"

using std::chrono::seconds;

CameraWrapper::CameraWrapper() : context(gp_context_new()) {}

CameraWrapper::~CameraWrapper() { freeCamera(); }

void CameraWrapper::freeCamera()
{
    if (camera != nullptr)
    {
        gp_camera_exit(camera, context);
        gp_camera_free(camera);
        camera = nullptr;
    }
}

bool CameraWrapper::connect()
{
    gp_camera_new(&camera);
    int result = gp_camera_init(camera, context);
    if (result == GP_OK)
    {
        serial = getSerialNumber();

        // Sleep for 2 seconds to avoid errors if capturing too early
        std::this_thread::sleep_for(seconds(2));

        D(printf("Connected to camera: %s\n", serial.c_str()));
        return true;
    }
    camera = nullptr;
    return false;
}

bool CameraWrapper::isConnected()
{
    if (camera != nullptr)
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
    }
    return false;
}

bool CameraWrapper::capture()
{
    CameraFilePath path;
    return capture(path);
}

bool CameraWrapper::capture(CameraFilePath& path)
{
    int result = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &path, context);

    if (result == GP_OK)
    {
        D(printf("Captured to: %s/%s\n", path.folder, path.name));
        return true;
    }
    else
    {
        D(printf("Error capturing picture: %d\n", result));
        return false;
    }
}

bool CameraWrapper::downloadFile(CameraFilePath path, string dest_file_path)
{
    CameraFile* file;
    int result, fd;
    bool success = false, gp_file_created = false;

    D(printf("Download dest: %s\n", dest_file_path.c_str()));
    FILE* f = fopen(dest_file_path.c_str(), "w");

    if (f == NULL)
    {
        /*D(printf("Error opening file (%s): %d\n", dest_file_path.c_str(),
                 errno));*/
        goto out;
    }

    fd = fileno(f);

    if (fd < 0)
    {
        D(printf("Error getting file descriptor (%s): %d\n",
                 dest_file_path.c_str(), fd));
        goto out;
    }

    result = gp_file_new_from_fd(&file, fd);
    if (result != GP_OK)
    {
        D(printf("Error creating CameraFile (%s): %d\n", dest_file_path.c_str(),
                 result));
        goto out;
    }
    else
    {
        gp_file_created = true;
    }

    result = gp_camera_file_get(camera, path.folder, path.name,
                                GP_FILE_TYPE_RAW, file, context);
    if (result != GP_OK)
    {
        D(printf("Error getting file from camera (%s): %d\n",
                 dest_file_path.c_str(), result));
        goto out;
    }
    success = true;
out:
    if (f != NULL)
    {
        fclose(f);
    }
    if (gp_file_created)
    {
        gp_file_free(file);
    }
    return success;
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

bool CameraWrapper::waitForCapture()
{
    CameraEventType type;
    void* data;

    auto start = std::chrono::system_clock::now();

    int res = gp_camera_wait_for_event(camera, 30000, &type, &data, context);

    if (res != GP_OK)
    {
        return false;
    }
    auto end = std::chrono::system_clock::now();

    int dur =
        std::chrono::milliseconds(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start))
            .count();
    switch (type)
    {
        case GP_EVENT_UNKNOWN:
            printf("GP_EVENT_UNKNOWN T:%d ms\n", dur);
            break;
        case GP_EVENT_TIMEOUT:
            printf("GP_EVENT_TIMEOUT T:%d ms\n", dur);
            break;
        case GP_EVENT_FILE_ADDED:
            printf("GP_EVENT_FILE_ADDED T:%d ms\n", dur);
            break;
        case GP_EVENT_FOLDER_ADDED:
            printf("GP_EVENT_FOLDER_ADDED T:%d ms\n", dur);
            break;
        case GP_EVENT_CAPTURE_COMPLETE:
            printf("GP_EVENT_CAPTURE_COMPLETE T:%d ms\n", dur);
            break;
        case GP_EVENT_FILE_CHANGED:
            printf("GP_EVENT_FILE_CHANGED T:%d ms\n", dur);
            break;
        default:
            printf("WTF event T:%d ms\n", dur);
    }

    return true;
}
