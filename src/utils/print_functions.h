/*
 *  Created on: Jul 24, 2018
 *      Author: Luca Erbetta
 */

#ifndef SRC_UTILS_PRINT_FUNCTIONS_H
#define SRC_UTILS_PRINT_FUNCTIONS_H

#include <cstdint>
#include <cstdio>
#include <cstring>

void print_buffer(uint8_t buf, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        printf(" %d", buf[i]);
    }
    printf("\n");
}
#endif /* SRC_UTILS_PRINT_FUNCTIONS_H */
