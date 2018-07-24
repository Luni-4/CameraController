/*
 *  Created on: Jul 21, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_COMMUNICATION_TCPSERVER_H
#define SRC_COMMUNICATION_TCPSERVER_H

#include <sys/socket.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <ostream>
#include <streambuf>
#include <thread>

#include "MessageDecoder.h"
#include "circular_buffer.h"

using std::atomic_bool;
using std::atomic_int;
using std::condition_variable;
using std::mutex;
using std::thread;

using std::unique_ptr;

static const unsigned int SEND_BUF_SIZE   = 1024 * 1024;  // 1 MiB
static const unsigned int STREAM_BUF_SIZE = 2048;         // 2 KiB
static const unsigned int RECV_BUF_SIZE   = 512;          // 512 Bytes

class TCPServer
{
public:
    TCPServer(int port = 8888);
    ~TCPServer();

    bool start();

    void sendData(const uint8_t *data, size_t size);

private:
    uint8_t *recv_buf;

    const int port;

    void fn_server();
    void fn_sender();

    bool receive();

    atomic_int sck_client{-1};
    int sck_server = -1;

    condition_variable cv_sender;
    mutex mtx_sender;
    atomic_bool client_connected{false};
    CircularBuffer send_buf{SEND_BUF_SIZE};  // buffer guarded by mtx_sender

    unique_ptr<thread> thread_recv;
    atomic_bool thread_recv_terminated{false};

    unique_ptr<thread> thread_send;
    atomic_bool thread_send_terminated{false};

    MessageDecoder decoder;
};

class TCPStreamBuf : public std::streambuf
{

public:
    TCPStreamBuf(TCPServer *server) : server(server)
    {
        setp(buf, buf + STREAM_BUF_SIZE - 1);
    }

protected:
    void send()
    {
        std::ptrdiff_t len = pptr() - pbase();
        server->sendData((uint8_t *)pbase(), len);

        pbump(-len);
    }

private:
    TCPStreamBuf(const TCPStreamBuf &) = delete;
    TCPStreamBuf &operator=(const TCPStreamBuf &) = delete;

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
    TCPServer *server;
};

class TCPStream : public std::ostream
{
public:
    TCPStream(TCPServer *server) : buf(server) { rdbuf(&buf); }

private:
    TCPStreamBuf buf;
};

#endif /* SRC_COMMUNICATION_TCPSERVER_H */
