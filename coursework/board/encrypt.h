/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __ENCRYPT_H
#define __ENCRYPT_H

#include  <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/aes.h>

typedef uint8_t aes_gf28_t;
typedef uint32_t aes_gf28_row_t;
typedef uint32_t aes_gf28_col_t;

aes_gf28_t xtime(aes_gf28_t a);
aes_gf28_t aes_gf28_mul(aes_gf28_t a, aes_gf28_t b);
aes_gf28_t aes_gf28_inv(aes_gf28_t a);
void aes_enc_exp_step(aes_gf28_t* rk, aes_gf28_t rc);
void aes_enc_rnd_key(aes_gf28_t* s, const aes_gf28_t* rk);
void aes_enc_rnd_sub(aes_gf28_t* s);
void aes_enc_rnd_row(aes_gf28_t* s);
void aes_enc_rnd_mix(aes_gf28_t* s, aes_gf28_t* r);
void aes_enc(uint8_t* c, uint8_t* m, uint8_t* k);

#endif
