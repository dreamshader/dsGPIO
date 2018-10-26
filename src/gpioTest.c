

#include "dsGPIO.h"

#define MYPIN    18

void callBackFunc( uint8_t pin, struct gpioevent_data* event, void* pData )
{
printf("Callback!\n");
}


int main( int argc, char* argv[] )
{
    int exitCode = 0;
    uint8_t pin = MYPIN;

    if( (exitCode = pinLock( pin, DSGPIO_PIN_MODE_OUTPUT )) >= 0 )
    {

        if( exitCode = pinState( pin, DSGPIO_ACTION_SET_STATE, 
                                   DSGPIO_PIN_STATE_HIGH ) >= 0 )
        {

            exitCode = pinState( pin, DSGPIO_ACTION_GET_STATE, 0 );
printf("get state: %d\n", exitCode );
        }
        else
        {
fprintf(stderr, "set state failed!\n");
        }

        pinRelease( pin );

        exitCode =  pinHandler( pin, DSGPIO_ACTION_SET_HANDLER, 
                    GPIOEVENT_REQUEST_RISING_EDGE | 
                    GPIOEVENT_REQUEST_FALLING_EDGE, &callBackFunc, NULL );
        sleep(8);

        exitCode = pinHandler(pin, DSGPIO_ACTION_CLEAR_HANDLER, 0, NULL, NULL);

printf("handler cleared[%d] ...\n", exitCode);

        sleep(3);

        pinRelease( pin );

    }
    else
    {
fprintf(stderr, "lock failed!\n");
    }
printf("ends with exitcode %d\n", exitCode);
    return( exitCode );
}

