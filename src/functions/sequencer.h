/*
 *  Created on: Jul 16, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_FUNCTIONS_SEQUENCER_H
#define SRC_FUNCTIONS_SEQUENCER_H

#include <gphoto2/gphoto2-camera.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <ratio>
#include <string>
#include <thread>

#include "camera/CameraWrapper.h"
#include "camerafunction.h"

using std::atomic_bool;

using std::mutex;

using std::string;
using std::thread;
using std::unique_ptr;
using std::chrono::duration;
using std::chrono::milliseconds;

/**
 * Takes N photos one after the other, with as minimum delay as possible.
 * Optionally downloads the photo after it is taken.
 */
class Sequencer : public CameraFunction
{
public:
    struct SequencerStats
    {
        int mean_intertime()
        {
            return total_exposure_intertime / exposures_count;
        }

        int exposure_time        = 0;
        int programmed_exposures = 0;

        int total_exposure_intertime = 0;
        int max_intertime            = 0;
        int last_intertime           = 0;
        int exposures_count          = 0;

        void registerExposureStat(int intertime)
        {
            last_intertime = intertime;
            total_exposure_intertime += intertime;

            if (max_intertime < intertime)
            {
                max_intertime = intertime;
            }

            exposures_count++;
        }

        void print()
        {
            Log.i("Intertime: %d ms, Mean: %d ms, Max: %d ms (Exp time: %d ms)",
                  last_intertime, mean_intertime(), max_intertime,
                  exposure_time);
        }
    };

    /**
     * Constructor.
     * @param n_exposures Number of exposures to take
     * @param exposure_time Exposure duration in ms
     */
    Sequencer(int n_exposures, int exposure_time,
              bool download_after_exp = false,
              string default_folder   = DEFAULT_DOWNLOAD_FOLDER);

    ~Sequencer();

    void downloadAfterExposure(bool value) override
    {
        download_after_exposure = value;
    };

    bool downloadAfterExposure() override { return download_after_exposure; };

    FunctionID getID() override { return FunctionID::SEQUENCER; }

    void configure(int n_exposures, int exposure_time);

    bool start() override;

    void abort() override;

    /**
     * Checks if the camera has began acquiring the exposures
     * @return True if the process has begun
     */
    bool isStarted() override { return started; }

    /**
     * Checks if the camera has taken all the requested exposures
     * @return True if intervalometer has finished
     */
    bool isFinished() override;

    /**
     * Download the last picture taken
     * @param path Folder where to download the picture
     * @return True if download successful
     */
    bool downloadLastPicture(string path);

protected:
    bool capture() override;

private:
    void run();

    bool started = false;

    atomic_bool finished{};
    int num_shots;
    int exposure_time;

    atomic_bool download_after_exposure{};
    string download_folder{};

    mutex mutex_run;
    atomic_bool abort_cond{};

    unique_ptr<thread> thread_run;

    CameraFilePath last_shot_path;

    SequencerStats stats;
};

#endif /* SRC_FUNCTIONS_SEQUENCER_H */
