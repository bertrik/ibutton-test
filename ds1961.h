#include <stdbool.h>
#include <stdint.h>

#include "OneWire.h"

bool ReadAuthWithChallenge(OneWire *ow, const uint8_t id[], uint16_t addr, const uint8_t challenge[], uint8_t data[], uint8_t mac[]);
bool DS1961WriteSecret(OneWire *ow, const uint8_t id[], const uint8_t secret[]);

