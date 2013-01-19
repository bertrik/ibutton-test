#include <stdint.h>
#include "ds1961_sha.h"


//constants used in SHA computation
static const uint32_t KTN[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

// calculation used for the SHA MAC
static uint32_t NLF(uint32_t B, uint32_t C, uint32_t D, int n)
{
    if (n < 20)
        return ((B & C) | ((~B) & D));
    else if (n < 40)
        return (B ^ C ^ D);
    else if (n < 60)
        return ((B & C) | (B & D) | (C & D));
    else
        return (B ^ C ^ D);
}

//----------------------------------------------------------------------
// computes a SHA given the 64 byte MT digest buffer.  The resulting 5
// long values are stored in the given long array, hash.
//
// Note: This algorithm is the SHA-1 algorithm as specified in the
// datasheet for the DS1961S, where the last step of the official
// FIPS-180 SHA routine is omitted (which only involves the addition of
// constant values).
//
// 'MT'        - buffer containing the message digest
// 'hash'      - result buffer
//
void ComputeSHAVM(const uint32_t MT[], uint32_t hash[])
{
    uint32_t MTword[80];
    int i;
    uint32_t ShftTmp;
    uint32_t Temp;

    for (i = 0; i < 16; i++) {
        MTword[i] = MT[i];
    }

    for (; i < 80; i++) {
        ShftTmp = MTword[i - 3] ^ MTword[i - 8] ^ MTword[i - 14] ^ MTword[i - 16];
        MTword[i] = ((ShftTmp << 1) & 0xFFFFFFFE) | ((ShftTmp >> 31) & 0x00000001);
    }

    hash[0] = 0x67452301;
    hash[1] = 0xEFCDAB89;
    hash[2] = 0x98BADCFE;
    hash[3] = 0x10325476;
    hash[4] = 0xC3D2E1F0;

    for (i = 0; i < 80; i++) {
        ShftTmp = ((hash[0] << 5) & 0xFFFFFFE0) | ((hash[0] >> 27) & 0x0000001F);
        Temp = NLF(hash[1], hash[2], hash[3], i) + hash[4] + KTN[i / 20] + MTword[i] + ShftTmp;
        hash[4] = hash[3];
        hash[3] = hash[2];
        hash[2] = ((hash[1] << 30) & 0xC0000000) | ((hash[1] >> 2) & 0x3FFFFFFF);
        hash[1] = hash[0];
        hash[0] = Temp;
    }
}

//----------------------------------------------------------------------
// Converts the 5 long numbers that represent the result of a SHA
// computation into the 20 bytes (with proper byte ordering) that the
// SHA iButton's expect.
//
// 'hash'      - result of SHA calculation
// 'MAC'       - 20-byte, LSB-first message authentication code for SHA
//                iButtons.
//
void HashToMAC(const uint32_t hash[], uint8_t MAC[])
{
    uint32_t temp;
    int i, j, offset;

    //iButtons use LSB first, so we have to turn
    //the result around a little bit.  Instead of
    //result A-B-C-D-E, our result is E-D-C-B-A,
    //where each letter represents four bytes of
    //the result.
    for (j = 4; j >= 0; j--) {
        temp = hash[j];
        offset = (4 - j) * 4;
        for (i = 0; i < 4; i++) {
            MAC[i + offset] = (uint8_t) temp;
            temp >>= 8;
        }
    }
}

