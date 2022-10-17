#ifndef UART_WORK_H__
#define UART_WORK_H__

#include <stdint.h>
#include <inttypes.h>
#include <zephyr.h>

void uart_answer(uint8_t *data, uint16_t len);

void uart_init(uint8_t *data_array_, void *uart_callback_event_, const uint32_t uart_speed_preset);

/*UART*/
#define BUF_SIZE 500
#define BUF_SIZE_GLOBAL 10000

/*UART definitions*/
#define UART_RX_TIMEOUT                 200
#define UART_WAIT_FOR_RX                100

enum
{
    TX_EVENT,
    RX_EVENT
};
#endif