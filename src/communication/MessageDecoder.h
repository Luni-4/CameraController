/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_MESSAGEDECODER_H
#define SRC_COMMUNICATION_MESSAGEDECODER_H

#include <cstdint>
#include <cstring>

#include "Message.h"
#include "MessageHandler.h"
#include "circular_buffer.h"

class MessageDecoder
{
public:
    MessageDecoder(MessageHandler& msgHandler);
    ~MessageDecoder();

    void decode(uint8_t* data, size_t len);

private:
    enum class DecoderState
    {
        START,
        MAGIC1_FOUND,
        MAGIC2_FOUND,
        TYPE_RECEIVED,
        SIZE1_RECEIVED,
        RECEIVING_DATA
    };

    void dispatch();

    DecoderState state   = DecoderState::START;
    uint8_t temp_type    = 0;
    uint8_t temp_size1   = 0;
    size_t received_data = 0;

    Message* message = nullptr;

    MessageHandler& handler;
};

#endif /* SRC_COMMUNICATION_MESSAGEDECODER_H */
