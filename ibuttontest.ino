#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "OneWire.h"

#include "ds1961.h"
#include "editline.c"
#include "hexutil.c"

OneWire ow(2);

void hexdump(uint8_t *data, int size, int modulo)
{
    int i, len;
    int addr = 0;
    uint8_t b;

    while (size > 0) {
        if ((modulo > 0) && (size > modulo)) {
            len = modulo;
        } else {
            len = size;
        }
        size -= len;

        for (i = 0; i < len; i++) {
            b = data[addr++];
            Serial.print((b >> 4) & 0xF, HEX);
            Serial.print((b >> 0) & 0xF, HEX);
        }
    }
}

static void DoPoll(uint8_t id[8])
{
  Serial.print("<");
  
  // identify
  ow.reset_search();
  if (ow.search(id)) {
    hexdump(id, 8, 0);
  }

  Serial.print(">\n");
}

static void DoReadAuthWithChallenge(const uint8_t id[], const char *chalstr)
{
  uint8_t challenge[] = {0, 0, 0};
  uint8_t data[32];
  uint8_t mac[20];
    
  if (!parseHexString(chalstr, challenge, sizeof(challenge))) {
//    Serial.println("E Parse challenge failed!");
    return;
  }
    
  if (!ReadAuthWithChallenge(&ow, id, 0, challenge, data, mac)) {
//    Serial.println("E ReadAuthWithChallenge failed!");
    return;
  }
  
  Serial.print("R ");
  hexdump(data, 32, 0);
  Serial.print(" ");
  hexdump(mac, 20, 0);
  Serial.print("\n");
}

static void DoWriteSecret(const uint8_t id[], const char *secretstr)
{
  uint8_t secret[8];
  
  if (!parseHexString(secretstr, secret, sizeof(secret))) {
//    Serial.println("E Parse secret failed!");
    return;
  }
  
  if (!DS1961WriteSecret(&ow, id, secret)) {
    Serial.println("E ReadAuthWithChallenge failed!");
    return;
  }
  
  Serial.println("OK");
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Hello World!");
}

static uint8_t id[8];
static char line[30];

void loop()
{
  bool ret;
  char c;
  
  // process incoming serial chars
  ret = false;
  while (Serial.available() && !ret) {
    ret = EditLine(Serial.read(), &c, line, sizeof(line));
    Serial.print(c);
  }
  
  // process line
  if (ret) {
    switch (line[0]) {
    case 'P':
      DoPoll(id);
      break;
    case 'C':
      DoReadAuthWithChallenge(id, &line[2]);
      break;
    case 'S':
      DoWriteSecret(id, &line[2]);
      break;
    case 'D':
      delay(10);
      break;
    case '\0':
      // ignore empty line
      break;
    default:
      Serial.println("Huh?");
      break;
    }
  }
}
