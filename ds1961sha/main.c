
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ds1961_sha.h"

void hexdump(uint8_t *data, int size, int modulo)
{
    int i, len;
    int addr = 0;

    while (size > 0) {
        if ((modulo > 0) && (size > modulo)) {
            len = modulo;
        } else {
            len = size;
        }
        size -= len;

        for (i = 0; i < len; i++) {
            printf("%02X", data[addr++]);
        }
        printf("\n");
    }
}


static void calcmac(uint8_t mac[], int addr, uint8_t pp[], uint8_t ss[], uint8_t ch[], uint8_t id[])
{
    uint32_t input[16];
    uint32_t hash[16];
    int i;
  
    input[0] = (ss[0] << 24) | (ss[1] << 16) | (ss[2] << 8) | ss[3];
    for (i = 0; i < 32; i += 4) {
        input[i/4 + 1] = (pp[i] << 24) | (pp[i + 1] << 16) | (pp[i + 2] << 8) | pp[i + 3];
    }
    input[9] = 0xFFFFFFFF;
    uint8_t mp = (8 << 3) | ((addr >> 5) & 7);
    input[10] = (mp << 24) | (id[0] << 16) | (id[1] << 8) | id[2];
    input[11] = (id[3] << 24) | (id[4] << 16) | (id[5] << 8) | id[6];
    input[12] = (ss[4] << 24) | (ss[5] << 16) | (ss[6] << 8) | ss[7];
    input[13] = (ch[0] << 24) | (ch[1] << 16) | (ch[2] << 8) | 0x80;
    input[14] = 0;
    input[15] = 0x1B8; 
    
    ComputeSHAVM(input, hash);
    HashToMAC(hash, mac);
}

static void doit(void)
{
    uint8_t secret[8];
    uint8_t mac[20];
    uint8_t identity[7];
    uint8_t challenge[3];
    uint8_t data[32];
 
    memset(data, 0, sizeof(data));
    memset(secret, 0, sizeof(secret));
    
    printf ("IBID='%s'\n", getenv( "IBID" ) );
    printf ("IBCHAL='%s'\n", getenv( "IBCHAL" ) );
    printf ("IBSEC='%s'\n", getenv( "IBSEC" ) );
    parseHexString( getenv( "IBID" ), identity , 8 );
    parseHexString( getenv( "IBCHAL" ), challenge , 3 );
    parseHexString( getenv( "IBSEC" ), secret , 8 );
    printf ("Identity='%s'\n", identity );
    printf ("Challenge='%s'\n", challenge );
    printf ("Secret='%s'\n", secret );

    calcmac(mac, 0, data, secret, challenge, identity);

    printf("mac :");
    hexdump(mac, sizeof(mac), 0);
}


int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    doit();
    
    return 0;
}

