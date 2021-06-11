#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <soc.h>

/* USER PUSH BUTTON*/
#define USER_PB_GPIO_PORT  "GPIOB"
#define USER_PB_GPIO_PIN  15

/* LD0 green LED*/
#define LG_GPIO_PORT  "GPIOB"
#define LG_GPIO_PIN  0

/* DEFINE ALIASES TO MAKE THE BASIC SAMPLES WORK*/
#define LED0_GPIO_PORT  LG_GPIO_PORT
#define LED0_GPIO_PIN  LG_GPIO_PIN

#endif 
