/*
 *  Created on: Jul 12, 2018
 *      Author: Luca Erbetta
 */

#include "CameraWrapper.h"
#include <gphoto2/gphoto2.h>
#include <cmath>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

#include "logger.h"
#include "utils/RemoteTrigger.h"

using namespace std::this_thread;

using std::max;
using std::min;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

typedef system_clock Clock;

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
    if (!connected)
    {
        int result = gp_camera_new(&camera);

        if (result != GP_OK)
        {
            Log.e("Error instantiating camera: %d", result);
            return false;
        }

        result = gp_camera_init(camera, context);

        if (result == GP_OK)
        {
            serial = getSerialNumber();

            // Sleep for 2 seconds to avoid errors if capturing too early
            std::this_thread::sleep_for(seconds(2));

            Log.i("Connected to camera: %s", serial.c_str());
            connected = true;
            return true;
        }
        Log.e("Error initiating camera: %d", result);
        gp_camera_free(camera);
        return false;
    }
    return true;
}

void CameraWrapper::disconnect()
{
    if (connected)
    {
        freeCamera();
        connected = false;
    }
}

bool CameraWrapper::isConnected() { return connected; }

bool CameraWrapper::isResponsive()
{
    return connected && camera != nullptr && getSerialNumber() == serial;
}

bool CameraWrapper::remoteCapture(int exposure_time, CameraFilePath& path)
{
    int curr_exp = getCurrentExposureTime();
    if (curr_exp != 0)
    {
        Log.e("Remote capture: Not in BULB mode. Exp time: %d", curr_exp);
        return false;
    }

    Log.i("Trigger 1");

    const milliseconds checkpoint_interval(30000);

    trigger();

    auto now          = Clock::now();
    auto end_exposure = now + milliseconds(exposure_time);
    do
    {
        now            = Clock::now();
        auto remaining = duration_cast<milliseconds>(end_exposure - now);
        Log.i("Remote capture: %d ms left", (int)remaining.count());

        if (remaining < checkpoint_interval)
        {
            sleep_until(end_exposure);
        }
        else
        {
            sleep_for(checkpoint_interval);
        }
    } while (now < end_exposure);

    trigger();
    Log.i("Trigger 2");

    // Wait a bit before checking for capture completed
    sleep_for(milliseconds(500));

    return waitForCapture(path);
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
        Log.d("Captured to: %s/%s", path.folder, path.name);
        return true;
    }
    else
    {
        Log.e("Error capturing picture: %d", result);
        return false;
    }
}

bool CameraWrapper::downloadFile(CameraFilePath path, string dest_file_path)
{
    CameraFile* file;
    int result, fd;
    bool success = false, gp_file_created = false;
    Log.d("Download file: %s, fld: %s", path.name, path.folder);

    Log.d("Download dest: %s", dest_file_path.c_str());
    FILE* f = fopen(dest_file_path.c_str(), "w");

    if (f == NULL)
    {
        Log.e("Error opening file (%s): %s", dest_file_path.c_str(),
              std::strerror(errno));
        goto out;
    }

    fd = fileno(f);

    if (fd < 0)
    {
        Log.e("Error getting file descriptor (%s): %d", dest_file_path.c_str(),
              fd);
        goto out;
    }

    result = gp_file_new_from_fd(&file, fd);
    if (result != GP_OK)
    {
        Log.e("Error creating CameraFile (%s): %d", dest_file_path.c_str(),
              result);
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
        Log.e("Error getting file from camera (%s): %d", dest_file_path.c_str(),
              result);
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
        Log.e("Couldn't get single config (%s): %d", config_name.c_str(),
              result);
        goto out;
    }

    const char* value;
    result = gp_widget_get_value(widget, &value);

    if (result != GP_OK)
    {
        Log.e("Couldn't get widget value (%s): %d", config_name.c_str(),
              result);
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
        Log.e("Couldn't get single config (%s): %d", config_name.c_str(),
              result);
        goto out;
    }
    CameraWidgetType type;
    result = gp_widget_get_type(widget, &type);
    if (result != GP_OK)
    {
        Log.e("Couldn't get config type (%s): %d", config_name.c_str(), result);
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
                Log.e("Couldn't set config value (%s): %d", config_name.c_str(),
                      result);
                goto out;
            }
            // Finally set the config on the camera
            result = gp_camera_set_single_config(camera, config_name.c_str(),
                                                 widget, context);
            if (result != GP_OK)
            {
                Log.e("Couldn't set config on camera (%s): %d",
                      config_name.c_str(), result);
                goto out;
            }
            success = true;
            goto out;
            break;
        }

        default:
            Log.e("Bad widget type (%s): %d", config_name.c_str(), type);
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
        Log.e("Couldn't get single config (%s): %d", config_name.c_str(),
              result);
        goto out;
    }

    n = gp_widget_count_choices(widget);

    const char* ch;
    for (int i = 0; i < n; i++)
    {
        int result = gp_widget_get_choice(widget, i, &ch);
        if (result != GP_OK)
        {
            Log.e("Couldn't get choice (%s): %d", config_name.c_str(), result);
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

bool CameraWrapper::waitForCapture(CameraFilePath& file, int timeout)
{
    const milliseconds min_wait_time(1000);
    CameraEventType type;
    void* data;

    auto c_start = Clock::now();

    do
    {
        auto start = std::chrono::system_clock::now();
        int res =
            gp_camera_wait_for_event(camera, timeout, &type, &data, context);

        if (res != GP_OK)
        {
            Log.e("Couldn't wait for event");
            return false;
        }
        auto end = std::chrono::system_clock::now();

        auto duration = duration_cast<milliseconds>(end - start);
        int dur       = (int)duration.count();
        timeout -= dur;
    } while (type != GP_EVENT_FILE_ADDED);

    auto c_end    = Clock::now();
    auto duration = duration_cast<milliseconds>(c_end - c_start);
    int dur       = (int)duration.count();

    if (duration < min_wait_time)
    {
        Log.w("Additional wait of %d ms",
              (int)milliseconds(min_wait_time - duration).count());
        sleep_for(min_wait_time - duration);
    }

    switch (type)
    {
        case GP_EVENT_UNKNOWN:
            Log.i("GP_EVENT_UNKNOWN T:%d ms", dur);
            return true;
        case GP_EVENT_TIMEOUT:
            Log.i("GP_EVENT_TIMEOUT T:%d ms", dur);
            break;
        case GP_EVENT_FILE_ADDED:
            Log.i("GP_EVENT_FILE_ADDED T:%d ms", dur);
            file = *((CameraFilePath*)data);
            return true;
        case GP_EVENT_FOLDER_ADDED:
            Log.i("GP_EVENT_FOLDER_ADDED T:%d ms", dur);
            break;
        case GP_EVENT_CAPTURE_COMPLETE:
            Log.i("GP_EVENT_CAPTURE_COMPLETE T:%d ms", dur);
            break;
        case GP_EVENT_FILE_CHANGED:
            Log.i("GP_EVENT_FILE_CHANGED T:%d ms", dur);
            break;
        default:
            Log.i("WTF event T:%d ms", dur);
    }

    return false;
}
