#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "OneWire.h"

static bool WriteScratchPad(OneWire *ow, const uint8_t id[], uint16_t addr, const uint8_t data[])
{
    uint8_t buf[11];
    uint8_t crc[2];
    int len = 0;

    // reset and select
    if (!ow->reset()) {
        return false;
    }
    ow->select((uint8_t *)id);

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


static bool ReadScratchPad(OneWire *ow, const uint8_t id[], uint16_t *addr, uint8_t *es, uint8_t data[])
{
    uint8_t buf[12];
    uint8_t crc[2];
    int len = 0;

    // reset and select
    if (!ow->reset()) {
        return false;
    }
    ow->select((uint8_t *)id);

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

static bool ReadAuthPage(OneWire *ow, const uint8_t id[], uint16_t addr, uint8_t data[], uint8_t mac[])
{
    uint8_t buf[36];
    uint8_t crc[2];
    uint8_t status;
    int len = 0;

    // reset and select
    if (!ow->reset()) {
        return false;
    }
    ow->select((uint8_t *)id);

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
  if (!ow->reset()) {
    return false;
  }
  ow->select((uint8_t *)id);

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


bool ReadAuthWithChallenge(OneWire *ow, const uint8_t id[], uint16_t addr, const uint8_t challenge[], uint8_t data[], uint8_t mac[])
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

bool DS1961WriteSecret(OneWire *ow, const uint8_t id[], const uint8_t secret[])
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

