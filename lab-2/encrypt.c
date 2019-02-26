/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "encrypt.h"

aes_gf28_t rc[16] = {0x01, 0x02, 0x04, 0x08, 0x10,
                     0x20, 0x40, 0x80, 0x1B, 0x36};

int main(int argc, char* argv[]) {

  uint8_t k[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                   0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};

  uint8_t m[16] = {0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D,
                   0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34};

  uint8_t c[16] = {0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
                   0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32};

  uint8_t t[16];

  AES_KEY rk;

  AES_set_encrypt_key(k, 128, &rk);
  AES_encrypt(m, t, &rk);

  if (!memcmp(t, c, 16 * sizeof(uint8_t))) {
    printf("AES.Enc( k, m ) == c\n");
  } else {
    printf("AES.Enc( k, m ) != c\n");
  }

}

aes_gf28_t xtime(aes_gf28_t a) {
  if ((a & 0x80) == 0x80) {
    return 0x1B ^ (a << 1);
  }
  return (a << 1);
}

aes_gf28_t aes_gf28_mul(aes_gf28_t a, aes_gf28_t b) {
  aes_gf28_t t = 0;

  for (int i = 7; i >= 0; i--) {
    t = xtime(t);
    if ((b >> i) & 1) {
      t = t ^ a;
    }
  }

  return t;
}

aes_gf28_t aes_gf28_inv(aes_gf28_t a) {
  aes_gf28_t t_0 = aes_gf28_mul(  a,   a); // a^2
  aes_gf28_t t_1 = aes_gf28_mul(t_0,   a); // a^3
             t_0 = aes_gf28_mul(t_0, t_0); // a^4
             t_1 = aes_gf28_mul(t_1, t_0); // a^7
             t_0 = aes_gf28_mul(t_0, t_0); // a^8
             t_0 = aes_gf28_mul(t_1, t_0); // a^15
             t_0 = aes_gf28_mul(t_0, t_0); // a^30
             t_0 = aes_gf28_mul(t_0, t_0); // a^60
             t_1 = aes_gf28_mul(t_1, t_0); // a^67
             t_0 = aes_gf28_mul(t_0, t_1); // a^127
             t_0 = aes_gf28_mul(t_0, t_0); // a^254

  return t_0;
}

aes_gf28_t sbox(aes_gf28_t a) {
  a = aes_gf28_inv(a);

  a = ( 0x63   ) ^
      ( a      ) ^
      ( a << 1 ) ^
      ( a << 2 ) ^
      ( a << 3 ) ^
      ( a << 4 ) ^
      ( a >> 7 ) ^
      ( a >> 6 ) ^
      ( a >> 5 ) ^
      ( a >> 4 );

      return a;
}

void aes_enc_exp_step(aes_gf28_t* rk, aes_gf28_t rc) {
  aes_gf28_t temp[16];
  memcpy(temp, rk, 16);
}

void aes_enc_rnd_key(aes_gf28_t* s, aes_gf28_t* rk) {
  for (int i = 0; i < 16; i++) {
    s[i] = s[i] ^ rk[i];
  }
}

void aes_enc_rnd_sub(aes_gf28_t* s) {
  for (int i = 0; i < 16; i++) {
    s[i] = sbox(s[i]);
  }
}

void aes_enc_rnd_row(aes_gf28_t* s) {
  aes_gf28_t temp[16];
  memcpy(temp, s, 16);

  s[13] = temp[1];
  s[1]  = temp[5];
  s[5]  = temp[9];
  s[9]  = temp[13];

  s[10] = temp[2];
  s[14] = temp[6];
  s[2]  = temp[10];
  s[6]  = temp[14];

  s[7]  = temp[3];
  s[11] = temp[7];
  s[15] = temp[11];
  s[3]  = temp[15];
}

void aes_enc_rnd_mix(aes_gf28_t* s) {
  for (int i = 0; i < 4; i++, s += 4) {
    s[0] = xtime(s[0]) ^ (s[1] ^ xtime(s[1])) ^ s[2] ^ s[3];
    s[1] = s[0] ^ xtime(s[1]) ^ (s[2] ^  xtime(s[2])) ^ s[3];
    s[2] = s[0] ^ s[1] ^ xtime(s[2]) ^ (s[3] ^ xtime(s[3]));
    s[3] = (s[0] ^ xtime(s[0])) ^ s[1] ^ s[2] ^ xtime(s[3]);
  }
}

void aes_enc(uint8_t* c, uint8_t* m, uint8_t* k) {
  uint8_t Nr = 10;
  aes_gf28_t rk[16], s[16];

  aes_gf28_t* rkp = rk;

  memcpy(s  , m, 16);
  memcpy(rkp, k, 16);

  aes_enc_rnd_key(s, rkp);
  for (int i = 1; i < Nr; i++) {
    aes_enc_rnd_sub(s);
    aes_enc_rnd_row(s);
    aes_enc_rnd_mix(s);
    aes_enc_exp_step(rkp, rc[i - 1]);
    aes_enc_rnd_key(s, rkp);
  }
  aes_enc_rnd_sub(s);
  aes_enc_rnd_row(s);
  aes_enc_exp_step(rkp, rc[9]);
  aes_enc_rnd_key(s, rkp);

  memcpy(c, s, 16);
}
