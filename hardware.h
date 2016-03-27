/*
 * Generic Hardware interface for nRF2401l.h
 * by conjam, adapted from coon's nRF2401l lib
 *
 * 
 * Designed such that if you impliment these functions correctly
 * the nRF2401 library associated with this file (written by coon, modified by conjam)
 * will function. 
 *
 *
 */


#include "time.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

/* Initialize your system:
 * Set Clock speed, enable any debug interfaces, configure your
 * MCU such that it is "on". Examples can be seen in the TM4C123GH6PM and SmartFusion folders
 * Can often be left blank
 */
void sysInit();

/* Initialize a SPI module:
 * Clock Speed can be touchy depending on MCU. 
 * nRF2401l+ can cap out as slow as 4 MHz, so start at 2Mhz and scale up.
 *   TM4C: 10 MHZ
 * 	 SFusion: 2 MHz
 *  
 *  Frame Size NEEDS to be 8 bits long
 *  You need NOT establish a CS line. One should use a GPIO as CS in order to 
 *  adhere to nRF's communication protocol.
 *
 *  Clk Polarity is zero, Clk Phase is zero.
 *  This means Clock mode is zero!!
 *
 */

void spiInit(); 


/* Sets the CS line low
 * CS needs to be  configured as a GPIO
 */

void csnLow();

/* Sets the CS line high
 * CS needs to be configured as GPIO
 */
void csnHigh();

/* Initializes all GPIO ports, 
   allows you to write to CS, CE and IRQ line 
*/ 
  void gpioInit();

/* Delays for N microseconds 
   Doesn't have to be that accurate
   but avoid clock() 
*/
  void delayUs(uint32_t delay);
  

/* Full Duplex SPI tranmission.
   Occurs on the spi line initialized in the 
   SPI_INIT() function
*/
  uint8_t spiXmitByte(uint8_t data);
 
/* Sets CE line high, CE must be init'd 
   in gpioInit() */
  void ceHigh();

/* Sets CE line low, CE must be init'd in gpioInit() */
  void ceLow();

/* gets the status of the CE pin */ 
  uint8_t getCe();

/* enables the interrupt for the IRQ pin. this will WILDLY vary between implimentations.
   TODO: impliment in SF and TM4C
*/  
  void initIRQ();


/* TODO: impliment in SF and TM4C
   ISR for receiving message. Gets thrown when IRQ goes high 
*/
__attribute__ ((interrupt)) void nRFrecv_hanlder(void);
