/*
 *  Created on: Jul 14, 2018
 *      Author: Luca Erbetta
 */

#include "intervalometer.h"
#include "debug.h"

using namespace std::this_thread;

using std::unique_lock;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

typedef unique_lock<mutex> Lock;
typedef system_clock Clock;

Intervalometer::Intervalometer(int n_exposures, int interval)
    : interval(milliseconds(interval)), num_shots(n_exposures),
      camera(CameraWrapper::getInstance())
{
}

Intervalometer::~Intervalometer() {}

bool Intervalometer::start()
{
    if (!started)
    {
        if (!camera.isConnected())
        {
            if (!camera.connect())
            {
                return false;
            }
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

        CameraFilePath p = capture();
        i++;
        D(printf("Captured exposure %d\n", i));

        auto end = Clock::now();

        milliseconds remaining =
            duration_cast<milliseconds>(next_exposure - end);

        if (end > next_exposure)
        {
            D(printf("Delayed exposure by: %d ms\n",
                     -1 * (int)remaining.count()));
        }

        Lock lk(mutex_run);

        last_shot_path = p;
        stats.registerExposureStat(-1 * (int)remaining.count());

        while (!abort_cond)
        {
            if (cv_run.wait_until(lk, next_exposure) == std::cv_status::timeout)
            {
                break;
            }
        }
        auto end2 = Clock::now();
        D(printf("Period duration: %d ms\n",
                 (int)duration_cast<milliseconds>(end2 - start).count()));
    }

    finished = true;
    D(printf("Runner thread ended. Shots taken: %d. Aborted: %s\n", i,
             abort_cond ? "true" : "false"));
}

CameraFilePath Intervalometer::capture()
{
    CameraFilePath path;
    camera.capture(path);
    // sleep_for(milliseconds(2 * 1000));
    return path;
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
