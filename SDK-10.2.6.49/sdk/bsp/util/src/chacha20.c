/**
 ****************************************************************************************
 *
 * @file chacha20.c
 *
 * @brief chacha20 random generator implementation.
 *
 * CSPRNG based on http://cr.yp.to/chacha/chacha-20080128.pdf
 * Uses 16-byte key
 * Optimized for size
 *
 * Source code downloaded from https://gist.github.com/Emill/d8e8df7269f75b9485a2
 * Author  (Github user name): Emill
 *
 ****************************************************************************************
 */

#if dg_configUSE_SYS_DRBG

#if dg_configUSE_CHACHA20_RAND

#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"
#include "chacha20.h"

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

typedef struct {
        uint64_t counter;
        uint32_t key[4];
        uint32_t random_output[16];
        uint8_t random_output_left;
} chacha20_state_t;

/*
 * VARIABLE DECLARATIONS
 *****************************************************************************************
 */

#if (MAIN_PROCESSOR_BUILD)
__IN_CMAC_MEM1_UNINIT static chacha20_state_t chacha20_state;
#elif (SNC_PROCESSOR_BUILD)
static chacha20_state_t chacha20_state;
#endif /* PROCESSOR_BUILD */

/* This is ASCII of "expand 16-byte k" */
static const uint32_t chacha_constants[4] = {0x61707865, 0x3120646e, 0x79622d36, 0x6b206574};

static const uint8_t chacha_order[8][4] = {
    {0, 4, 8, 12},
    {1, 5, 9, 13},
    {2, 6, 10, 14},
    {3, 7, 11, 15},
    {0, 5, 10, 15},
    {1, 6, 11, 12},
    {2, 7, 8, 13},
    {3, 4, 9, 14}
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void chacha_quarterround(const uint8_t indices[4]) {
        uint32_t* s = chacha20_state.random_output;
        int a = indices[0];
        int b = indices[1];
        int c = indices[2];
        int d = indices[3];
        s[a] += s[b]; s[d] ^= s[a]; s[d] = (s[d] << 16) | (s[d] >> 16);
        s[c] += s[d]; s[b] ^= s[c]; s[b] = (s[b] << 12) | (s[b] >> 20);
        s[a] += s[b]; s[d] ^= s[a]; s[d] = (s[d] << 8) | (s[d] >> 24);
        s[c] += s[d]; s[b] ^= s[c]; s[b] = (s[b] << 7) | (s[b] >> 25);
}

static void chacha_run(void) {
        uint32_t state[16];
        memcpy(state, chacha_constants, sizeof(chacha_constants));
        memcpy(state + 4, chacha20_state.key, sizeof(chacha20_state.key));
        memcpy(state + 8, chacha20_state.key, sizeof(chacha20_state.key));
        memset(state + 12, 0, 2 * sizeof(uint32_t));
        ++chacha20_state.counter;
        memcpy(state + 14, &chacha20_state.counter, sizeof(chacha20_state.counter));

        memcpy(chacha20_state.random_output, state, sizeof(state));

        for(int i = 0; i < 10; i++) {
                for(int j = 0; j < 8; j++) {
                        chacha_quarterround(chacha_order[j]);
                }
        }

        for(int i=0; i<16; i++) {
                chacha20_state.random_output[i] += state[i];
        }
}

void csprng_seed(const uint8_t key[16]) {
        memcpy(chacha20_state.key, key, sizeof(chacha20_state.key));
        chacha20_state.counter = 0;
        chacha20_state.random_output_left = 0;
}

uint32_t csprng_get_next_uint32(void) {
        if (chacha20_state.random_output_left == 0) {
                chacha_run();
                chacha20_state.random_output_left = 16;
        }
        return chacha20_state.random_output[--chacha20_state.random_output_left];
}

#endif /* dg_configUSE_CHACHA20_RAND */

#endif /* dg_configUSE_SYS_DRBG */

