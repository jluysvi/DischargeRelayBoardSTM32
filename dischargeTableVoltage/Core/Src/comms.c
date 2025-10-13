/*
 * comms.c
 *
 *  Created on: Oct 13, 2025
 *      Author: cobus
 */

#include "comms.h"
#include "string.h"

#define START_CHAR 0x01
#define MSG_CHAR 0x7B
#define END_CHAR 0x04
#define BUF_SIZE 3

static uint8_t rx_buf[BUF_SIZE];
static uint8_t tx_buf[BUF_SIZE];
static uint8_t got_signal;

void initComms() {
	HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET);
	HAL_UART_Receive_DMA(&huart1, rx_buf, BUF_SIZE);
	tx_buf[0] = START_CHAR;
	tx_buf[1] = MSG_CHAR;
	tx_buf[2] = END_CHAR;
	got_signal = 0;
}

void commsTransmit() {
	HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit_DMA(&huart1, tx_buf, 3);
}

uint8_t gotSignal() {
	return got_signal;
}

static void commsProcess() {
	if (rx_buf[0] == START_CHAR && rx_buf[1] == MSG_CHAR
			&& rx_buf[2] == END_CHAR) {
		got_signal = 1;
	}
	memset(rx_buf, 0, 3);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		commsProcess();
		HAL_UART_Receive_DMA(&huart1, rx_buf, BUF_SIZE);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET);
	}
}

