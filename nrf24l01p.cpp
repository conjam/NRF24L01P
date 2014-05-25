#include "SPI.h"
#include "nrf24l01p.h"

// TODO: 
// - sleep mode (PWR_BIT setzen, CE aus etc...)
// 



/*
//Read from or write to register from the NRF24L01+:
unsigned int NRF24::readRegister(byte reg) {
 // digitalWrite(CHIP_SELECT_PIN, LOW); // take the chip select low to select the device:
  SPI.transfer(CMD_R_REGISTER | reg & 0x1F); // drop status register
  SPI.transfer(0x00);
  
  unsigned int result = SPI.transfer(CMD_R_REGISTER | reg & 0x1F); 
//  digitalWrite(CHIP_SELECT_PIN, HIGH); // take the chip select high to de-select:
  
  return result;
}
*/

/*
// Sends a write command to NF24L01+
void NRF24::writeRegister(byte reg, byte value) {  
  digitalWrite(CHIP_SELECT_PIN, LOW); // take the chip select low to select the device:
  
  SPI.transfer(CMD_W_REGISTER | (reg & 0x1F));
  SPI.transfer(value);
  
  digitalWrite(CHIP_SELECT_PIN, HIGH); // take the chip select high to de-select:
}
*/

void NRF24::csnLow() {
  digitalWrite(CHIP_SELECT_PIN, LOW);
}

void NRF24::csnHigh() {
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}


uint8_t NRF24::readRegister(uint8_t reg, uint8_t* dataIn, uint8_t len) { 
  csnLow();
  
  uint8_t status = SPI.transfer(CMD_R_REGISTER | (0x1F & reg));
  for(int i = 0; i < len; i++)
    dataIn[i] = SPI.transfer(0x00);
    
  csnHigh();
  
  return status;
}

uint8_t NRF24::readRegister(uint8_t reg, uint8_t* dataIn) { 
  readRegister(reg, dataIn, 1);
}

uint8_t NRF24::writeRegister(uint8_t reg, uint8_t* dataOut, uint8_t len) {
  NRFDBG("NRF24::writeRegister(uint8_t reg, uint8_t* dataOut, uint8_t len)");
  
  csnLow();
      
  uint8_t status = SPI.transfer(CMD_W_REGISTER  | (0x1F & reg));
  for(int i = 0; i < len; i++)
    SPI.transfer(dataOut[i]);
  
  csnHigh();
  
  return 0; //status;
}

uint8_t NRF24::writeRegister(uint8_t reg, uint8_t* dataOut) {
  NRFDBG("NRF24::writeRegister(uint8_t reg, uint8_t* dataOut)");
  
  writeRegister(reg, dataOut, 1);
}

void NRF24::setMaskOfRegisterIfTrue(uint8_t reg, uint8_t mask, bool set) {
  uint8_t regContent;
  readRegister(reg, &regContent);
  
  if(set)
    setMask(&regContent, mask);
  else
    resetMask(&regContent, mask);
    
  writeRegister(reg, &regContent);
}

bool NRF24::shockburstIsEnabled(uint8_t pipeId) {
  uint8_t enaa;
  readRegister(REG_EN_AA, &enaa);
  return enaa & (1 << pipeId);
}

// NRF24 API

void NRF24::enableCRC(uint8_t numBytes) {
  uint8_t config;
  readRegister(REG_CONFIG, &config);
        
  switch(numBytes) {
    case 0:
      resetMask(&config, EN_CRC);
      resetMask(&config, CRCO);
      break;
    case 1:
      setMask(&config, EN_CRC);
      resetMask(&config, CRCO);
      break;
    case 2:
      setMask(&config, EN_CRC);
      setMask(&config, CRCO);
      break;
    default:
      break;
  }
    
  writeRegister(REG_CONFIG, &config);
}

void NRF24::powerUp(bool enable) {
  setMaskOfRegisterIfTrue(REG_CONFIG, PWR_UP, enable);
}

void NRF24::listenMode(bool enable) {
  setMaskOfRegisterIfTrue(REG_CONFIG, PRIM_RX, enable);
  digitalWrite(CHIP_ENABLE_PIN, enable ? HIGH : LOW);
}


// EN_AA

void NRF24::enableShockburst(byte pipeId, boolean enable) {
  setMaskOfRegisterIfTrue(REG_EN_AA, 1 << pipeId, enable);
}

// EN_RXADDR
void NRF24::enableDataPipe(byte pipeId, boolean enable) {
  setMaskOfRegisterIfTrue(REG_EN_RXADDR, 1 << pipeId, enable);
}

// SETUP_AW
void NRF24::setAddressWidth(uint8_t numBytes) {
  uint8_t setupaw = numBytes == 5 ? 3 : numBytes == 4 ? 2 : numBytes == 3 ? 1 : 0;
  writeRegister(REG_SETUP_AW, &setupaw);
}

// RF_CH
void NRF24::setRFChannel(uint8_t channel) {
  writeRegister(REG_RF_CH, &channel);
}

// RF_SETUP
void NRF24::setDataRate(uint8_t dataRate) {
  uint8_t rfSetup;
  readRegister(REG_RF_SETUP, &rfSetup);
  
  switch(dataRate) {
    case SPEED_250K:
      resetMask(&rfSetup, RF_DR_HIGH);
      setMask(&rfSetup, RF_DR_LOW);
      break;
      
    case SPEED_1M:
      resetMask(&rfSetup, RF_DR_HIGH);
      resetMask(&rfSetup, RF_DR_LOW);
      break;
      
    case SPEED_2M:
      setMask(&rfSetup, RF_DR_HIGH);
      resetMask(&rfSetup, RF_DR_LOW);
      break;
      
    default:
      break;
  }
  
  writeRegister(REG_RF_SETUP, &rfSetup);
}

void NRF24::setXmitPower(uint8_t powerLevel) {
  uint8_t rfSetup;
  readRegister(REG_RF_SETUP, &rfSetup);
  resetMask(&rfSetup, 3 << 1);
  
  switch(powerLevel) {  
    case RF_PWR_0:
      break;
    case RF_PWR_1:
      setMask(&rfSetup, 1 << 1);
      break;
    case RF_PWR_2:
      setMask(&rfSetup, 2 << 1);
      break;
    case RF_PWR_3:
      setMask(&rfSetup, 3 << 1);
      break;
    default:
      break;
  }
  
  writeRegister(REG_RF_SETUP, &rfSetup);
}


void NRF24::setRxAddress(uint8_t pipeId, uint8_t* rxAddr) {
  uint8_t addressWidth = getAddressWidths();
  
  switch(pipeId) {
    case 0: 
      writeRegister(REG_RX_ADDR_P0, rxAddr, addressWidth);
      break;
    case 1:
      writeRegister(REG_RX_ADDR_P1, rxAddr, addressWidth);
      break;
    case 2:
      writeRegister(REG_RX_ADDR_P2, rxAddr, 1);
      break;
    case 3:
      writeRegister(REG_RX_ADDR_P3, rxAddr, 1);
      break;
    case 4:
      writeRegister(REG_RX_ADDR_P4, rxAddr, 1);
      break;
    case 5:
      writeRegister(REG_RX_ADDR_P5, rxAddr, 1);
      break;
    default:
      break;
  }
}


void NRF24::setTxAddress(uint8_t* addr) {
  writeRegister(REG_TX_ADDR, addr, 5);
}

void NRF24::setPayloadSize(uint8_t pipeId, uint8_t size) {
  switch(pipeId) {
    case 0: 
      writeRegister(REG_RX_PW_P0, &size);
      break;
    case 1:
      writeRegister(REG_RX_PW_P1, &size);
      break;
    case 2:
      writeRegister(REG_RX_PW_P2, &size);
      break;
    case 3:
      writeRegister(REG_RX_PW_P3, &size);
      break;
    case 4:
      writeRegister(REG_RX_PW_P4, &size);
      break;
    case 5:
      writeRegister(REG_RX_PW_P5, &size);
      break;
    default:
      break;
  }
  
  writeRegister(REG_RX_PW_P0, &size);
}

uint8_t NRF24::getRxAddress(uint8_t pipeId, uint8_t* rxAddr) {
  uint8_t addressWidth = getAddressWidths();
  
  switch(pipeId) {
    case 0: 
      readRegister(REG_RX_ADDR_P0, rxAddr, addressWidth);
      break;
    case 1:
      readRegister(REG_RX_ADDR_P1, rxAddr, addressWidth);
      break;
    case 2:
      readRegister(REG_RX_ADDR_P1, rxAddr, addressWidth);
      readRegister(REG_RX_ADDR_P2, &rxAddr[addressWidth-1]);
      break;
    case 3:
      readRegister(REG_RX_ADDR_P1, rxAddr);
      readRegister(REG_RX_ADDR_P3, &rxAddr[addressWidth-1]);
      break;
    case 4:
      readRegister(REG_RX_ADDR_P1, rxAddr);
      readRegister(REG_RX_ADDR_P4, &rxAddr[addressWidth-1]);
      break;
    case 5:
      readRegister(REG_RX_ADDR_P1, rxAddr);
      readRegister(REG_RX_ADDR_P5, &rxAddr[addressWidth-1]);
      break;
    default:
      break;
  }
}

uint8_t NRF24::getTxAddress(uint8_t* txAddr) {
  readRegister(REG_TX_ADDR, txAddr, getAddressWidths());
}

bool NRF24::crcIsEnabled() {
  uint8_t config;
  readRegister(REG_CONFIG, &config);
  return config & EN_CRC;
}

uint8_t NRF24::getRFChannel() {
  uint8_t channel;
  readRegister(REG_RF_CH, &channel);
  return channel;
}

uint8_t NRF24::crcGetEncodingScheme() {
  uint8_t config;
  readRegister(REG_CONFIG, &config);
  return config & EN_CRC ? config & CRCO ? 2 : 1 :0;
}

bool NRF24::isPoweredOn() {
  uint8_t config;
  readRegister(REG_CONFIG, &config);
  return config & PWR_UP;
}

bool NRF24::dataPipeIsEnabled(uint8_t pipeId) {
  uint8_t rxaddr;
  readRegister(REG_EN_RXADDR, &rxaddr);
  return rxaddr & (1 << pipeId);
}

uint8_t NRF24::getAddressWidths() {
  uint8_t setupaw;
  readRegister(REG_SETUP_AW, &setupaw);
  return setupaw == 3 ? 5 : setupaw == 2 ? 4 : setupaw == 1 ? 3 : 0;
}

bool NRF24::dataIsAvailable() {
  uint8_t status;
  readRegister(REG_STATUS, &status);
  return status & RX_DR;
}

uint8_t NRF24::getDataRate() {
  uint8_t rfSetup;
  readRegister(REG_RF_SETUP, &rfSetup);
  
  if(rfSetup & RF_DR_LOW)
    return SPEED_250K;
  else if(rfSetup & RF_DR_HIGH)
    return SPEED_2M;
  else
    return SPEED_1M;
}

bool NRF24::isListening() {
  uint8_t rfConfig;
  readRegister(REG_CONFIG, &rfConfig);
  return rfConfig & PRIM_RX;
}

uint8_t NRF24::getPayloadSize(uint8_t pipeId) {
  uint8_t size;
  
  switch(pipeId) {
    case 0: 
      readRegister(REG_RX_PW_P0, &size);
      break;
    case 1:
      readRegister(REG_RX_PW_P1, &size);
      break;
    case 2:
      readRegister(REG_RX_PW_P2, &size);
      break;
    case 3:
      readRegister(REG_RX_PW_P3, &size);
      break;
    case 4:
      readRegister(REG_RX_PW_P4, &size);
      break;
    case 5:
      readRegister(REG_RX_PW_P5, &size);
      break;
    default:
      break;
  }
  
  return size;
}

void NRF24::init(uint8_t channel) {
  SPI.begin();
  pinMode(CHIP_ENABLE_PIN, OUTPUT);
  pinMode(CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(CHIP_ENABLE_PIN, LOW);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  
  setRFChannel(channel);
}
