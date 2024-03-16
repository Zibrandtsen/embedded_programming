/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: emp.c
*
* PROJECT....: EMP
*
* DESCRIPTION: See module specification file (.h-file).
*
* Change Log:
*****************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 150228  MoH   Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "uart0.h"
#include "tmodel.h"

/*****************************    Defines    *******************************/
#define SERIAL_SET_CLOCK '1'
#define SERIAL_GET_CLOCK '2'

#define SERIAL_ERROR_INVALID_HOUR 1
#define SERIAL_ERROR_INVALID_MIN  2
#define SERIAL_ERROR_INVALID_SEC  3

/*****************************   Constants   *******************************/

/*****************************   Variables   *******************************/
// INT8U recived_bytes[6] = {};
// INT8U recived_bytes_len = sizeof(recived_bytes);


/*****************************   Functions   *******************************/
// transmit funcs
void uart0_transmit_byte(INT8U byte) {
    while ( !uart0_tx_rdy() );  // Wait for TX buffer to be empty
    uart0_putc(byte);        // Transmit character
}

void uart0_transmit_string(char *str, INT8U len) {
	INT8U i;
  	for (i = 0; i < len; i++) {
		uart0_transmit_byte(str[i]);
  	}
}

// receove funcs
INT8U uart0_receive_byte() {
  while ( !uart0_rx_rdy() );  // Wait for RX buffer to be full
  return uart0_getc();        // Read character
}

INT8U * uart0_read_message(INT8U* buffer, INT8U length) {
    INT8U i;
    for(i = 0; i < length; i++) {
        buffer[i] = uart0_receive_byte();
    }
    return buffer;
}


void uart0_task(){
    INT8U hour, min, sec;
    INT8U error = 0;
    if(uart0_rx_rdy()){

        switch (uart0_receive_byte())
        {
        case SERIAL_SET_CLOCK:            
            if (wait ( MUTEX_UART0 )){
                hour =  (uart0_receive_byte() - '0') * 10;
                hour += (uart0_receive_byte() - '0');

                min =  (uart0_receive_byte() - '0') * 10;
                min += (uart0_receive_byte() - '0');

                sec =  (uart0_receive_byte() - '0') * 10;
                sec += (uart0_receive_byte() - '0');

                signal( MUTEX_UART0 );
            }

            if (wait ( MUTEX_LCD_DISPLAY )) {     // MUTEX_SYSTEM_RTC

                if (0 <= hour && hour <= 23) {
                    if (0 <= min && min <= 59) {
                        if (0 <= sec && sec <= 59) {
                            put_msg_state( SSM_RTC_HOUR, hour );
                            put_msg_state( SSM_RTC_MIN, min );
                            put_msg_state( SSM_RTC_SEC, sec );
                            error = 0;
                        } else {
                            error = SERIAL_ERROR_INVALID_SEC;
                        }
                    } else {
                        error = SERIAL_ERROR_INVALID_MIN;
                    }
                } else {
                    error = SERIAL_ERROR_INVALID_HOUR;
                }


                signal( MUTEX_LCD_DISPLAY );
            }
            if (error) {
                if (wait( MUTEX_UART0 )) {
                    uart0_transmit_string("Error: Invalied time", 20);
                    signal( MUTEX_UART0 );
                }
            }

            break;
        case SERIAL_GET_CLOCK:
            if (wait( MUTEX_SYSTEM_RTC )){   // wait for RTC mutex
                hour = get_msg_state( SSM_RTC_HOUR );
                min = get_msg_state( SSM_RTC_MIN );
                sec = get_msg_state( SSM_RTC_SEC );
                signal( MUTEX_SYSTEM_RTC );  // release RTC mutex
            }

            if (wait ( MUTEX_UART0 )) {  // wait for UART0 TX mutex
                // Send command
                uart0_transmit_byte('2');

                uart0_transmit_byte( (hour / 10) + '0' );
                uart0_transmit_byte( (hour % 10) + '0' );

                uart0_transmit_byte( (min / 10) + '0' );
                uart0_transmit_byte( (min % 10) + '0' );

                uart0_transmit_byte( (sec / 10) + '0' );
                uart0_transmit_byte( (sec % 10) + '0' );
                signal( MUTEX_UART0 );   // release UART0 TX mutex
            }

            break;
        
        default:
            break;
        }
    }
}

/****************************** End Of Module *******************************/












