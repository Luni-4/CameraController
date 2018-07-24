/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_MESSAGEDECODER_H
#define SRC_COMMUNICATION_MESSAGEDECODER_H

#include <cstdint>
#include <cstring>

#include "circular_buffer.h"

/*
 * Message structure:
 *       1           1        1    2    0 <  N <= 2^16
 * |MAGIC_WORD1|MAGIC_WORD2|TYPE|SIZE|     DATA...      |
 */

static const uint8_t MAGIC_WORD_1 = 0x54;
static const uint8_t MAGIC_WORD_2 = 0xF0;

static const unsigned int DECODER_BUF_SIZE = (2 ^ 16) + 5;  // Max msg length

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

class MessageDecoder
{
public:
    MessageDecoder();
    ~MessageDecoder();

    void decode(uint8_t* data, size_t len);

private:
    enum class DecoderState
    {
        START,
        MAGIC1_FOUND,
        MAGIC2_FOUND,
        TYPE_RECEIVED,
        RECEIVING_DATA
    };

    void dispatch();

    DecoderState state   = DecoderState::START;
    uint8_t temp_type    = 0;
    size_t received_data = 0;

    Message* message = nullptr;
};

#endif /* SRC_COMMUNICATION_MESSAGEDECODER_H */
