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

Intervalometer::Intervalometer(int n_exposures, int interval, int exposure_time,
                               bool download_after_exposure,
                               string download_folder)
    : CameraFunction(download_folder), interval(milliseconds(interval)),
      num_shots(n_exposures), exposure_time(exposure_time)
{
    downloadAfterExposure(download_after_exposure);

    Log.i("Intervalometer configured: Number of exposures: %d, Exp time: %d, Interval: %d",
          n_exposures, exposure_time, interval);
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
        return true;
    }
    else
    {
        Log.i("Intervalometer already started");
    }

    return false;
}

void Intervalometer::abort()
{
    if (started && !finished)
    {
        Log.i("Aborting intervalometer");
        abort_cond = true;
        cv_run.notify_one();
    }
    else
    {
        Log.i("Intervalometer not started or already ended");
    }
}
bool Intervalometer::isFinished() { return finished; }

void Intervalometer::run()
{
    int i = 0;
    while (!abort_cond && (i < num_shots || num_shots == -1))
    {
        auto start         = Clock::now();
        auto next_exposure = start + interval;
        i++;

        if (!camera.capture(exposure_time,
                            downloadAfterExposure() ? download_folder : ""))
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

        {
            Lock lk(mutex_run);
            stats.registerExposureStat(-1 * (int)remaining.count());

            while (!abort_cond)
            {
                if (cv_run.wait_until(lk, next_exposure) ==
                    std::cv_status::timeout)
                {
                    break;
                }
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

void Intervalometer::doTestCapture()
{
    testing = true;
    if (camera.capture(exposure_time,
                       downloadAfterExposure() ? download_folder : ""))
    {
        Log.i("Test capture completed successfully");
    }
    else
    {
        Log.i("Test capture finished with errors.");
    }
    testing = false;
}