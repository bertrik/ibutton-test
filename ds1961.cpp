#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "OneWire.h"
#include "ds1961.h"

DS1961::DS1961(OneWire *oneWire)
{
  ow = oneWire;
}

static bool ResetAndSelect(OneWire *ow, const uint8_t id[])
{
  if (!ow->reset()) {
    return false;
  }
  ow->select((uint8_t *) id);
  
  return true;
}

static bool WriteScratchPad(OneWire *ow, const uint8_t id[], uint16_t addr, const uint8_t data[])
{
  uint8_t buf[11];
  uint8_t crc[2];
  int len = 0;

  // reset and select
  if (!ResetAndSelect(ow, id)) {
    return false;
  }

  // perform write scratchpad command
  buf[len++] = 0x0F;                  // Write Scratchpad command
  buf[len++] = (addr >> 0) & 0xFF;    // 2 byte target address
  buf[len++] = (addr >> 8) & 0xFF;    // 2 byte target address
  memcpy(buf + len, data, 8);
  len += 8;
  ow->write_bytes(buf, len);
  ow->read_bytes(crc, 2);

  return ow->check_crc16(buf, len, crc);
}

static bool RefreshScratchPad(OneWire *ow, const uint8_t id[], uint16_t addr, const uint8_t data[])
{
  uint8_t buf[11];
  uint8_t crc[2];
  int len = 0;

  // reset and select
  if (!ResetAndSelect(ow, id)) {
    return false;
  }

  // perform refresh scratchpad command
  buf[len++] = 0xA3;                  // Refresh Scratchpad command
  buf[len++] = (addr >> 0) & 0xFF;    // 2 byte target address
  buf[len++] = (addr >> 8) & 0xFF;    // 2 byte target address
  memcpy(buf + len, data, 8);
  len += 8;
  ow->write_bytes(buf, len);
  ow->read_bytes(crc, 2);

  return ow->check_crc16(buf, len, crc);
}

static bool ReadScratchPad(OneWire *ow, const uint8_t id[], uint16_t *addr, uint8_t *es, uint8_t data[])
{
  uint8_t buf[12];
  uint8_t crc[2];
  int len = 0;

  // reset and select
  if (!ResetAndSelect(ow, id)) {
    return false;
  }

  // send read scratchpad command
  buf[len++] = 0xAA;              // Read Scratchpad command
  ow->write_bytes(buf, len);

  // get TA0/1 and ES
  ow->read_bytes(buf + len, 3);
  len += 3;
  *addr = (buf[2] << 8) | buf[1];
  *es = buf[3];

  // get data
  ow->read_bytes(buf + len, 8);
  len += 8;
  memcpy(data, buf + 4, 8);

  // check CRC
  ow->read_bytes(crc, 2);
  return ow->check_crc16(buf, len, crc);
}

static bool CopyScratchPad(OneWire *ow, const uint8_t id[], uint16_t addr, uint8_t es, const uint8_t mac[])
{
  uint8_t buf[4];
  int len = 0;
  uint8_t status;

  // reset and select
  if (!ResetAndSelect(ow, id)) {
    return false;
  }

  // send copy scratchpad command + arguments
  buf[len++] = 0x0F;                  // Copy Scratchpad command
  buf[len++] = (addr >> 0) & 0xFF;    // 2 byte target address
  buf[len++] = (addr >> 8) & 0xFF;    // 2 byte target address
  buf[len++] = es;                    // es
  ow->write_bytes(buf, len, 1);       // write and keep powered

  // wait while MAC is calculated
  delayMicroseconds(1500);

  // send MAC
  ow->write_bytes(mac, 20);
  
  // wait 10 ms
  delay(10);
  ow->depower();
  
  // check final status byte
  status = ow->read();
  return (status == 0xAA);
}

static bool ReadAuthPage(OneWire *ow, const uint8_t id[], uint16_t addr, uint8_t data[], uint8_t mac[])
{
  uint8_t buf[36];
  uint8_t crc[2];
  uint8_t status;
  int len = 0;

  // reset and select
  if (!ResetAndSelect(ow, id)) {
    return false;
  }

  // send command
  buf[len++] = 0xA5;                  // Read Authenticated Page command
  buf[len++] = (addr >> 0) & 0xFF;
  buf[len++] = (addr >> 8) & 0xFF;
  ow->write_bytes(buf, len);

  // read data part + 0xFF
  ow->read_bytes(buf + len, 33);
  len += 33;
  if (buf[35] != 0xFF) {
    return false;
  }
  ow->read_bytes(crc, 2);
  if (!ow->check_crc16(buf, len, crc)) {
    return false;
  }
  memcpy(data, buf + 3, 32);

  // read mac part
  delayMicroseconds(1500);
  ow->read_bytes(mac, 20);
  ow->read_bytes(crc, 2);
  if (!ow->check_crc16(mac, 20, crc)) {
    return false;
  }

  // check final status byte
  status = ow->read();
  return (status == 0xAA);
}

static bool LoadFirstSecret(OneWire *ow, const uint8_t id[], uint16_t addr, uint8_t es)
{
  uint8_t status;
  
  // reset and select
  if (!ResetAndSelect(ow, id)) {
    return false;
  }

  // write auth code
  ow->write(0x5A);
  ow->write((addr >> 0) & 0xFF);
  ow->write((addr >> 8) & 0xFF);
  ow->write(es, 1);
  delay(10);
  ow->depower();
  
  status = ow->read();
  return (status == 0xAA);
}

static bool ReadMemory(OneWire *ow, const uint8_t id[], int addr, int len, uint8_t data[])
{
  // reset and select
  if (!ResetAndSelect(ow, id)) {
     return false;
  }
  
  // write command/addr
  ow->write(0xF0);
  ow->write((addr >> 0) & 0xFF);
  ow->write((addr >> 8) & 0xFF);
  
  // read data
  for (int i = 0; i < len; i++) {
    data[i] = ow->read();
  }
  
  return true;
}

bool DS1961::ReadAuthWithChallenge(const uint8_t id[], uint16_t addr, const uint8_t challenge[], uint8_t data[], uint8_t mac[])
{
  uint8_t scratchpad[8];

  // put the challenge in the scratchpad
  memset(scratchpad, 0, sizeof(scratchpad));
  memcpy(scratchpad + 4, challenge, 3);
  if (!WriteScratchPad(ow, id, addr, scratchpad)) {
//        Serial.println("WriteScratchPad failed!");
    return false;
  }

  // perform the authenticated read
  if (!ReadAuthPage(ow, id, addr, data, mac)) {
//        Serial.println("ReadAuthPage failed!");
    return false;
  }

  return true;
}

bool DS1961::WriteSecret(const uint8_t id[], const uint8_t secret[])
{
  uint16_t addr;
  uint8_t es;
  uint8_t data[8];
  
  // write secret to scratch pad
  if (!WriteScratchPad(ow, id, 0x80, secret)) {
//    Serial.println("WriteScratchPad failed!");
    return false;
  }
  
  // read scratch pad for auth code
  if (!ReadScratchPad(ow, id, &addr, &es, data)) {
//    Serial.println("ReadScratchPad failed!");
    return false;
  }
  if (!LoadFirstSecret(ow, id, addr, es)) {
//    Serial.println("LoadFirstSecret failed!");
    return false;
  }

  return true;
}

/*
 * Writes 8 bytes of data to specified address
 */
bool DS1961::WriteData(const uint8_t id[], int addr, const uint8_t data[], const uint8_t mac[])
{
  uint8_t spad[8];
  uint16_t ad;
  uint8_t es;
  
  // write data into scratchpad
  if (!WriteScratchPad(ow, id, addr, data)) {
    Serial.println("WriteScratchPad failed!");
    return false;
  }
  
  // read scratch pad for auth code
  if (!ReadScratchPad(ow, id, &ad, &es, spad)) {
    Serial.println("ReadScratchPad failed!");
    return false;
  }
  
  // copy scratchpad to EEPROM
  if (!CopyScratchPad(ow, id, ad, es, mac)) {
    Serial.println("CopyScratchPad failed!");
    return false;
  }
  
  // refresh scratchpad
  if (!RefreshScratchPad(ow, id, addr, data)) {
    Serial.println("RefreshScratchPad failed!");
    return false;
  }
  
  // re-write with load first secret
  if (!LoadFirstSecret(ow, id, addr, es)) {
    Serial.println("LoadFirstSecret failed!");
    return false;
  }
  
  return true;
}

