/*
 *  Created on: Jul 16, 2018
 *      Author: Luca Erbetta
 */

#include "sequencer.h"
#include "debug.h"

using namespace std::this_thread;

using std::unique_lock;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

typedef unique_lock<mutex> Lock;
typedef system_clock Clock;

Sequencer::Sequencer(int n_exposures, int exposure_time)
    : num_shots(n_exposures), camera(CameraWrapper::getInstance()),
      exposure_time(exposure_time)
{
    printf("Exp time: %d\n", exposure_time);

    stats.exposure_time        = exposure_time;
    stats.programmed_exposures = n_exposures;
}

Sequencer::~Sequencer() {}

bool Sequencer::start()
{
    if (!started)
    {
        if (!camera.isConnected())
        {
            return false;
        }
        started = true;

        // Start the run thread
        thread_run = unique_ptr<thread>(new thread(&Sequencer::run, this));
        thread_run.get()->detach();
    }

    return true;
}

void Sequencer::abort()
{
    if (started && !finished)
    {
        abort_cond = true;
    }
}
bool Sequencer::isFinished() { return finished; }

void Sequencer::run()
{
    int i = 0;
    while (!abort_cond && i < num_shots)
    {
        auto start = Clock::now();

        CameraFilePath p{};

        i++;
        if (!camera.capture(p))
        {
            D(printf("Sequencer capture %d failed.\n", i));
            abort_cond = true;
            continue;
        }

        D(printf("Captured exposure %d\n", i));

        {
            Lock lk(mutex_run);
            last_shot_path = p;
        }

        if (download_after_exposure)
        {
            auto download_start = Clock::now();
            D(printf("Downloading...\n"));
            bool download_success = downloadLastPicture(download_folder);
            auto download_end     = Clock::now();

            if (download_success)
            {
                D(printf("Download complete. duration: %d ms\n",
                         (int)duration_cast<milliseconds>(download_end -
                                                          download_start)
                             .count()));
            }
            else
            {
                D(printf("Download failed.\n"));
            }
        }

        auto end = Clock::now();

        int exposure_duration =
            (int)duration_cast<milliseconds>(end - start).count();

        int intertime = exposure_duration - exposure_time;

        Lock lk(mutex_run);

        stats.registerExposureStat(intertime);

        D(stats.print());
    }

    finished = true;
    D(printf("Runner thread ended. Shots taken: %d. Aborted: %s\n", i,
             abort_cond ? "true" : "false"));
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
