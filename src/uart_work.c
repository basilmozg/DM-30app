#include "uart_work.h"
#include "DEFINES.h"

#include <drivers/uart.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(uart_work);

const struct device *lpuart;

static K_MEM_SLAB_DEFINE(uart_slab, BUF_SIZE, 3, 4);

static uint16_t uart_index;
static uint16_t uart_index_global;
static uint8_t *data_array;

static void (*uart_callback_event)(uint8_t, uint16_t *);

void uart_answer(uint8_t *data, uint16_t len)
{
    uart_tx(lpuart, data, len, 10);
}

static void rx_resume()
{
    int err;
    uint8_t *buf;

    err = k_mem_slab_alloc(&uart_slab, (void **)&buf, K_NO_WAIT);
    if(err) LOG_ERR_GLOB("Failed to alloc slab");

    err = uart_rx_enable(lpuart, buf, BUF_SIZE, UART_WAIT_FOR_RX);
    if(err) LOG_ERR_GLOB("Failed to enable RX");
}

K_TIMER_DEFINE(rx_resume_timer, rx_resume, NULL);

static void rx_timeout()
{
    (*uart_callback_event)(RX_EVENT, &uart_index_global);
    uart_index_global = 0;
    memset(data_array, 0, BUF_SIZE_GLOBAL);
}

K_TIMER_DEFINE(rx_timeout_timer, rx_timeout, NULL);

static void uart_callback(const struct device *dev,
                          struct uart_event *evt,
                          void *user_data)
{
    struct device *uart = user_data;
    int err;
    switch (evt->type) 
    {
        case UART_TX_DONE:
                   // LOG_INF("UART_TX_DONE");
        break;

        case UART_TX_ABORTED:
            LOG_ERR_GLOB("Tx aborted");
        break;

        case UART_RX_RDY:
                    //LOG_INF("UART_RX_RDY");
            if((uart_index_global + evt->data.rx.len) < BUF_SIZE_GLOBAL)
            {
                memcpy(&data_array[uart_index_global], &evt->data.rx.buf[uart_index], evt->data.rx.len);
                uart_index_global = uart_index_global + evt->data.rx.len;
            }
            uart_index = uart_index + evt->data.rx.len;
            k_timer_start(&rx_timeout_timer, K_MSEC(UART_RX_TIMEOUT), K_NO_WAIT);
            break;

        case UART_RX_BUF_REQUEST:
        {
            LOG_INF_GLOB("UART_RX_BUF_REQUEST");
            uint8_t *buf;

            err = k_mem_slab_alloc(&uart_slab, (void **)&buf, K_NO_WAIT);
            //__ASSERT(err == 0, "Failed to allocate slab");

            err = uart_rx_buf_rsp(uart, buf, BUF_SIZE);
           // __ASSERT(err == 0, "Failed to provide new buffer");
        }
        break;

        case UART_RX_BUF_RELEASED:
            uart_index = 0;
            LOG_INF_GLOB("UART_RX_BUF_RELEASED");
            k_mem_slab_free(&uart_slab, (void **)&evt->data.rx_buf.buf);
        break;

        case UART_RX_DISABLED:
            LOG_INF_GLOB("UART_RX_DISABLED");
            uart_index = 0;
            uart_index_global = 0;
        break;

        case UART_RX_STOPPED:LOG_INF_GLOB("UART_RX_STOPPED");
            k_timer_start(&rx_resume_timer, K_MSEC(100), K_NO_WAIT); 
        break;
    }
}

void uart_init(uint8_t *data_array_, void *uart_callback_event_, const uint32_t uart_speed_preset)
{
    lpuart = device_get_binding("UART_1");
    int err;
    uint8_t *buf;
    const struct uart_config uart_cfg = {
        .baudrate = uart_speed_preset,
        .parity = UART_CFG_PARITY_NONE,
        .stop_bits = UART_CFG_STOP_BITS_1,
        .data_bits = UART_CFG_DATA_BITS_8,
        .flow_ctrl = UART_CFG_FLOW_CTRL_NONE
    };
    uart_configure(lpuart, &uart_cfg);
    err = k_mem_slab_alloc(&uart_slab, (void **)&buf, K_NO_WAIT);
    err = uart_callback_set(lpuart, uart_callback, (void *)lpuart);
    err = uart_rx_enable(lpuart, buf, BUF_SIZE, UART_WAIT_FOR_RX);

    data_array = data_array_;
    uart_callback_event = uart_callback_event_;
}