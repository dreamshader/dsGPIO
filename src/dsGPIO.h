/*
 ***********************************************************************
 *
 *  dsGPIO.h - definitions/declarations for generic GPIO access
 *
 *  Copyright (C) 2018 Dreamshader (aka Dirk Schanz)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***********************************************************************
 */

#ifndef _DSGPIO_H_
#define _DSGPIO_H_


#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/gpio.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif
 
using namespace std;


#define DSGPIO_ERROR_NO_ERROR               0
#define DSGPIO_ERROR_NO_SUCH_BCM_PIN       -1
#define DSGPIO_ERROR_REQUEST_LINE_HANDLE   -2
#define DSGPIO_ERROR_HANDLE_IN_USE         -3
#define DSGPIO_ERROR_GPIO_MODE             -4
#define DSGPIO_ERROR_PIN_NOT_LOCKED        -5
#define DSGPIO_ERROR_PIN_RELEASE           -6
#define DSGPIO_ERROR_OPEN_DEVICE           -7
#define DSGPIO_ERROR_OUT_OF_MEMORY         -8
#define DSGPIO_ERROR_GPIO_STATE            -9
#define DSGPIO_ERROR_GPIO_ACTION          -10
#define DSGPIO_ERROR_SET_LINE_VALUES      -11
#define DSGPIO_ERROR_GET_LINE_VALUES      -12

#define DSGPIO_GPIODEV                     "gpiochip0"
#define DSGPIO_CONSUMER_LABEL              "dsGPIO"

#define DSGPIO_PIN_MODE_OUTPUT              1
#define DSGPIO_PIN_MODE_INPUT               2

#define DSGPIO_PIN_STATE_HIGH               1
#define DSGPIO_PIN_STATE_LOW                0
#define DSGPIO_PIN_STATE_NO_STATE          -1

#define DSGPIO_ACTION_SET_MODE             0b00000001
#define DSGPIO_ACTION_GET_MODE             0b00000010
#define DSGPIO_ACTION_SET_STATE            0b00000100
#define DSGPIO_ACTION_GET_STATE            0b00001000
#define DSGPIO_ACTION_SET_HANDLER          0b00010000
#define DSGPIO_ACTION_CLEAR_HANDLER        0b00100000


typedef void (*pinCallback_t) (uint8_t pin, struct gpioevent_data* event, void* pData);

struct _event_thread_arg {
    int   eventFlags;
    int  linefd;
    uint8_t  pin;
    pinCallback_t callBack;
    void *pUserData;
};

struct _bcm_pin_map {
    int phys;
    uint8_t bcm;
    int fd;
    pthread_t *pCallback;
    struct _event_thread_arg* pArgs;
};



int pinLock( uint8_t pin, int mode );
int pinRelease( uint8_t pin );
int pinState( uint8_t pin, uint8_t action, int state );
int pinHandler( uint8_t pin, uint8_t action, int event, pinCallback_t cb, void* pData );




#ifdef __cplusplus
}
#endif


#endif // _DSGPIO_H_




