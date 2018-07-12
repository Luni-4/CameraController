/*
 *  Created on: Jul 13, 2018
 *      Author: Luca Erbetta
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG
#define D(x) x;
#else
#define D(x)
#endif

#endif /* DEBUG_H_ */
