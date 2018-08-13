/*
 *  Created on: Jul 16, 2018
 *      Author: Luca Erbetta
 */

#include "sequencer.h"
#include "logger.h"

using namespace std::this_thread;

using std::unique_lock;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

typedef unique_lock<mutex> Lock;
typedef system_clock Clock;

Sequencer::Sequencer(int n_exposures, int exposure_time,
                     bool download_after_exp, string default_folder)

    : CameraFunction(), num_shots(n_exposures), exposure_time(exposure_time),
      download_after_exposure(download_after_exp),
      download_folder(default_folder)
{
    stats.exposure_time        = exposure_time;
    stats.programmed_exposures = n_exposures;

    Log.i("Sequencer configured: Number of exposures: %d, Exp time: %d",
          n_exposures, exposure_time);
}

Sequencer::~Sequencer() {}

void Sequencer::configure(int n_exposures, int exposure_time)
{
    if (!isStarted() && !isOperating())
    {
        Log.i("Sequencer configured: Number of exposures: %d, Exp time: %d",
              n_exposures, exposure_time);
        num_shots           = n_exposures;
        this->exposure_time = exposure_time;
    }
    else
    {
        Log.w("Cannot configure: already running.");
    }
}

bool Sequencer::start()
{
    if (isTesting())
    {
        Log.e("Can't start while testing.");
        return false;
    }

    if (!started)
    {
        Log.i("Starting sequencer");
        if (!connectCamera())
        {
            Log.e("Cannot start sequencer");
            return false;
        }
        started = true;

        // Start the run thread
        thread_run = unique_ptr<thread>(new thread(&Sequencer::run, this));
        thread_run.get()->detach();
        return true;
    }
    else
    {
        Log.i("Sequencer already started");
    }

    return true;
}

void Sequencer::abort()
{

    if (started && !finished)
    {
        Log.i("Aborting sequencer");
        abort_cond = true;
    }
    else
    {
        Log.i("Sequencer not started or already ended");
    }
}

bool Sequencer::isFinished() { return finished; }

void Sequencer::run()
{
    int i = 0;
    while (!abort_cond && (i < num_shots || num_shots == -1))
    {
        i++;
        auto start = Clock::now();

        if (!capture())
        {
            Log.e("Capture %d failed.", i);
            break;
        }

        auto end = Clock::now();

        int exposure_duration =
            (int)duration_cast<milliseconds>(end - start).count();

        int intertime = exposure_duration - exposure_time;

        Lock lk(mutex_run);

        stats.registerExposureStat(intertime);

        Log.i("Sequencer shot %d/%d completed.", i, num_shots);
        stats.print();
    }

    finished = true;
    Log.i("Sequencer finished. Shots taken: %d/%d. Aborted: %s", i, num_shots,
          abort_cond ? "true" : "false");
}

bool Sequencer::capture()
{
    CameraFilePath p{};
    if (camera.getCurrentExposureTime() == 0)  // If BULB use remote trigger
    {
        if (!camera.remoteCapture(exposure_time, p))
        {
            Log.e("Sequencer capture failed.");
            return false;
        }
    }
    else
    {
        if (!camera.capture(p))
        {
            Log.e("Sequencer capture failed.");
            return false;
        }
    }

    Log.d("Captured exposure");

    {
        Lock lk(mutex_run);
        last_shot_path = p;
    }

    if (download_after_exposure)
    {
        auto download_start = Clock::now();
        Log.d("Downloading...");
        bool download_success = downloadLastPicture(download_folder);
        auto download_end     = Clock::now();

        if (download_success)
        {
            Log.i(
                "Download complete. duration: %d ms",
                (int)duration_cast<milliseconds>(download_end - download_start)
                    .count());
        }
        else
        {
            Log.e("Download failed.");
        }
    }
    return true;
}

bool Sequencer::downloadLastPicture(string destination_folder)
{
    CameraFilePath p;
    {
        Lock lk(mutex_run);
        p = last_shot_path;
    }

    return camera.downloadFile(p, destination_folder + string(p.name));
}
