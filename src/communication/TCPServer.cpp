/*
 *  Created on: Jul 22, 2018
 *      Author: Luca Erbetta
 */

#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <cerrno>

#include "TCPServer.h"
#include "logger.h"

using std::unique_lock;

TCPServer::TCPServer(int port) : port(port)
{
    recv_buf = new uint8_t[RECV_BUF_SIZE];
    memset(recv_buf, 0, RECV_BUF_SIZE);
}

TCPServer::~TCPServer() { delete[] recv_buf; }

bool TCPServer::start()
{
    int result;
    sockaddr_in addr_serv{0};

    sck_server = socket(AF_INET, SOCK_STREAM, 0);

    if (sck_server < 0)
    {
        log.err("Error opening server socket: %d, errno: %d", sck_server,
                errno);
        return false;
    }

    addr_serv.sin_family      = AF_INET;
    addr_serv.sin_addr.s_addr = INADDR_ANY;
    addr_serv.sin_port        = htons(port);

    result = bind(sck_server, (sockaddr *)&addr_serv, sizeof(addr_serv));
    if (result != 0)
    {
        log.err("Error binding socket: %d, errno: %d", result, errno);
        return false;
    }

    result = listen(sck_server, 1);
    if (result != 0)
    {
        log.err("Error listening: %d", result);
        return false;
    }

    // Run the receiver thread
    thread_recv = unique_ptr<thread>(new thread(&TCPServer::fn_server, this));
    thread_send = unique_ptr<thread>(new thread(&TCPServer::fn_sender, this));

    thread_recv.get()->detach();
    thread_send.get()->detach();

    return true;
}

void TCPServer::sendData(const uint8_t *data, size_t size)
{
    {
        unique_lock<mutex> l(mtx_sender);
        send_buf.put(data, size);
    }
    cv_sender.notify_one();
}

void TCPServer::fn_server()
{
    int result       = 0;
    socklen_t clilen = 0;
    sockaddr_in addr_client{0};

    while (true)
    {
        log.info("Waiting for client...");
        sck_client = accept(sck_server, (sockaddr *)&addr_client, &clilen);

        if (sck_client < 0)
        {
            log.err("Error accepting client connection: %d", result);
            continue;
        }
        log.info("Client connected.");

        client_connected = true;
        cv_sender.notify_one();

        while (receive())
            ;

        log.info("Client disconnected");
        client_connected = false;
        cv_sender.notify_one();
    }

    log.err("Server thread terminated. (sck: %d)", sck_server);
}

void TCPServer::fn_sender()
{
    while (true)
    {
        const size_t buf_size = 512;
        uint8_t buf[buf_size];
        size_t len;

        {
            unique_lock<mutex> l(mtx_sender);
            while (!client_connected || send_buf.currentSize() == 0)
            {
                cv_sender.wait(l);
            }
            len = send_buf.get(buf, buf_size);
        }

        send(sck_client.load(), buf, len, 0);
    }
}

bool TCPServer::receive()
{
    // bzero((void *)recv_buf, BUF_SIZE);
    int n = read(sck_client, recv_buf, RECV_BUF_SIZE);
    log.info("Read %d bytes.", n);
    if (n > 0)
    {
        decoder.decode(recv_buf, n);
    }
    return n > 0;
}
