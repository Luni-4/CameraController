/*
 *  Created on: Jul 23, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_LOGGER_H
#define SRC_LOGGER_H

#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

using std::cout;
using std::endl;
using std::lock_guard;
using std::mutex;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

enum LogLevel : int
{
    LOG_DEBUG   = 0,
    LOG_INFO    = 1,
    LOG_WARNING = 2,
    LOG_ERROR   = 3
};

static const string LEVEL_STRINGS[] = {"DEBUG", "INFO", "WARNING", "ERROR"};

class Logger
{
public:
    Logger(vector<pair<ostream*, LogLevel>> streams = {{&cout, LOG_DEBUG}})
    {
        this->streams = streams;
    }
    ~Logger() {}

    template <typename T, typename... Args>
    void i(const char* str, T value, Args... args)
    {
        log(LOG_INFO, str, value, args...);
    }
    void i(const char* str) { log(LOG_INFO, str); }

    template <typename T, typename... Args>
    void d(const char* str, T value, Args... args)
    {
        log(LOG_DEBUG, str, value, args...);
    }
    void d(const char* str) { log(LOG_DEBUG, str); }

    template <typename T, typename... Args>
    void w(const char* str, T value, Args... args)
    {
        log(LOG_WARNING, str, value, args...);
    }
    void w(const char* str) { log(LOG_WARNING, str); }

    template <typename T, typename... Args>
    void e(const char* str, T value, Args... args)
    {
        log(LOG_ERROR, str, value, args...);
    }
    void e(const char* str) { log(LOG_ERROR, str); }

    void addStream(ostream* stream, LogLevel minlevel)
    {
        lock_guard<mutex> l(mtx_streams);
        streams.push_back({stream, minlevel});
    }

    void removeStream(ostream* stream)
    {
        lock_guard<mutex> l(mtx_streams);
        for (auto it = streams.begin(); it != streams.end(); it++)
        {
            if ((*it).first == stream)
                it = streams.erase(it);
        }
    }

    void clearStreams()
    {
        lock_guard<mutex> l(mtx_streams);
        streams.clear();
    }

    template <typename T, typename... Args>
    void log(LogLevel level, const char* str, T value, Args... args)
    {
        char buf[256];
        sprintf(buf, str, value, args...);

        log(level, buf);
    }

    void log(LogLevel level, const char* str)
    {
        auto t  = std::time(nullptr);
        auto tm = *std::localtime(&t);
        lock_guard<mutex> l1(mtx_streams);
        lock_guard<mutex> l2(mtx_log);
        for (auto it = streams.begin(); it != streams.end(); it++)
        {
            if (level >= (*it).second)
            {
                *(*it).first << std::put_time(&tm, "[%H:%M:%S]") << std::left
                             << std::setw(10) << getLogLevelString(level) << str
                             << endl;
                (*it).first->flush();
            }
        }
    }

private:
    vector<pair<ostream*, LogLevel>> streams;

    string getLogLevelString(LogLevel level)
    {
        return "[" + LEVEL_STRINGS[level] + "]";
    }

    mutex mtx_log;
    mutex mtx_streams;
};

extern Logger Log;

#endif /* SRC_LOGGER_H */
