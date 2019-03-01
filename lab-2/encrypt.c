/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "encrypt.h"

#define AES_ENC_RND_MIX_STEP(a, b, c, d) { \
  aes_gf28_t __a1 = s[ a ];                \
  aes_gf28_t __b1 = s[ b ];                \
  aes_gf28_t __c1 = s[ c ];                \
  aes_gf28_t __d1 = s[ d ];                \
  aes_gf28_t __a2 = xtime( __a1 );         \
  aes_gf28_t __b2 = xtime( __b1 );         \
  aes_gf28_t __c2 = xtime( __c1 );         \
  aes_gf28_t __d2 = xtime( __d1 );         \
  aes_gf28_t __a3 = __a1 ^ __a2;           \
  aes_gf28_t __b3 = __b1 ^ __b2;           \
  aes_gf28_t __c3 = __c1 ^ __c2;           \
  aes_gf28_t __d3 = __d1 ^ __d2;           \
  s[ a ] = __a2 ^ __b3 ^ __c1 ^ __d1;      \
  s[ b ] = __a1 ^ __b2 ^ __c3 ^ __d1;      \
  s[ c ] = __a1 ^ __b1 ^ __c2 ^ __d3;      \
  s[ d ] = __a3 ^ __b1 ^ __c1 ^ __d2;      \
}

#define AES_ENC_RND_INIT() { \
  t_0 = rkp[0] ^ t_0;        \
  t_1 = rkp[1] ^ t_1;        \
  t_2 = rkp[1] ^ t_2;        \
  t_3 = rkp[1] ^ t_3;        \
  rkp += Nb;                 \
}

#define AES_ENC_RND_ITER() {                                    \
  t_4 = rkp[ 0 ] ^ ( AES_ENC_TBOX_0[ ( t_0 >>  0 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_1[ ( t_1 >>  8 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_2[ ( t_2 >> 16 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_3[ ( t_3 >> 24 ) & 0xFF ] ) ; \
  t_5 = rkp[ 1 ] ^ ( AES_ENC_TBOX_0[ ( t_1 >>  0 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_1[ ( t_2 >>  8 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_2[ ( t_3 >> 16 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_3[ ( t_0 >> 24 ) & 0xFF ] ) ; \
  t_6 = rkp[ 2 ] ^ ( AES_ENC_TBOX_0[ ( t_2 >>  0 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_1[ ( t_3 >>  8 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_2[ ( t_0 >> 16 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_3[ ( t_1 >> 24 ) & 0xFF ] ) ; \
  t_7 = rkp[ 3 ] ^ ( AES_ENC_TBOX_0[ ( t_3 >>  0 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_1[ ( t_0 >>  8 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_2[ ( t_1 >> 16 ) & 0xFF ] ) ^ \
                   ( AES_ENC_TBOX_3[ ( t_2 >> 24 ) & 0xFF ] ) ; \
  rkp += Nb; t_0 = t_4; t_1 = t_5; t_2 = t_6; t_3 = t_7;        \
}

#define AES_ENC_RND_FINI() {                                                 \
  t_4 = rkp[ 0 ] ^ ( AES_ENC_TBOX_4[ ( t_0 >>  0 ) & 0xFF ] & 0x000000FF ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_1 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_2 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_3 >> 24 ) & 0xFF ] & 0xFF000000 ) ; \
  t_5 = rkp[ 1 ] ^ ( AES_ENC_TBOX_4[ ( t_1 >>  0 ) & 0xFF ] & 0x000000FF ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_2 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_3 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_0 >> 24 ) & 0xFF ] & 0xFF000000 ) ; \
  t_6 = rkp[ 2 ] ^ ( AES_ENC_TBOX_4[ ( t_2 >>  0 ) & 0xFF ] & 0x000000FF ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_3 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_0 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_1 >> 24 ) & 0xFF ] & 0xFF000000 ) ; \
  t_7 = rkp[ 3 ] ^ ( AES_ENC_TBOX_4[ ( t_3 >>  0 ) & 0xFF ] & 0x000000FF ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_0 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_1 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^ \
                   ( AES_ENC_TBOX_4[ ( t_2 >> 24 ) & 0xFF ] & 0xFF000000 ) ; \
  rkp += Nb; t_0 = t_4; t_1 = t_5; t_2 = t_6; t_3 = t_7;                     \
}

aes_gf28_t rc[16] = {0x01, 0x02, 0x04, 0x08, 0x10,
                     0x20, 0x40, 0x80, 0x1B, 0x36};

uint8_t Nb = 4, Nr = 10;

int main(int argc, char* argv[]) {

  uint8_t k[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                   0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};

  uint8_t m[16] = {0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D,
                   0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34};

  uint8_t c[16] = {0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
                   0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32};

  uint8_t t[16];

  aes_enc(t, m, k);

  if (!memcmp(t, c, 16 * sizeof(uint8_t))) {
    printf("AES.Enc( k, m ) == c\n");
  } else {
    printf("AES.Enc( k, m ) != c\n");
  }

}

void print_values(aes_gf28_t* array, int size) {
  for (int i = 0; i < size; i++) {
    printf("%02X ", array[i]);
  }
  printf("\n");
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
  rk[0]  = rc ^ sbox(rk[13]) ^ rk[0];
  rk[1]  =      sbox(rk[14]) ^ rk[1];
  rk[2]  =      sbox(rk[15]) ^ rk[2];
  rk[3]  =      sbox(rk[12]) ^ rk[3];

  rk[4]  = rk[0]  ^ rk[4];
  rk[5]  = rk[1]  ^ rk[5];
  rk[6]  = rk[2]  ^ rk[6];
  rk[7]  = rk[3]  ^ rk[7];

  rk[8]  = rk[4]  ^ rk[8];
  rk[9]  = rk[5]  ^ rk[9];
  rk[10] = rk[6]  ^ rk[10];
  rk[11] = rk[7]  ^ rk[11];

  rk[12] = rk[8]  ^ rk[12];
  rk[13] = rk[9]  ^ rk[13];
  rk[14] = rk[10] ^ rk[14];
  rk[15] = rk[11] ^ rk[15];
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
  for (int i = 0; i < 4; i++, s += 4 ) {
    AES_ENC_RND_MIX_STEP(0, 1, 2, 3);
  }
}

void aes_enc_keyexp(uint8_t* r, const uint8_t* k) {
  aes_gf28_col_t* rp = (aes_gf28_col_t*) (r);

  memcpy(rp[0], k     , 4);
  memcpy(rp[1], k + 4 , 4);
  memcpy(rp[2], k + 8 , 4);
  memcpy(rp[3], k + 12, 4);

  for (int i = Nk; i < (Nb * (Nr + 1)); i++) {
    aes_gf28_col_t t_0 = rp[i - 1];
    aes_gf28_col_t t_1 = rp[i - Nb];

    if ((i % Nk) == 0) {
      t_0 = AES_RC[i / Nk] ^ (AES_ENC_TBOX_4[ (t_0 >>  8) & 0xFF ] & 0x000000FF) ^
                             (AES_ENC_TBOX_4[ (t_0 >> 16) & 0xFF ] & 0x0000FF00) ^
                             (AES_ENC_TBOX_4[ (t_0 >> 24) & 0xFF ] & 0x00FF0000) ^
                             (AES_ENC_TBOX_4[ (t_0 >>  0) & 0xFF ] & 0xFF000000) ;
    }

    rp[i] = t_0 ^ t_1;
  }
}

void aes_enc(uint8_t* c, uint8_t* m, uint8_t* k) {
  aes_gf28_t rk[4 * Nb], s[4 * Nb];

  memcpy(s , m, 16);
  memcpy(rk, k, 16);

  // Initial round
  aes_enc_rnd_key(s, rk);
  // (Nr - 1) iterated rounds
  for (int i = 1; i < Nr; i++) {
    aes_enc_rnd_sub(s);
    aes_enc_rnd_row(s);
    aes_enc_rnd_mix(s);
    aes_enc_exp_step(rk, rc[i - 1]);
    aes_enc_rnd_key(s, rk);
  }
  // Final round
  aes_enc_rnd_sub(s);
  aes_enc_rnd_row(s);
  aes_enc_exp_step(rk, rc[9]);
  aes_enc_rnd_key(s, rk);

  memcpy(c, s, 16);
}

void aes_enc_table(uint8_t* r, const uint8_t* m, const uint8_t* k) {
  aes_gf28_col_t* rkp = (aes_gf28_col_t*) (k);

  aes_enc_keyexp(rkp, k);

  aes_gf28_col_t t_0, t_1, t_2, t_3, t_4, t_5, t_6, t_7;

  memcpy(t_0, m     , 4);
  memcpy(t_1, m + 4 , 4);
  memcpy(t_2, m + 8 , 4);
  memcpy(t_3, m + 12, 4);

  // Initial   round
  AES_ENC_RND_INIT();
  // (Nr - 1) iterated rounds
  for (int i = 1; i < Nr; i++) {
    AES_ENC_RND_ITER();
  }
  // Final round
  AES_ENC_RND_FINI();

  memcpy(r     , t_0, 4);
  memcpy(r + 4 , t_1, 4);
  memcpy(r + 8 , t_2, 4);
  memcpy(r + 12, t_3, 4);
}
