#ifndef _DS1961_H_
#define _DS1961_H_

#include <stdbool.h>
#include <stdint.h>

#include "OneWire.h"

class DS1961 {

public:
  DS1961(OneWire *oneWire);

  bool WriteSecret(const uint8_t id[], const uint8_t secret[]);
  bool ReadAuthWithChallenge(const uint8_t id[], uint16_t addr, const uint8_t challenge[], uint8_t data[], uint8_t mac[]);
  bool WriteData(const uint8_t id[], int addr, const uint8_t data[], const uint8_t mac[]);

private:
  OneWire *ow;

};

#endif /* _DS1961_H_ */
