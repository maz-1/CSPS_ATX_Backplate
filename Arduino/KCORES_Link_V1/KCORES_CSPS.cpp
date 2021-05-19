/*!
 * @file KCORES_CSPS.cpp
 *
 * This is a library for the CSPS PMBUS
 * Written by AlphaArea
 *
 * GPL license, all text here must be included in any redistribution.
 *
 */

#include "KCORES_CSPS.h"

// avoid PB7 PB6 
// SDA SCL
//TwoWire Wire2(PB11,PB10);

#define WIRE Wire

CSPS::CSPS(byte CSPS_addr, uint8_t sda, uint8_t scl)
{
  _CSPS_addr = CSPS_addr;
  _ROM_addr = CSPS_addr - 8;
  WIRE.setSDA(sda);
  WIRE.setSCL(scl);
  WIRE.begin();
}

CSPS::CSPS(byte CSPS_addr, byte ROM_addr, uint8_t sda, uint8_t scl)
{
  _CSPS_addr = CSPS_addr;
  _ROM_addr = ROM_addr;
  WIRE.setSDA(sda);
  WIRE.setSCL(scl);
  WIRE.begin();
}

String CSPS::getROM(byte addr, byte len)
{
  String rec;
  for (byte n = 0; n < len; n++)
    rec.concat((char)readROM(addr + n));
  return rec;
}

byte CSPS::readROM(byte dataAddr)
{
  byte rec = 0x00;
  WIRE.beginTransmission(_ROM_addr);
  WIRE.write(dataAddr);
  WIRE.endTransmission();
  WIRE.requestFrom(_ROM_addr, 1);
  if (WIRE.available())
    rec = WIRE.read();
  return rec;
}


uint32_t CSPS::readCSPSword(byte dataAddr)
{
  byte regCS = ((0xFF - (dataAddr + (_CSPS_addr << 1))) + 1) & 0xFF;
  uint32_t rec = 0xFFFF;
  WIRE.beginTransmission(_CSPS_addr);
  WIRE.write(dataAddr);
  WIRE.write(regCS);
  WIRE.endTransmission();
  WIRE.requestFrom(_CSPS_addr, 3);
  if (WIRE.available())
  {
    rec = WIRE.read();
    rec |= WIRE.read() << 8;
    //rec |= WIRE.read() << 16;
    return rec;
  }
  else
    return 0;
}

void CSPS::writeCSPSword(byte dataAddr, unsigned int data)
{
  byte valLSB = lowByte(data),
       valMSB = highByte(data);
  uint8_t cs = (_CSPS_addr << 1) + dataAddr + valLSB + valMSB;
  uint8_t regCS = ((0xFF - cs) + 1) & 0xff;
  //the checksum is the 'secret sauce'
  //writeInts = [reg, valLSB, valMSB, regCS]; //write these 4 bytes to i2c
  WIRE.beginTransmission(_CSPS_addr);
  WIRE.write(dataAddr);
  WIRE.write(valLSB);
  WIRE.write(valMSB);
  WIRE.write(regCS);
  WIRE.endTransmission();
}
