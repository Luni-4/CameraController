/*
 *  Created on: Jul 12, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_CAMERA_CAMERA_H
#define SRC_CAMERA_CAMERA_H

#include <gphoto2/gphoto2.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

static const string NOT_A_GOOD_SERIAL = "NOT_A_GOOD_SERIAL";

static const string CONFIG_SERIAL_NUMBER = "serialnumber";
static const string CONFIG_EXPOSURE_TIME = "500d";

class CameraWrapper
{
public:
    static CameraWrapper& getInstance()
    {
        static CameraWrapper instance;
        return instance;
    }

    CameraWrapper(CameraWrapper const&) = delete;
    void operator=(CameraWrapper const&) = delete;

    bool connect();

    bool isConnected();

    string getSerialNumber();

    void capture();

    /**
     * Gets the value of a config of type text.
     * Use gphoto2 --list-config to view the available configs.
     * @param config The config name. Ex:/main/status/serialnumber -->
     * serialnumber
     * @return The config value, or an empty string if there was an error.
     */
    string getTextConfigValue(string config);
    bool setConfigValue(string config_name, string val);

    vector<string> listConfigChoices(string config);

    /**
     * Returns current exposure time in microseconds. 0 if BULB, -1 if error
     * @return
     */
    int getCurrentExposureTime();

    /**
     * Sets the exposure time on the camera to the specified index
     * @param index Exposure time index (obtained from
     * listAvailableExposureTimes())
     * @return True if success
     */
    bool setExposureTime(int index);

    /**
     * Ordered list of available exposure times in microseconds. 0 if BULB
     * @return
     */
    vector<int> listAvailableExposureTimes();

private:
    CameraWrapper();
    ~CameraWrapper();

    void freeCamera();

    static int exposureTimeFromString(string exposure_time);

    string serial = NOT_A_GOOD_SERIAL;

    Camera* camera = nullptr;
    GPContext* context;
};

#endif /* SRC_CAMERA_CAMERA_H_ */
