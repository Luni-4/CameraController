/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#include "MessageDecoder.h"
#include "logger.h"

#include <algorithm>

using std::max;
using std::min;

MessageDecoder::MessageDecoder(MessageHandler& msgHandler) : handler(msgHandler)
{
}

MessageDecoder::~MessageDecoder()
{
    if (message != nullptr)
    {
        delete message;
    }
}
void MessageDecoder::dispatch() { handler.handleMessage(*message); }

void MessageDecoder::decode(uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        switch (state)
        {
            case DecoderState::START:
                if (data[i] == MAGIC_WORD_1)
                {
                    state = DecoderState::MAGIC1_FOUND;
                }
                break;
            case DecoderState::MAGIC1_FOUND:
                if (data[i] == MAGIC_WORD_2)
                {
                    state = DecoderState::MAGIC2_FOUND;
                }
                else
                {
                    state = DecoderState::START;
                }
                break;
            case DecoderState::MAGIC2_FOUND:
                temp_type = data[i];
                state     = DecoderState::TYPE_RECEIVED;
                break;
            case DecoderState::TYPE_RECEIVED:
            {
                temp_size1 = data[i];
                state      = DecoderState::SIZE1_RECEIVED;
                break;
            }
            case DecoderState::SIZE1_RECEIVED:
            {
                uint16_t size = data[i] << 8 | (uint16_t)temp_size1;
                state         = DecoderState::RECEIVING_DATA;
                if (message != nullptr)
                {
                    delete message;
                }
                message       = new Message(temp_type, size);
                received_data = 0;
                state         = DecoderState::RECEIVING_DATA;
                Log.d("Message created: type:%d, size:%d", temp_type, size);
                break;
            }
            case DecoderState::RECEIVING_DATA:
            {
                size_t sz = min(len - i, (size_t)message->size - received_data);
                memcpy(message->data + received_data, data + i, sz);
                received_data += sz;
                i += sz - 1;  // i must point to the last copied byte
                Log.d("Receive data: received: %d, sz: %d", received_data, sz);
                if (received_data == message->size)
                {
                    Log.d("Dispatch.");
                    dispatch();
                    state = DecoderState::START;
                }
                break;
            }
        }
    }
}
