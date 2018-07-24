/*
 *  Created on: Jul 23, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_CIRCULAR_BUFFER_H
#define SRC_CIRCULAR_BUFFER_H

#include <cstdint>
#include <cstdio>
#include <cstring>

class CircularBuffer
{
public:
    CircularBuffer(size_t size) : size(size)
    {
        buffer = new uint8_t[size];
        memset(buffer, 0, size);
    }

    ~CircularBuffer() { delete[] buffer; }

    void put(uint8_t val) { put(&val, 1); }

    void put(const uint8_t* values, size_t length)
    {
        lastop = length;
        if (length == 0)
        {
            return;
        }
        // Truncate input data to fit the buffer
        if (length > size)
        {
            values += length - size;
            length = size;
        }

        unsigned int new_head = head + length;
        unsigned int t_tail   = tail >= head ? tail : tail + size;

        if (new_head > size)
        {
            unsigned int sz = size - head;

            memcpy(buffer + head, values, sz);         // From head to end
            memcpy(buffer, values + sz, length - sz);  // From start to new_head
        }
        else
        {
            memcpy(buffer + head, values,
                   new_head - head);  // From head to new_head
        }

        if (head <= t_tail && new_head > t_tail && !empty)
        {
            t_tail = new_head;
        }

        head  = new_head % size;
        tail  = t_tail % size;
        empty = false;
    }

    size_t get(uint8_t* val)
    {
        lastop = -1;
        if (empty)
        {
            return 0;
        }
        *val = buffer[tail++];
        tail %= size;
        empty = tail == head;
        return 1;
    }

    size_t get(uint8_t* buf, size_t length)
    {
        lastop = -length;

        if (empty)
        {
            return 0;
        }

        unsigned int t_head = head < tail ? head + size : head;

        size_t max_length = t_head == tail && !empty ? size : t_head - tail;

        unsigned int new_tail = tail + length;

        if (length >= max_length)
        {
            new_tail = tail + max_length;
            empty    = true;
            length   = max_length;
        }

        if (new_tail > size)
        {
            unsigned int sz = size - tail;
            memcpy(buf, buffer + tail, sz);
            memcpy(buf + sz, buffer, length - sz);
        }
        else
        {
            memcpy(buf, buffer + tail, length);
        }

        tail = new_tail % size;

        return length;
    }

    size_t currentSize() const
    {
        if (empty)
            return 0;

        if (head > tail)
        {
            return head - tail;
        }
        else
        {
            return head + size - tail;
        }
    }

    void print()
    {
        printf("CB (%d)\t LastOp: ", size);
        if (lastop > 0)
        {
            printf("put %d\n", lastop);
        }
        else if (lastop < 0)
        {
            printf("get %d\n", -lastop);
        }
        else
        {
            printf("none\n");
        }

        for (unsigned int i = 0; i < size; i++)
        {
            printf("\t%d", i);
        }
        printf("\n");
        for (unsigned int i = 0; i < size; i++)
        {
            printf("\t%d", buffer[i]);
        }
        printf("\n");
        for (unsigned int i = 0; i < size; i++)
        {
            if (i == tail)
            {
                printf("\tT");
            }
            else
            {
                printf("\t ");
            }
        }
        printf("\n");
        for (unsigned int i = 0; i < size; i++)
        {
            if (i == head)
            {
                printf("\tH");
            }
            else
            {
                printf("\t ");
            }
        }
        printf("\n");
        printf("Curr size: %d, empty: %s\n", currentSize(),
               empty ? "true" : "false");
    }

    const size_t totalSize() const { return size; }

    int lastop = 0;
    uint8_t* buffer;
    unsigned int head = 0, tail = 0;
    bool empty = true;
    const size_t size;
};

#endif /* SRC_CIRCULAR_BUFFER_H */
