/*
 * discharge.c
 *
 *  Created on: Jun 3, 2025
 *      Author: cobus
 */

#include "discharge.h"

//Setpoints
//This is an inverse, when the voltage is low the reading is high, en the voltage is high, the reading is low.
//Discharge when the voltage is higher than this.
//The current 2000 voltage indicates a voltage of around 1.3V
const uint16_t inverterCutoff = 2000;
//110 reading is about 2v
const uint16_t dischargeDoneLevel = 110;


const uint16_t relayVoltageLevel = 3000;

//These are the pins the correspond to the discharge relays.
//TODO: Create and order array for this as well.

const GPIOPin RelayPins[] = { { GPIOC, GPIO_PIN_12 }, { GPIOC, GPIO_PIN_11 }, {
GPIOC, GPIO_PIN_10 }, { GPIOA, GPIO_PIN_15 }, { GPIOA, GPIO_PIN_12 }, { GPIOA,
GPIO_PIN_11 }, { GPIOA, GPIO_PIN_10 }, { GPIOA, GPIO_PIN_9 }, { GPIOA,
GPIO_PIN_8 }, { GPIOC, GPIO_PIN_9 }, { GPIOC, GPIO_PIN_8 }, { GPIOC,
GPIO_PIN_7 }, { GPIOC, GPIO_PIN_6 }, { GPIOB, GPIO_PIN_15 }, { GPIOB,
GPIO_PIN_14 }, { GPIOB, GPIO_PIN_13 }, { GPIOB, GPIO_PIN_12 }, { GPIOB,
GPIO_PIN_11 }, { GPIOB, GPIO_PIN_10 }, { GPIOB, GPIO_PIN_2 }, };

const GPIOPin AuxRelays[] = { { GPIOC, GPIO_PIN_5 }, { GPIOC, GPIO_PIN_4 } };

//Pins are A,B,C pins which represent 1, 2 and 4 bit values of the mux.
const GPIOPin muxPins[] = { { GPIOB, GPIO_PIN_4 }, { GPIOB, GPIO_PIN_3 }, {GPIOD,
GPIO_PIN_2 }, };

uint8_t lowVoltage;
uint8_t dischargeDone;

uint32_t readMuxChannel(uint8_t ch, ADC_HandleTypeDef *adc) {
	//This is the order that the lines come into the multiplexer.
	const uint8_t muxOrder[] = { 3, 0, 1, 2, 5, 7, 6, 4 };
	HAL_GPIO_WritePin(muxPins[0].port, muxPins[0].pin,
			(muxOrder[ch] & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(muxPins[1].port, muxPins[1].pin,
			(muxOrder[ch] & 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(muxPins[2].port, muxPins[2].pin,
			(muxOrder[ch] & 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_ADC_Start(adc);
	HAL_ADC_PollForConversion(adc, HAL_MAX_DELAY);
	return HAL_ADC_GetValue(adc);
}

void checkVoltages(uint32_t lines[20]) {
	// Set default values
	lowVoltage = 0;
	dischargeDone = 1;

	// Check if any voltage is above the threshold
	for (int i = 0; i <= 19; i++) {
		if (lines[i] > inverterCutoff) {
			lowVoltage = 1;
			break;
		}
	}

	// Check if all voltages are above the threshold
	for (int i = 0; i <= 19; i++) {
		if (lines[i] <= dischargeDoneLevel) {
			dischargeDone = 0;
			break;
		}
	}
}

void controlDischarge(uint32_t lines[20]) {
	for (int i = 0; i <= 19; i++) {
		if (lines[i] > relayVoltageLevel) {
			HAL_GPIO_WritePin(RelayPins[i].port, RelayPins[i].pin,
					GPIO_PIN_RESET);
		}
	}

	if (lowVoltage) {
		HAL_GPIO_WritePin(AuxRelays[0].port, AuxRelays[0].pin, GPIO_PIN_RESET);
	}
	if (dischargeDone) {
		HAL_GPIO_WritePin(AuxRelays[1].port, AuxRelays[1].pin, GPIO_PIN_SET);
	}

}

void initOutputs(void) {
	//All relays on
	for (int i = 0; i <= 19; i++) {
		HAL_GPIO_WritePin(RelayPins[i].port, RelayPins[i].pin, GPIO_PIN_SET);

	}
	//Inverter on
	HAL_GPIO_WritePin(AuxRelays[0].port, AuxRelays[0].pin, GPIO_PIN_SET);
	//Discharge done off
	HAL_GPIO_WritePin(AuxRelays[1].port, AuxRelays[1].pin, GPIO_PIN_RESET);
}

