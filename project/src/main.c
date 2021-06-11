/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <string.h>
#include <drivers/uart.h>
#include <stdlib.h>
#include <stdio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000
#define SEMIHOSTING 1

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)


#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED1	""
//#define PIN	0
//#define FLAGS	0

#endif

#define GPS_UART "UART_1"
#define WIFI_UART "UART_4"
#define NBIOT_UART "UART_5"
#define FPS_UART "UART_2"

/*9static struct k_thread tx_thread_data;*/
static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);
 char uart_buf[1024];
struct uart_data_t {
	void  *fifo_reserved;
	unsigned char    data[1024];
	unsigned short   len;
};



 void uart_send(struct device *uart, char *Cont){
    unsigned short len = strlen(Cont); 
         uart_fifo_fill(uart, Cont,len );
         uart_irq_tx_enable(uart);
         
     
}
void uart_callback(struct device *uart)
{
	uart_irq_update(uart);
	int data_length = 0;

	if (uart_irq_rx_ready(uart)) {
                data_length = uart_fifo_read(uart, uart_buf, sizeof(uart_buf));
		uart_buf[data_length] = 0;
	}
	#if SEMIHOSTING
	printf("%s", uart_buf);
	#endif

       	if (uart_irq_tx_ready(uart)) {
               
        	struct uart_data_t *buf =
			k_fifo_get(&fifo_uart_tx_data, K_NO_WAIT);
		unsigned short written = 0;

		 /*Nothing in the FIFO, nothing to send */
		if (!buf) {
			uart_irq_tx_disable(uart);
			return;
		}

		while (buf->len > written) {
			written += uart_fifo_fill(uart,
						  &buf->data[written],
						  buf->len - written);
		}

		while (!uart_irq_tx_complete(uart)) {
			 /*Wait for the last byte to get
			  shifted out of the module*/
			 
		}

		if (k_fifo_is_empty(&fifo_uart_tx_data)) {
			uart_irq_tx_disable(uart);
		}

		k_free(buf);
	
	
	}

       
}

void main(void)
{
	struct device  * uart_GPS, *uart_WIFI,  *uart_NBIOT,  *uart_FPS;
	const struct device *dev_led0,*dev_led1;
	bool led_is_on = true;
	int ret_led0,ret_led1;
	volatile int vble = 0;
	printf("Hola");
	dev_led0 = device_get_binding(LED0);
	dev_led1 = device_get_binding(LED1);

	if (dev_led0 == 0 || dev_led1 == 0 ) {
		#if SEMIHOSTING
			printf("Cannot bind led!\n");
		#endif
		return;
	}
	uart_GPS = device_get_binding(GPS_UART);
	uart_WIFI = device_get_binding(WIFI_UART);
	//uart_NBIOT = device_get_binding(NBIOT_UART);
	uart_FPS = device_get_binding(FPS_UART);

	if(!uart_GPS || !uart_WIFI || !uart_FPS){
		gpio_pin_set(dev_led0, PIN, 1);
		k_msleep(SLEEP_TIME_MS);
		gpio_pin_set(dev_led0, PIN, 0);
		#if SEMIHOSTING
			printf("Cannot find  uart!\n");
		#endif
        	return;
	}
	
	
	ret_led0 = gpio_pin_configure(dev_led0, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	ret_led1 = gpio_pin_configure(dev_led1, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1);

	if (ret_led0 < 0 || ret_led1 ){
		return; 
	}
	
	//uart_send(uart_GPS,"$PMTK101*32\r\n");
	//uart_send(uart_WIFI, "AT\r\n");
	while (1) {
		vble++;
		uart_irq_rx_enable(uart_WIFI);
		uart_irq_callback_set(uart_WIFI,uart_callback);
		uart_send(uart_WIFI, "AT\r\n");
		//gpio_pin_set(dev_led0, PIN, (int)led_is_on);
		#if SEMIHOSTING
			printf("RX has stopped due to external event.\n");
		#endif
		led_is_on = !led_is_on;	
		gpio_pin_set(dev_led0, PIN, 1);
		k_msleep(SLEEP_TIME_MS);
		gpio_pin_set(dev_led0, PIN, 0);
		#if SEMIHOSTING
			printf("hello %d \n", vble);
		#endif
		k_msleep(SLEEP_TIME_MS);

	}
}
