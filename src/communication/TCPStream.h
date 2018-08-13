/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_TCPSTREAM_H
#define SRC_COMMUNICATION_TCPSTREAM_H

#include <cstdio>
#include <ostream>
#include <streambuf>

#include "MessageEncoder.h"

static const unsigned int STREAM_BUF_SIZE = 2048;  // 2 KiB

class NetStreamBuf : public std::streambuf
{

public:
    NetStreamBuf(MessageEncoder *encoder) : encoder(encoder)
    {
        setp(buf, buf + STREAM_BUF_SIZE - 1);
    }

protected:
    void send()
    {
        std::ptrdiff_t len = pptr() - pbase();
        encoder->sendLog(pbase(), len);

        setp(buf, buf + STREAM_BUF_SIZE - 1);
    }

private:
    NetStreamBuf(const NetStreamBuf &) = delete;
    NetStreamBuf &operator=(const NetStreamBuf &) = delete;

    int sync() override
    {
        send();
        return 0;
    }

    int_type overflow(int_type ch) override
    {
        if (ch != traits_type::eof())
        {
            assert(std::less_equal<char *>()(pptr(), epptr()));
            *pptr() = ch;
            pbump(1);
            send();
            return ch;
        }

        return traits_type::eof();
    }

    char buf[STREAM_BUF_SIZE];
    MessageEncoder *encoder;
};

class NetStream : public std::ostream
{
public:
    NetStream(MessageEncoder *encoder) : buf(encoder) { rdbuf(&buf); }

private:
    NetStreamBuf buf;
};

#endif /* SRC_COMMUNICATION_TCPSTREAM_H */
