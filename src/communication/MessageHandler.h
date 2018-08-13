/*
 *  Created on: Aug 8, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_MESSAGEHANDLER_H
#define SRC_COMMUNICATION_MESSAGEHANDLER_H

#include <string>
#include <vector>

#include "Message.h"
#include "commands/Commands.h"

using std::string;
using std::vector;

class OnMessageReceivedListener
{
public:
    virtual void onCommandReceived(const Command& command) = 0;

    virtual ~OnMessageReceivedListener() {}
};

class MessageHandler
{
public:
    MessageHandler(OnMessageReceivedListener& listener) : listener(listener) {}

    void handleMessage(const Message& msg)
    {
        switch (msg.type)
        {
            case MessageType::MSGTYPE_TELECOMMAND:
            {
                string s =
                    string(reinterpret_cast<const char*>(msg.data), msg.size);

                json j = json::parse(s);
                Command* c;
                if (JsonCommandDecoder::decode(&c, j))
                {
                    listener.onCommandReceived(*c);
                    delete c;
                }

                break;
            }
        }

        return;
    }

private:
    OnMessageReceivedListener& listener;
};

#endif /* SRC_COMMUNICATION_MESSAGEHANDLER_H */
