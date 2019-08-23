/*
 *  Created on: Aug 8, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMANDS_COMMANDS_H
#define SRC_COMMANDS_COMMANDS_H

#include <cstdint>
#include <nlohmann/json.hpp>

#include <functional>
#include <map>

#include "logger.h"

using json = nlohmann::json;
using std::function;
using std::map;
using std::pair;

enum Commands : uint8_t
{
    CMD_ID_SHUTDOWN = 1,
    CMD_ID_REBOOT   = 2,

    CMD_ID_FUNCTIONSTART = 5,
    CMD_ID_FUNCTIONSTOP  = 6,

    CMD_ID_CAMERA_TEST_CONNECTION = 9,
    CMD_ID_CAMERA_RECONNECT       = 10,

    CMD_ID_FUNCTION_TEST_CAPTURE   = 18,
    CMD_ID_DOWNLOAD_AFTER_EXPOSURE = 19,

    CMD_ID_SEQUENCERSETUP      = 20,
    CMD_ID_INTERVALOMETERSETUP = 30
};

static const char* KEY_CMDID         = "cmd_id";
static const char* KEY_NUM_EXPOSURES = "num_exposures";
static const char* KEY_EXPOSURE_TIME = "exposure_time";
static const char* KEY_INTERVAL      = "interval";
static const char* KEY_DOWNLOAD      = "download";

class JsonCommandDecoder;

struct Command
{
    friend class JsonCommandDecoder;

    uint8_t cmd_id = 0;

    Command(uint8_t cmd_id) : cmd_id(cmd_id) {}
    virtual ~Command() {}

    virtual void toJson(json& j) { j[KEY_CMDID] = cmd_id; }

    virtual void print() const { Log.i("CMD{cmd: %d, et: %d, d: %s}", cmd_id); }

protected:
    Command() {}
};

struct DownloadAfterExposureCommand : public Command
{
    friend class JsonCommandDecoder;

    bool download = false;

    DownloadAfterExposureCommand(bool download)
        : Command(cmd_id), download(download)
    {
    }

    void print() const override
    {
        Log.i("DAE{cmd: %d, dwnld: %s}", cmd_id, download ? "true" : "false");
    }

protected:
    DownloadAfterExposureCommand() : Command() {}
};

struct SequencerSetupCommand : public Command
{
    friend class JsonCommandDecoder;

    int num_exposures = 0;
    int exp_time      = 0;
    bool download     = false;

    SequencerSetupCommand(uint8_t cmd_id, int num_exposures, int exp_time,
                          bool download = false)
        : Command(cmd_id), num_exposures(num_exposures), exp_time(exp_time),
          download(download)
    {
    }

    void print() const override
    {
        Log.i("SSC{cmd: %d, ne: %d, et: %d, d: %s}", cmd_id, num_exposures,
              exp_time, download ? "true" : "false");
    }

protected:
    SequencerSetupCommand() : Command() {}
};

struct IntervalometerSetupCommand : public Command
{
    friend class JsonCommandDecoder;

    int num_exposures = 0;
    int exp_time      = 0;
    int interval      = 0;
    bool download     = false;

    IntervalometerSetupCommand(uint8_t cmd_id, int num_exposures, int exp_time,
                               int interval, bool download = false)
        : Command(cmd_id), num_exposures(num_exposures), exp_time(exp_time),
          interval(interval), download(download)
    {
    }

    void print() const override
    {
        Log.i("ISC{cmd: %d, ne: %d, et: %d, int: %d, d: %s}", cmd_id,
              num_exposures, exp_time, interval, download ? "true" : "false");
    }

protected:
    IntervalometerSetupCommand() : Command() {}
};

class JsonCommandDecoder
{
    typedef function<bool(Command**, json&)> Decoder;
    typedef map<uint8_t, Decoder> DecoderMap;

public:
    static bool decode(Command** cmd, json& j)
    {
        try
        {
            uint8_t cmd_id = j.at(KEY_CMDID).get<uint8_t>();
            if (decoder_map.count(cmd_id) >= 1)
            {
                return decoder_map.at(cmd_id)(cmd, j);
            }
            else
            {
                Log.w("Unrecognized command: %d", cmd_id);
                return false;
            }
        }
        catch (std::exception& e)
        {
            Log.e(e.what());
            return false;
        }
    }

private:
    JsonCommandDecoder(){};

    static bool decodeEmptyCommand(Command** cmd, json& j);

    static bool decodeSetupSequencer(Command** cmd, json& j);
    static bool decodeSetupIntervalometer(Command** cmd, json& j);

    static bool decodeDownloadAfterExposure(Command** cmd, json& j);

    static const DecoderMap decoder_map;
};

#endif /* SRC_COMMANDS_COMMANDS_H */
