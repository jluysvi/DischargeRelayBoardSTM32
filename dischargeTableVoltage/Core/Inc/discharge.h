/*
 * discharge.h
 *
 *  Created on: Jun 3, 2025
 *      Author: cobus
 */

#ifndef INC_DISCHARGE_H_
#define INC_DISCHARGE_H_

#include <stdint.h>
#include "main.h"

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;

} GPIOPin;

uint8_t checkVoltages(uint32_t lines[20]);
uint32_t readMuxChannel(uint8_t ch, ADC_HandleTypeDef *adc);
void controlDischarge(uint32_t lines[20]);
void initOutputs(void);
void relaysOn();

#endif /* INC_DISCHARGE_H_ */
