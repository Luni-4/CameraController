/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_MESSAGE_H
#define SRC_COMMUNICATION_MESSAGE_H

#include <cstdint>
#include <cstring>
/*
 * Message structure:
 *       1           1        1    2    0 <  N <= 2^16
 * |MAGIC_WORD1|MAGIC_WORD2|TYPE|SIZE|     DATA...      |
 */

static const uint8_t MAGIC_WORD_1 = 0x54;
static const uint8_t MAGIC_WORD_2 = 0xF0;

static const unsigned int MSG_HEADER_SIZE = 5;
static const unsigned int MSG_MAX_LENGTH  = 0xFFFF + MSG_HEADER_SIZE;

enum MessageType : uint8_t
{
    MSGTYPE_LOG         = 1,
    MSGTYPE_TELECOMMAND = 2,
    MSGTYPE_TELEMETRY   = 3,
    MSGTYPE_FILE        = 4
};

struct Message
{
    uint8_t type;
    uint16_t size;
    uint8_t* data;

    Message(uint8_t type, uint16_t size) : type(type), size(size)
    {
        data = new uint8_t[size];
    }

    ~Message() { delete[] data; }

    Message(const Message& msg)
    {
        type = msg.type;
        size = msg.size;
        data = new uint8_t[size];
        memcpy(data, msg.data, size);
    }

    Message& operator=(const Message& other)
    {
        if (&other == this)
            return *this;

        delete[] data;

        type = other.type;
        size = other.size;
        data = new uint8_t[size];
        memcpy(data, other.data, size);

        return *this;
    }
};

#endif /* SRC_COMMUNICATION_MESSAGE_H */
