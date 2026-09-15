/*
 * comms.h
 *
 *  Created on: Oct 13, 2025
 *      Author: cobus
 */

#ifndef INC_COMMS_H_
#define INC_COMMS_H_

#include <stdint.h>
#include "usart.h"
#include "main.h"

void initComms();
void commsTransmit();
uint8_t gotSignal();

#endif /* INC_COMMS_H_ */
