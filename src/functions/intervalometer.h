/*
 *  Created on: Jul 14, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_FUNCTIONS_INTERVALOMETER_H
#define SRC_FUNCTIONS_INTERVALOMETER_H

#include <gphoto2/gphoto2-camera.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <ratio>
#include <string>
#include <thread>

#include "camera/CameraWrapper.h"

using std::atomic_bool;

using std::condition_variable;
using std::mutex;

using std::string;
using std::thread;
using std::unique_ptr;
using std::chrono::duration;
using std::chrono::milliseconds;

class Intervalometer
{
public:
    struct IntervalometerStats
    {
        int mean_delay() { return total_delay / exposures_count; }
        int total_delay;
        int max_delay;

        int delayed_exposures_count;
        int exposures_count;

        void registerExposureStat(int delay)
        {
            total_delay += delay;

            if (max_delay < delay)
            {
                max_delay = delay;
            }

            exposures_count++;

            if (delay > 0)
            {
                delayed_exposures_count++;
            }
        }
    };

    /**
     * Constructor
     * @param n_shots Number of exposures to take
     * @param interval Time between the start of each exposure in milliseconds
     */
    Intervalometer(int n_exposures, int interval);
    ~Intervalometer();

    bool start();

    void abort();

    /**
     * Checks if the camera has taken all the requested exposures
     * @return True if intervalometer has finished
     */
    bool isFinished();

    /**
     * Download the last picture taken
     * @param path Folder where to download the picture
     * @return True if download successful
     */
    bool downloadLastPicture(string path);

private:
    void run();

    /**
     * Captures a picture on the camera and stores the location in last_shot
     * @return Path of the picture on the camera
     */
    CameraFilePath capture();

    bool started = false;
    atomic_bool finished{};

    const milliseconds interval;
    const int num_shots;

    IntervalometerStats stats;

    mutex mutex_run;
    condition_variable cv_run;
    atomic_bool abort_cond{};

    unique_ptr<thread> thread_run;

    CameraFilePath last_shot_path;

    CameraWrapper& camera;
};

#endif /* SRC_FUNCTIONS_INTERVALOMETER_H */
