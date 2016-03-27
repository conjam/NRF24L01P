
#include "nrf24l01p.h"

char rx_bf[33];
uint8_t rxaddr[] = {0x69, 0x69, 0x69, 0x69, 0x69};
uint8_t txaddr[] = {0x69, 0x69, 0x69, 0x69, 0x69};

int main() {
		sysInit();
		int dad = SysCtlClockGet();
		nrf24_init(81);

		nrf24_setRxAddress(PIPE_0, rxaddr);
		nrf24_setTxAddress(txaddr);
		nrf24_enableCRC(CRC_MODE_OFF);
		nrf24_enableDataPipe(PIPE_0, TRUE);
		nrf24_enableShockburst(PIPE_0, FALSE);
		nrf24_setAddressWidth(5);
		nrf24_listenMode(1);
		nrf24_setDataRate(SPEED_2M);
		nrf24_setPayloadSize(PIPE_0, 32);
		nrf24_powerUp(1);



		char* message = "this works, you can go to bed!!!";
		char* message2 = "really, go home james";
		while(1) {
				nrf24_sendPacket(message, 32, FALSE);
				delayUs(50000);
				nrf24_sendPacket(message2, 32, FALSE);
				delayUs(50000);
		}
}
