#include "hardware.h"

static int CLOCKS_PER_US = 1;
static uint8_t CE_var;
void sysInit(){
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | 
				   SYSCTL_OSC_MAIN |SYSCTL_XTAL_16MHZ);	//on board OSC is slower (10Mhz)
	//more?
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
}


/* 
	INITIALIZES SPI FOR THE NRF, ACCORDING TO 
	PCB DESIGN
*/

void spiInit(){
	

/*
s function configures the pin mux that selects the peripheral function associated with a
particular GPIO pin. Only one peripheral function at a time can be associated with a GPIO
pin, and each peripheral function should only be associated with a single GPIO pin at a time
(despite the fact that many of them can be associated with more than one GPIO pin). To fully
configure a pin, a GPIOPinType∗() function should also be called.
*/	

//	GPIOPinConfigure(GPIO_PF0_SSI1RX);
	//GPIOPinConfigure(GPIO_PF1_SSI1TX);
	//GPIOPinConfigure(GPIO_PF3_SSIFSS);
	//GPIOPinConfigure(GPIO_PF2_SSI1CLK);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
//	GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	GPIOPinConfigure(GPIO_PA4_SSI0RX);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);


/*
SI pins must be properly configured for the SSI peripheral to function correctly. This
function provides a typical configuration for those pin(s); other configurations may work as well
depending upon the board setup (for example, using the on-chip pull-ups).
The pin(s) are specified using a bit-packed byte, where each bit that is set identifies the pin to
be accessed, and where bit 0 of the byte represents GPIO port pin 0, bit 1 represents GPIO
port pin 1, and so on.
*/
	//GPIOPinTypeSSI(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4  |
                   GPIO_PIN_2);

	/* These values will change.
	 * Specifically, should probably change clock rate
	 * Fields, respectfully:
	 * BaseReg
	 * Clock Supplied to SSI
	 * Phase + Polarity (see DS for more)
	 * Mode (Master or Slave)
	 * Bit rate (see DS for req)
	 * Frame size, between 4 and 16 bits inclusive
	*/
	//should be ss1
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 2000000, 8);
	SSIEnable(SSI0_BASE);
}

void gpioInit(){
	/*
		fields:
			PORT_BASE = which gpio port you're dealin w/
			PIN = BIT_MAP OF WHAT PINS U WANT. OR THESE PUPS
	*/

	//IRQ
	//ONLY FOR LAUNCHPAD, WANT TO BE A1
	//GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_2);
	//CS
	//ONLY FOR LAUNCHPAD
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_7);
	//CE
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
}

void csnHigh(){
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7, 1 << 7);
}

void csnLow() {
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7, 0);
}

void ceHigh() {
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 1 << 3);
	CE_var = 1;
}

void ceLow() {
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
	CE_var = 0;
}

uint8_t spiXmitByte(uint8_t data){
	SSIDataPut(SSI0_BASE, data);
	uint32_t temp;
	while(SSIBusy(SSI0_BASE)){}
	SSIDataGet(SSI0_BASE, &temp);
	temp &= 0xFF;
	return (uint8_t) temp;
}


//FIND BETTER WAY. THIS LEGIT WASTES CYCLES. 
void delayUs(uint32_t delay){
	SysCtlDelay(delay * 100);
}



uint8_t getCe(){
	return CE_var;
}
