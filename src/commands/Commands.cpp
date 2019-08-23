/*
 *  Created on: Aug 8, 2018
 *      Author: Luca Erbetta
 */

#include "Commands.h"

// clang-format off

const JsonCommandDecoder::DecoderMap JsonCommandDecoder::decoder_map = {

    {CMD_ID_SHUTDOWN, JsonCommandDecoder::decodeEmptyCommand},
    {CMD_ID_REBOOT, JsonCommandDecoder::decodeEmptyCommand},

    {CMD_ID_FUNCTIONSTART, JsonCommandDecoder::decodeEmptyCommand},
    {CMD_ID_FUNCTIONSTOP, JsonCommandDecoder::decodeEmptyCommand},

	{CMD_ID_CAMERA_TEST_CONNECTION, JsonCommandDecoder::decodeEmptyCommand},
	{CMD_ID_CAMERA_RECONNECT, JsonCommandDecoder::decodeEmptyCommand},

	{CMD_ID_FUNCTION_TEST_CAPTURE, JsonCommandDecoder::decodeEmptyCommand},
    {CMD_ID_DOWNLOAD_AFTER_EXPOSURE, JsonCommandDecoder::decodeDownloadAfterExposure},

    {CMD_ID_SEQUENCERSETUP, JsonCommandDecoder::decodeSetupSequencer},
    {CMD_ID_INTERVALOMETERSETUP, JsonCommandDecoder::decodeSetupIntervalometer}

};

// clang-format on

bool JsonCommandDecoder::decodeEmptyCommand(Command** cmd, json& j)
{
    Command* c = new Command();

    try
    {
        c->cmd_id = j.at(KEY_CMDID).get<uint8_t>();
    }
    catch (std::exception& e)
    {
        delete c;
        Log.e(e.what());
        return false;
    }

    *cmd = c;
    return true;
}

bool JsonCommandDecoder::decodeSetupSequencer(Command** cmd, json& j)
{
    SequencerSetupCommand* c = new SequencerSetupCommand();

    try
    {
        c->cmd_id        = j.at(KEY_CMDID).get<uint8_t>();
        c->num_exposures = j.at(KEY_NUM_EXPOSURES).get<int>();
        c->exp_time      = j.at(KEY_EXPOSURE_TIME).get<int>();
        c->download      = j.at(KEY_DOWNLOAD).get<bool>();
    }
    catch (std::exception& e)
    {
        delete c;
        Log.e(e.what());
        return false;
    }

    *cmd = c;
    return true;
}

bool JsonCommandDecoder::decodeSetupIntervalometer(Command** cmd, json& j)
{
    IntervalometerSetupCommand* c = new IntervalometerSetupCommand();

    try
    {
        c->cmd_id        = j.at(KEY_CMDID).get<uint8_t>();
        c->num_exposures = j.at(KEY_NUM_EXPOSURES).get<int>();
        c->exp_time      = j.at(KEY_EXPOSURE_TIME).get<int>();
        c->interval      = j.at(KEY_INTERVAL).get<int>();
        c->download      = j.at(KEY_DOWNLOAD).get<bool>();
    }
    catch (std::exception& e)
    {
        delete c;
        Log.e(e.what());
        return false;
    }

    *cmd = c;
    return true;
}

bool JsonCommandDecoder::decodeDownloadAfterExposure(Command** cmd, json& j)
{
    DownloadAfterExposureCommand* c = new DownloadAfterExposureCommand();

    try
    {
        c->cmd_id   = j.at(KEY_CMDID).get<uint8_t>();
        c->download = j.at(KEY_DOWNLOAD).get<bool>();
    }
    catch (std::exception& e)
    {
        delete c;
        Log.e(e.what());
        return false;
    }

    *cmd = c;
    return true;
}
