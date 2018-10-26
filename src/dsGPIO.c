/*
 ***********************************************************************
 *
 *  dsGPIO.c - provides generic access to the GPIOs
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
 *
 * Restrictions: at this time, only GPIOs on 40 Pin P1 header 
 *              are supported
 *
 ***********************************************************************
 */

#include "dsGPIO.h"


// ==========================================================================
// --------------------         GPIO lookup table        --------------------
// ==========================================================================

    static struct _bcm_pin_map _p1[] = {
        // P1-1 - P1-2
        {  1, 255,  -1, NULL, NULL }, {  2, 255,  -1, NULL, NULL },
        {  3,   2,  -1, NULL, NULL }, {  4, 255,  -1, NULL, NULL },
        {  5,   3,  -1, NULL, NULL }, {  6, 255,  -1, NULL, NULL },
        {  7,   4,  -1, NULL, NULL }, {  8,  14,  -1, NULL, NULL },
        {  9, 255,  -1, NULL, NULL }, { 10,  15,  -1, NULL, NULL },
        { 11,  17,  -1, NULL, NULL }, { 12,  18,  -1, NULL, NULL },
        { 13,  27,  -1, NULL, NULL }, { 14, 255,  -1, NULL, NULL },
        { 15,  22,  -1, NULL, NULL }, { 16,  23,  -1, NULL, NULL },
        { 17, 255,  -1, NULL, NULL }, { 18,  24,  -1, NULL, NULL },
        { 19,  10,  -1, NULL, NULL }, { 20, 255,  -1, NULL, NULL },
        { 21,   9,  -1, NULL, NULL }, { 22,  25,  -1, NULL, NULL },
        { 23,  11,  -1, NULL, NULL }, { 24,   8,  -1, NULL, NULL },
        { 25, 255,  -1, NULL, NULL }, { 26,   7,  -1, NULL, NULL },
        { 27,   0,  -1, NULL, NULL }, { 28,   1,  -1, NULL, NULL },
        { 29,   5,  -1, NULL, NULL }, { 30, 255,  -1, NULL, NULL },
        { 31,   6,  -1, NULL, NULL }, { 32,  12,  -1, NULL, NULL },
        { 33,  13,  -1, NULL, NULL }, { 34, 255,  -1, NULL, NULL },
        { 35,  19,  -1, NULL, NULL }, { 36,  16,  -1, NULL, NULL },
        { 37,  26,  -1, NULL, NULL }, { 38,  20,  -1, NULL, NULL },
        // P1-39 - P1-40
        { 39, 255,  -1, NULL, NULL }, { 40,  21 , -1, NULL, NULL }
    };
 

// **************************************************************************
// static int mapFindBCM( uint8_t gpio )
// -----------------------------------------------------------------
//
// find the specific GPIO by searching the map table above
//
// Note: the map table has grown historically and is subject
//       to change in near future
// -----------------------------------------------------------------
//
// uint8_t pin    bcm no of pin
//
// -----------------------------------------------------------------
//
// returns index of map member on success, otherwise an error code
//
// **************************************************************************
static int mapFindBCM( uint8_t gpio )
{
    int i;

    for( i = 0; i < 40 && _p1[i].bcm != gpio; i++ ) 
        ;

    if( i < 40 )
    {
        return( i );
    }
    else
    {
        return( DSGPIO_ERROR_NO_SUCH_BCM_PIN );
    }
}


// **************************************************************************
// int pinLock( uint8_t pin, int mode )
// -----------------------------------------------------------------
//
// lock a specific GPIO by requesting a handle to it
//
// -----------------------------------------------------------------
//
// uint8_t pin    bcm no of pin
// int    mode    either INPUT or OUTPUT
//
// -----------------------------------------------------------------
//
// DSGPIO_ERROR_NO_ERROR on success, otherwise an error code
//
// **************************************************************************
int pinLock( uint8_t pin, int mode )
{
    int retVal = 0;
    int mapEntry;
    struct gpiohandle_request req;
    char *chrdev_name;
    int devfd;

    if( (mapEntry = retVal = mapFindBCM( pin )) >= 0 )
    {
        if( _p1[mapEntry].fd < 0 )
        {
            if( mode != DSGPIO_PIN_MODE_INPUT &&
                mode != DSGPIO_PIN_MODE_OUTPUT )
            {
                retVal = DSGPIO_ERROR_GPIO_MODE;
            }
            else
            {
                if( (asprintf(&chrdev_name, "/dev/%s", DSGPIO_GPIODEV)) >= 0 )
                {
                    if( (devfd = open(chrdev_name, 0)) >= 0 )
                    {
                        req.lineoffsets[0] = pin;
                        req.lines = 1;
                        strcpy(req.consumer_label, DSGPIO_CONSUMER_LABEL);
    
                        if( mode == DSGPIO_PIN_MODE_OUTPUT )
                        {
                            req.flags = GPIOHANDLE_REQUEST_OUTPUT;
                        }
                        else
                        {
                            req.flags = GPIOHANDLE_REQUEST_INPUT;
                        }
    
                        if( ioctl(devfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0 )
                        {
                            close(devfd);
                            free( chrdev_name );
                            retVal = DSGPIO_ERROR_REQUEST_LINE_HANDLE;
                        }
                        else
                        {
                            _p1[mapEntry].fd = req.fd;
                            close(devfd);
                            retVal = DSGPIO_ERROR_NO_ERROR;
                        }
                    }
                    else
                    {
                        retVal = DSGPIO_ERROR_OPEN_DEVICE;
                    }
                }
                else
                {
                    retVal = DSGPIO_ERROR_OUT_OF_MEMORY;
                }
            }
        }
        else
        {
            retVal = DSGPIO_ERROR_HANDLE_IN_USE;
        }
    }

    return(retVal);
}

// **************************************************************************
// int pinRelease( uint8_t pin )
// -----------------------------------------------------------------
//
// release a specific GPIO by simply close its handle
//
// -----------------------------------------------------------------
//
// uint8_t pin    bcm no of pin
//
// -----------------------------------------------------------------
//
// DSGPIO_ERROR_NO_ERROR on success, otherwise an error code
//
// **************************************************************************
int pinRelease( uint8_t pin )
{
    int retVal = 0;
    int mapEntry;

    if( (mapEntry = retVal = mapFindBCM( pin )) >= 0 )
    {
        if( _p1[mapEntry].fd < 0 )
        {
            retVal = DSGPIO_ERROR_PIN_NOT_LOCKED;
        }
        else
        {
            if( close(_p1[mapEntry].fd) < 0 )
            {
                retVal = DSGPIO_ERROR_PIN_RELEASE;
            }
            else
            {
                close( _p1[mapEntry].fd );
                retVal = DSGPIO_ERROR_NO_ERROR;
            }

            _p1[mapEntry].fd = -1;
        }
    }

    return(retVal);
}


// **************************************************************************
// int pinState( uint8_t pin, uint8_t action, int state )
// -----------------------------------------------------------------
//
// set a specific GPIO to DSGPIO_PIN_STATE_HIGH/DSGPIO_PIN_STATE_LOW
//  or return its value, depending on action
//
// -----------------------------------------------------------------
//
// uint8_t pin    bcm no of pin
// uint8_t action either DSGPIO_ACTION_SET_STATE or DSGPIO_ACTION_GET_STATE
// int state      either DSGPIO_PIN_STATE_HIGH or DSGPIO_PIN_STATE_LOW
//                is ignored, if action is DSGPIO_ACTION_GET_STATE
//
// -----------------------------------------------------------------
//
// DSGPIO_ERROR_NO_ERROR, DSGPIO_PIN_STATE_HIGH or DSGPIO_PIN_STATE_LOW
// on success, otherwise an error code
//
// **************************************************************************
int pinState( uint8_t pin, uint8_t action, int state )
{
    int retVal = 0;
    int mapEntry;
    struct gpiohandle_data data;

    if( (mapEntry = retVal = mapFindBCM( pin )) >= 0 )
    {
        if( _p1[mapEntry].fd < 0 )
        {
            retVal = DSGPIO_ERROR_PIN_NOT_LOCKED;
        }
        else
        {
            if( action != DSGPIO_ACTION_SET_STATE &&
                action != DSGPIO_ACTION_GET_STATE )
            {
                retVal = DSGPIO_ERROR_GPIO_ACTION;
            }
            else
            {

	        memset(&data.values, 0, sizeof(data.values));

                if( action == DSGPIO_ACTION_SET_STATE )
                {
                    if( state != DSGPIO_PIN_STATE_HIGH &&
                        state != DSGPIO_PIN_STATE_LOW )
                    {
                        retVal = DSGPIO_ERROR_GPIO_STATE;
                    }
                    else
                    {
                        if( state == DSGPIO_PIN_STATE_HIGH )
                        {
                            data.values[0] = 1;
                        }
                        else
                        {
                            data.values[0] = 0;
                        }

                        if( (retVal = ioctl(_p1[mapEntry].fd, 
                                            GPIOHANDLE_SET_LINE_VALUES_IOCTL, 
                                            &data)) < 0 )
                        {
                            retVal = DSGPIO_ERROR_SET_LINE_VALUES;
                        }
                        else
                        {
                            retVal = DSGPIO_ERROR_NO_ERROR;
                        }
                    }
                }
                else
                {
                    if( (retVal = ioctl(_p1[mapEntry].fd, 
                                        GPIOHANDLE_GET_LINE_VALUES_IOCTL, 
                                        &data)) < 0 )
                    {
                        retVal = DSGPIO_ERROR_GET_LINE_VALUES;
                    }
                    else
                    {
printf("data: %d\n", data.values[0]);

                        if( data.values[0] > 0 )
                        {
                            retVal = DSGPIO_PIN_STATE_HIGH;
                        }
                        else
                        {
                            retVal = DSGPIO_PIN_STATE_LOW;
                        }
                    }
                }
            }
        }
    }

    return( retVal );
}



// **************************************************************************
// static void* eventThread( void* pArg )
// -----------------------------------------------------------------
//
// a thread that is created for each event handler. It checks for
// the occurrance of the specified event(s) and calls the handler
// function
//
// -----------------------------------------------------------------
//
// void* pArg     pointer to thread related information. At this 
//                time this is stored in the map table, too.
//
// -----------------------------------------------------------------
//
// returns nothing
//
// **************************************************************************
static void* eventThread( void* pArg )
{
    struct _event_thread_arg* pData;
    struct gpioevent_data event;
    static uint64_t lastTimestamp = 0;

    if( (pData = (struct _event_thread_arg*) pArg) != NULL )
    {
fprintf(stdout, "eventThread: pData != NULL\n");
        while( 1 )
        {
fprintf(stdout, "eventThread: check for event\n");
            if( read(pData->linefd, &event, sizeof(event)) > 0 )
            {
fprintf(stdout, " timestamp %" PRIu64, event.timestamp);
fprintf(stdout, " diff %" PRIu64 " ", event.timestamp - lastTimestamp);
lastTimestamp = event.timestamp;

// 1540458114526859912 diff 1000486554

fprintf(stdout, "Event ");
                if( event.id & pData->eventFlags )
                {
fprintf(stdout, " MATCH ");
                    if( pData->callBack != NULL )
                    {
fprintf(stdout, "  ... calling handler\n");
                       pData->callBack( pData->pin, &event, pData->pUserData );
                    }
                    else
                    {
fprintf(stdout, " ... BUT NULL handler\n");
                    }
                }
            }
        }
    }

    return( NULL );
}


// **************************************************************************
// int pinHandler( uint8_t pin, uint8_t action, int event, 
//                 pinCallback_t cb, void* pData )
// -----------------------------------------------------------------
//
// install/clear an event handler for the specified event(s) on
// the given GPIO
//
// NOTE: in case of action is DSGPIO_ACTION_CLEAR_HANDLER, the
//       specified pin is unlocked, too ...
// -----------------------------------------------------------------
//
// uint8_t pin      bcm no of pin
// uint8_t action   either DSGPIO_ACTION_SET_HANDLER or 
//                         DSGPIO_ACTION_CLEAR_HANDLER
// int event        GPIOEVENT_EVENT_RISING_EDGE, 
//                  GPIOEVENT_EVENT_FALLING_EDGE or a combination
//                  of both 
// pinCallback_t cb pointer to a void function that will be called
//                  everytime a matching event occurs eg:
//                  void callBackFunc( uint8_t pin, 
//                            struct gpioevent_data* event, void* pData )
//                  pin and event data are arguments set by the handler
//                  thread.
//                  
// void* pData      pointer to extra user data that will be delivered
//                  to the callback function
//
// -----------------------------------------------------------------
//
// DSGPIO_ERROR_NO_ERROR, DSGPIO_PIN_STATE_HIGH or DSGPIO_PIN_STATE_LOW
// on success, otherwise an error code
//
// **************************************************************************
int pinHandler( uint8_t pin, uint8_t action, int event, pinCallback_t cb, void* pData )
{
    int retVal = 0;
    int mapEntry;
    struct gpioevent_request req;
    struct gpiohandle_data data;
    char *chrdev_name;
    int devfd;
    struct _event_thread_arg* pArgs;


    if( (mapEntry = retVal = mapFindBCM( pin )) >= 0 )
    {
        if( _p1[mapEntry].fd < 0 )
        {
            if( action == DSGPIO_ACTION_SET_HANDLER )
            {
                if( (asprintf(&chrdev_name, "/dev/%s", DSGPIO_GPIODEV)) >= 0 )
                {
                    if( (devfd = open(chrdev_name, 0)) >= 0 )
                    {
                        req.lineoffset = pin;
                        strcpy(req.consumer_label, DSGPIO_CONSUMER_LABEL);
    
                        if( action == DSGPIO_ACTION_SET_HANDLER )
                        {
                            req.eventflags = event;
                            req.handleflags = GPIOHANDLE_REQUEST_INPUT;
                            if(ioctl(devfd,GPIO_GET_LINEEVENT_IOCTL,&req) < 0)
                            {
                                close(devfd);
                                free( chrdev_name );
                                retVal = DSGPIO_ERROR_REQUEST_LINE_HANDLE;
                            }
                            else
                            {
                                close(devfd);
                                free( chrdev_name );

	                        ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL,
                                       &data);
	                        fprintf(stdout, "Initial line value: %d\n", data.values[0]);
                                _p1[mapEntry].fd = req.fd;

                                if( ( _p1[mapEntry].pCallback = 
                                    (pthread_t*) malloc(sizeof(pthread_t)) ) == NULL)
                                {
                                    retVal = DSGPIO_ERROR_OUT_OF_MEMORY;
                                }
                                else
                                {
                                    memset( (char*) _p1[mapEntry].pCallback, 
                                                 '\0', sizeof(pthread_t) );
                                    if( (pArgs = (struct _event_thread_arg*) malloc( 
                                        sizeof(struct _event_thread_arg))) == NULL )
                                    {
                                        retVal = DSGPIO_ERROR_OUT_OF_MEMORY;
                                    }
                                    else
                                    {
                                        memset( (char*) pArgs, '\0', 
                                            sizeof(struct _event_thread_arg) );

                                        pArgs->eventFlags = event;
                                        pArgs->linefd = _p1[mapEntry].fd;
                                        pArgs->pin = pin;
                                        pArgs->callBack = cb;

fprintf(stdout, "start thread ...");
                                        retVal  = pthread_create( _p1[mapEntry].pCallback, NULL, &eventThread, pArgs );

fprintf(stdout, "done. retVal = %d\n", retVal);

                                        retVal = DSGPIO_ERROR_NO_ERROR;
                                    }
                                }

                            }
                        }
                    }
                    else
                    {
                        retVal = DSGPIO_ERROR_OPEN_DEVICE;
                    }
                }
                else
                {
                    retVal = DSGPIO_ERROR_OUT_OF_MEMORY;
                }
            }
            else
            {
                if( action != DSGPIO_ACTION_CLEAR_HANDLER )
                {
                    retVal = DSGPIO_ERROR_GPIO_ACTION;
                }
            }
        }
        else
        {
            if( action == DSGPIO_ACTION_CLEAR_HANDLER )
            {
                pthread_cancel( *_p1[mapEntry].pCallback );
                pthread_join( *_p1[mapEntry].pCallback, NULL );

                free( (void*) _p1[mapEntry].pCallback);
                _p1[mapEntry].pCallback = NULL;

                free( (void*) _p1[mapEntry].pArgs);
                _p1[mapEntry].pArgs = NULL;

                retVal = pinRelease( pin );
            }
            else
            {
                if( action != DSGPIO_ACTION_SET_HANDLER )
                {
                    retVal = DSGPIO_ERROR_GPIO_ACTION;
                }
            }
        }
    }

    return(retVal);
}



