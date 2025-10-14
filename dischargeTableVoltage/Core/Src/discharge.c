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
static const uint16_t inverterCutoff = 2000;
//110 reading is about 2v
static const uint16_t dischargeDoneLevel = 110;


static const uint16_t relayVoltageLevel = 3000;
//These are the pins the correspond to the discharge relays.
//TODO: Create and order array for this as well.

static const GPIOPin RelayPins[] = { { GPIOC, GPIO_PIN_12 }, { GPIOC, GPIO_PIN_11 }, {
GPIOC, GPIO_PIN_10 }, { GPIOA, GPIO_PIN_15 }, { GPIOA, GPIO_PIN_12 }, { GPIOA,
GPIO_PIN_11 }, { GPIOA, GPIO_PIN_10 }, { GPIOA, GPIO_PIN_9 }, { GPIOA,
GPIO_PIN_8 }, { GPIOC, GPIO_PIN_9 }, { GPIOC, GPIO_PIN_8 }, { GPIOC,
GPIO_PIN_7 }, { GPIOC, GPIO_PIN_6 }, { GPIOB, GPIO_PIN_15 }, { GPIOB,
GPIO_PIN_14 }, { GPIOB, GPIO_PIN_13 }, { GPIOB, GPIO_PIN_12 }, { GPIOB,
GPIO_PIN_11 }, { GPIOB, GPIO_PIN_10 }, { GPIOB, GPIO_PIN_2 }, };

static const GPIOPin AuxRelays[] = { { GPIOC, GPIO_PIN_5 }, { GPIOC, GPIO_PIN_4 } };

//Pins are A,B,C pins which represent 1, 2 and 4 bit values of the mux.
static const GPIOPin muxPins[] = { { GPIOB, GPIO_PIN_4 }, { GPIOB, GPIO_PIN_3 }, {
		GPIOD,
		GPIO_PIN_2 }, };

static uint8_t lowVoltage;
static uint8_t dischargeDone;
static uint8_t relaysOnState = 0;
static uint32_t oldTime = 0;

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

uint8_t checkVoltages(uint32_t lines[20]) {
	// Set default values
	static lowVoltageFlag = 0;
	lowVoltage = 0;
	dischargeDone = 1;

	// Check if any voltage is above the threshold
	for (int i = 0; i <= 19; i++) {
		if (lines[i] > inverterCutoff) {
			lowVoltage = 1;
			lowVoltageFlag = 1;
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
	if(lowVoltageFlag)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

//Compare cell voltages with same voltages five seconds ago, if they have not changed and the relays are not on, then the inverter is not discharging.
uint8_t inverterOn(uint32_t lines[20]) {

	static uint32_t oldLines[20] = { 0 };
	uint8_t lowerCount = 0;

	if (HAL_GetTick() - oldTime > 5000) {
		oldTime = HAL_GetTick();

		for (int i = 0; i < 20; i++) {
			//If the new line analog value is bigger, e.g. voltage is lower, increment counter;
			if (lines[i] > oldLines[i]) {
				lowerCount++;
			}
			oldLines[i] = lines[i];
		}

	}
	//If more then 5 of the lines are lower now and the relays are not on, the inverter is on
	if(lowerCount > 5 && relaysOnState == 0)
	{
		return 1;
	}
	else
	{
		return 0;
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
		HAL_GPIO_WritePin(RelayPins[i].port, RelayPins[i].pin, GPIO_PIN_RESET);

	}
	//Inverter on
	HAL_GPIO_WritePin(AuxRelays[0].port, AuxRelays[0].pin, GPIO_PIN_SET);
	//Discharge done off
	HAL_GPIO_WritePin(AuxRelays[1].port, AuxRelays[1].pin, GPIO_PIN_RESET);
}

void relaysOn() {
	for (int i = 0; i <= 19; i++) {
		HAL_GPIO_WritePin(RelayPins[i].port, RelayPins[i].pin, GPIO_PIN_SET);

	}
	relaysOnState = 1;

}

