/*
 *  Created on: Jul 12, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_CAMERA_CAMERA_H
#define SRC_CAMERA_CAMERA_H

#include <gphoto2/gphoto2.h>
#include <string>

using std::string;

static const string NOT_A_GOOD_SERIAL = "NOT_A_GOOD_SERIAL";
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

    /**
     * Gets the value of a config of type text.
     * Use gphoto2 --list-config to view the available configs.
     * @param config The config name. Ex:/main/status/serialnumber -->
     * serialnumber
     * @return The config value, or an empty string if there was an error.
     */
    string getTextConfig(string config);

private:
    CameraWrapper();
    ~CameraWrapper();

    void freeCamera();

    string serial = NOT_A_GOOD_SERIAL;

    Camera* camera = nullptr;
    GPContext* context;
};

#endif /* SRC_CAMERA_CAMERA_H_ */
