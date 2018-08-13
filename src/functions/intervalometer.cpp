/*
 *  Created on: Jul 14, 2018
 *      Author: Luca Erbetta
 */

#include "intervalometer.h"
#include "logger.h"

using namespace std::this_thread;

using std::unique_lock;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

typedef unique_lock<mutex> Lock;
typedef system_clock Clock;

Intervalometer::Intervalometer(int n_exposures, int interval,
                               string default_folder)
    : CameraFunction(), interval(milliseconds(interval)),
      num_shots(n_exposures), download_folder(default_folder)
{
}

Intervalometer::~Intervalometer() {}

bool Intervalometer::start()
{
    if (isTesting())
    {
        Log.e("Can't start while testing.");
        return false;
    }

    if (!started)
    {
        if (!connectCamera())
        {
            Log.e("Cannot start intervalometer");
            return false;
        }
        started = true;

        // Start the run thread
        thread_run = unique_ptr<thread>(new thread(&Intervalometer::run, this));
        thread_run.get()->detach();
    }

    return true;
}

void Intervalometer::abort()
{
    if (started && !finished)
    {
        abort_cond = true;
        cv_run.notify_one();
    }
}
bool Intervalometer::isFinished() { return finished; }

void Intervalometer::run()
{
    int i = 0;
    while (!abort_cond && i < num_shots)
    {
        auto start         = Clock::now();
        auto next_exposure = start + interval;
        i++;

        if (!capture())
        {
            Log.e("Capture %d failed.", i);
            break;
        }

        auto end = Clock::now();

        milliseconds remaining =
            duration_cast<milliseconds>(next_exposure - end);

        if (end > next_exposure)
        {
            Log.w("Delayed exposure by: %d ms", -1 * (int)remaining.count());
        }

        Lock lk(mutex_run);
        stats.registerExposureStat(-1 * (int)remaining.count());

        while (!abort_cond)
        {
            if (cv_run.wait_until(lk, next_exposure) == std::cv_status::timeout)
            {
                break;
            }
        }
        auto end2 = Clock::now();
        Log.i("Period duration: %d ms",
              (int)duration_cast<milliseconds>(end2 - start).count());
    }

    finished = true;
    Log.i("Intervalometer finished. Shots taken: %d/%d. Aborted: %s", i,
          num_shots, abort_cond ? "true" : "false");
}

bool Intervalometer::capture()
{
    CameraFilePath p{};
    if (!camera.capture(p))
    {
        Log.i("Sequencer capture failed.");
        return false;
    }

    Log.i("Captured exposure");

    {
        Lock lk(mutex_run);
        last_shot_path = p;
    }

    if (download_after_exposure)
    {
        auto download_start = Clock::now();
        Log.i("Downloading...");
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

bool Intervalometer::downloadLastPicture(string destination_folder)
{
    CameraFilePath p;
    {
        Lock lk(mutex_run);
        p = last_shot_path;
    }

    return camera.downloadFile(p, destination_folder + string(p.name));
}
