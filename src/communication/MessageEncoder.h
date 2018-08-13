/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_MESSAGEENCODER_H
#define SRC_COMMUNICATION_MESSAGEENCODER_H

#include <algorithm>
#include <cstdio>
#include <cstring>
#include "TCPServer.h"

class MessageEncoder
{
public:
    MessageEncoder(TCPServer* server) : server(server)
    {
        buf = new uint8_t[MSG_MAX_LENGTH];
    }
    ~MessageEncoder() { delete[] buf; }

    bool sendLog(const char* str, size_t len)
    {
        uint16_t maxlen = 0xFFFF;
        uint8_t type    = MSGTYPE_LOG;

        buf[0] = MAGIC_WORD_1;
        buf[1] = MAGIC_WORD_2;
        buf[2] = type;

        size_t consumed = 0;

        while (consumed < len)
        {
            uint16_t size;
            if (len - consumed > (size_t)maxlen)
            {
                size = maxlen;
            }
            else
            {
                size = (uint16_t)(len - consumed);
            }

            buf[3] = (uint8_t)size;
            buf[4] = (uint8_t)(size >> 8);

            memcpy(buf + MSG_HEADER_SIZE, str + consumed, size);
            consumed += size;

            server->sendData(buf, size + MSG_HEADER_SIZE);
        }
        return true;
    }

    void sendTelemetry() {}

    void sendFile() {}

private:
    uint8_t* buf;
    TCPServer* server;
};

#endif /* SRC_COMMUNICATION_MESSAGEENCODER_H */
