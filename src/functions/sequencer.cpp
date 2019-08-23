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
                     bool download_after_exp, string download_folder)

    : CameraFunction(download_folder), num_shots(n_exposures),
      exposure_time(exposure_time)
{
    downloadAfterExposure(download_after_exp);

    stats.exposure_time        = exposure_time;
    stats.programmed_exposures = n_exposures;

    Log.i("Sequencer configured: Number of exposures: %d, Exp time: %d",
          n_exposures, exposure_time);
}

Sequencer::~Sequencer() {}

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

    return false;
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

        if (!camera.capture(exposure_time,
                            downloadAfterExposure() ? download_folder : ""))
        {
            Log.e("Capture %d failed.", i);
            break;
        }

        auto end = Clock::now();

        int exposure_duration =
            (int)duration_cast<milliseconds>(end - start).count();

        int intertime = exposure_duration - exposure_time;

        {
            Lock lk(mutex_run);

            stats.registerExposureStat(intertime);

            Log.i("Sequencer shot %d/%d completed.", i, num_shots);
            stats.print();
        }
    }

    finished = true;
    Log.i("Sequencer finished. Shots taken: %d/%d. Aborted: %s", i, num_shots,
          abort_cond ? "true" : "false");
}

void Sequencer::doTestCapture()
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