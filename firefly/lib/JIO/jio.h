#ifndef JIO_H
#define JIO_H
#include "stm32f0xx_gpio.h"
/**
 *   \file jio.h
 *   \brief A convienient wrapper around stm32 gpio.
 *
 *  (C) Jack Hosemans 2016
 */
void JIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef GPIO_InitStruct);

/**
 *  \brief Named parameter wrapper around GPIO_Init
 *
 *  By default sets the given IO to input with a pulldown.
 *
 *  \param port Port to set
 *  \param .GPIO_Pin Pins to set to given settings
 */
#define JIO_setMode(port,...) JIO_Init(port,                           \
                                        (GPIO_InitTypeDef){             \
                                          .GPIO_Pin = 0,                \
                                            .GPIO_Mode = GPIO_Mode_IN,  \
                                            .GPIO_OType = GPIO_OType_PP, \
                                            .GPIO_Speed = GPIO_Speed_2MHz, \
                                            .GPIO_PuPd = GPIO_PuPd_DOWN, \
                                            __VA_ARGS__                 \
                                            })

/**
 *  \brief Named parameter wrapper around GPIO_Init, sets pins to output by default
 *
 *  By default sets the given IO to output, 50MHz speed
 *
 *  \param port Port to set
 *  \param .GPIO_Pin Pins to set to given settings
 */

#define JIO_setOut(port,...) JIO_Init(port,                            \
                                       (GPIO_InitTypeDef){              \
                                         .GPIO_Pin = 0,                 \
                                           .GPIO_Mode = GPIO_Mode_OUT,  \
                                           .GPIO_OType = GPIO_OType_PP, \
                                           .GPIO_Speed = GPIO_Speed_50MHz, \
                                           .GPIO_PuPd = GPIO_PuPd_NOPULL, \
					   __VA_ARGS__			\
                                           })

#endif /* JIO_H */
