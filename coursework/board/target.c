/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "target.h"

uint8_t sbox[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
 };

uint8_t masked_sbox[256];
uint8_t mask, m_, m1, m2, m3, m4, m1_, m2_, m3_, m4_;

aes_gf28_t rc[16] = {0x01, 0x02, 0x04, 0x08, 0x10,
                     0x20, 0x40, 0x80, 0x1B, 0x36};

uint8_t Nb = 4, Nr = 10;

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

char hex_to_char(int n) {
  if (n > 0xF) return -1;
  else if (n < 0xA) return n + 0x30;
  else return n + 0x37;
}

uint8_t char_to_hex(char c) {
  if (c > 0x2F && c < 0x3A) return c - 0x30;
  else if (c > 0x40 && c < 0x47) return c - 0x37;
  else if (c > 0x60 && c < 0x67) return c - 0x57;
  else return -1;
}

/** Read  an octet string (or sequence of bytes) from the UART, using a simple
  * length-prefixed, little-endian hexadecimal format.
  *
  * \param[out] r the destination octet string read
  * \return       the number of octets read
  */

int  octetstr_rd(       uint8_t* r, int n_r ) {
  char c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  char c2 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
            scale_uart_rd(SCALE_UART_MODE_BLOCKING); // :
  uint8_t length = (char_to_hex(c1) * 0x10) + char_to_hex(c2);
  if (length > n_r) length = n_r;
  for (int i = 0; i < length; i++) {
    c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    c2 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    r[i] = (char_to_hex(c1) * 0x10) + char_to_hex(c2);
  }
  scale_uart_rd(SCALE_UART_MODE_BLOCKING); // CR
  return length;
}

/** Write an octet string (or sequence of bytes) to   the UART, using a simple
  * length-prefixed, little-endian hexadecimal format.
  *
  * \param[in]  r the source      octet string written
  * \param[in]  n the number of octets written
  */

void octetstr_wr( const uint8_t* x, int n_x ) {
  uint8_t n1 = hex_to_char(n_x / 0x10);
  uint8_t n2 = hex_to_char(n_x % 0x10);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, n1);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, n2);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, ':');
  for (int i = 0; i < n_x; i++) {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, hex_to_char(x[i] / 0x10));
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, hex_to_char(x[i] % 0x10));
  }
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D'); // CR
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

void aes_enc_rnd_key(aes_gf28_t* s, aes_gf28_t* rk) {
  for (int i = 0; i < 16; i++) {
    s[i] = s[i] ^ rk[i];
  }
}

void aes_enc_rnd_sub(aes_gf28_t* s) {
  for (int i = 0; i < 16; i++) {
    s[i] = masked_sbox[s[i]];
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

void aes_enc_exp_step(aes_gf28_t* rk, aes_gf28_t rc) {
  rk[0]  = rc ^ sbox[rk[13]] ^ rk[0];
  rk[1]  =      sbox[rk[14]] ^ rk[1];
  rk[2]  =      sbox[rk[15]] ^ rk[2];
  rk[3]  =      sbox[rk[12]] ^ rk[3];

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

void calculate_masked_sbox() {
  for (int i = 0; i < 256; i++) {
    masked_sbox[i ^ mask] = sbox[i] ^ m_;
  }
}

void calculate_mix_column_masks() {
  aes_gf28_t m1_square = xtime( m1 );
  aes_gf28_t m2_square = xtime( m2 );
  aes_gf28_t m3_square = xtime( m3 );
  aes_gf28_t m4_square = xtime( m4 );
  aes_gf28_t m1_cube = m1 ^ m1_square;
  aes_gf28_t m2_cube = m2 ^ m2_square;
  aes_gf28_t m3_cube = m3 ^ m3_square;
  aes_gf28_t m4_cube = m4 ^ m4_square;
  m1_ = m1_square ^ m2_cube   ^ m3        ^ m4       ;
  m2_ = m1        ^ m2_square ^ m3_cube   ^ m4       ;
  m3_ = m1        ^ m2        ^ m3_square ^ m4_cube  ;
  m4_ = m1_cube   ^ m2        ^ m3        ^ m4_square;
}

void mask_rk(uint8_t* rk, uint8_t* masked_rk) {
  for (int i = 0; i < 4; i++) {
    masked_rk[0 + i*4] = rk[0 + i*4] ^ m1_ ^ mask;
    masked_rk[1 + i*4] = rk[1 + i*4] ^ m2_ ^ mask;
    masked_rk[2 + i*4] = rk[2 + i*4] ^ m3_ ^ mask;
    masked_rk[3 + i*4] = rk[3 + i*4] ^ m4_ ^ mask;
  }
}

void mask_rk_final(uint8_t* rk, uint8_t* masked_rk) {
  for (int i = 0; i < 4; i++) {
    masked_rk[0 + i*4] = rk[0 + i*4] ^ m_;
    masked_rk[1 + i*4] = rk[1 + i*4] ^ m_;
    masked_rk[2 + i*4] = rk[2 + i*4] ^ m_;
    masked_rk[3 + i*4] = rk[3 + i*4] ^ m_;
  }
}

void mask_s(uint8_t* s) {
  for (int i = 0; i < 4; i++) {
    s[0 + i*4] ^= m1_;
    s[1 + i*4] ^= m2_;
    s[2 + i*4] ^= m3_;
    s[3 + i*4] ^= m4_;
  }
}

void remask(uint8_t* s) {
  for (int i = 0; i < 4; i++) {
    s[0 + i*4] ^= m_ ^ m1;
    s[1 + i*4] ^= m_ ^ m2;
    s[2 + i*4] ^= m_ ^ m3;
    s[3 + i*4] ^= m_ ^ m4;
  }
}

/** Initialise an AES-128 encryption, e.g., expand the cipher key k into round
  * keys, or perform randomised pre-computation in support of a countermeasure;
  * this can be left blank if no such initialisation is required, because the
  * same k and r will be passed as input to the encryption itself.
  *
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */

void aes_init(                               const uint8_t* k, const uint8_t* r ) {
  mask  = r[0];
  m_    = r[1];
  m1    = r[2];
  m2    = r[3];
  m3    = r[4];
  m4    = r[5];

  calculate_mix_column_masks();
  calculate_masked_sbox();
}

/** Perform    an AES-128 encryption of a plaintext m under a cipher key k, to
  * yield the corresponding ciphertext c.
  *
  * \param[out] c   an   AES-128 ciphertext
  * \param[in]  m   an   AES-128 plaintext
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */

void aes     ( uint8_t* c, const uint8_t* m, const uint8_t* k, const uint8_t* r ) {
  aes_gf28_t rk[4 * Nb], s[4 * Nb], masked_rk[4 * Nb];

  memcpy(s , m, 16);
  memcpy(rk,       k, 16);

  mask_s(s);
  mask_rk(rk, masked_rk);

  // Initial round
  aes_enc_rnd_key(s, masked_rk);
  // (Nr - 1) iterated rounds
  for (int i = 1; i < Nr; i++) {
    aes_enc_rnd_sub(s);
    aes_enc_rnd_row(s);
    remask(s);
    aes_enc_rnd_mix(s);
    aes_enc_exp_step(rk, rc[i - 1]);
    mask_rk(rk, masked_rk);
    aes_enc_rnd_key(s, masked_rk);
  }
  // Final round
  aes_enc_rnd_sub(s);
  aes_enc_rnd_row(s);
  aes_enc_exp_step(rk, rc[9]);
  mask_rk_final(rk, masked_rk);
  aes_enc_rnd_key(s, masked_rk);

  memcpy(c, s, 16);
}

void print(char* string) {
  int length = strlen(string);
  for (int i = 0; i < length; i++) {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, string[i]);
  }
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D'); // CR
}

/** Initialise the SCALE development board, then loop indefinitely, reading a
  * command then processing it:
  *
  * 1. If command is inspect, then
  *
  *    - write the SIZEOF_BLK parameter,
  *      i.e., number of bytes in an  AES-128 plaintext  m, or ciphertext c,
  *      to the UART,
  *    - write the SIZEOF_KEY parameter,
  *      i.e., number of bytes in an  AES-128 cipher key k,
  *      to the UART,
  *    - write the SIZEOF_RND parameter,
  *      i.e., number of bytes in the         randomness r.
  *      to the UART.
  *
  * 2. If command is encrypt, then
  *
  *    - read  an   AES-128 plaintext  m from the UART,
  *    - read  some         randomness r from the UART,
  *    - initalise the encryption,
  *    - set the trigger signal to 1,
  *    - execute   the encryption, producing the ciphertext
  *
  *      c = AES-128.Enc( m, k )
  *
  *      using the hard-coded cipher key k plus randomness r if/when need be,
  *    - set the trigger signal to 0,
  *    - write an   AES-128 ciphertext c to   the UART.
  */

int main( int argc, char* argv[] ) {
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }

  uint8_t cmd[ 1 ], c[ SIZEOF_BLK ], m[ SIZEOF_BLK ], k[ SIZEOF_KEY ] = { 0xD4, 0x96, 0xE8, 0x8F, 0x21, 0x40, 0x55, 0x92, 0xED, 0x18, 0x62, 0xA9, 0x8C, 0x68, 0x35, 0xE6 }, r[ SIZEOF_RND ];

  while( true ) {
    if( 1 != octetstr_rd( cmd, 1 ) ) {
      break;
    }

    switch( cmd[ 0 ] ) {
      case COMMAND_INSPECT : {
        uint8_t t = SIZEOF_BLK;
                    octetstr_wr( &t, 1 );
                t = SIZEOF_KEY;
                    octetstr_wr( &t, 1 );
                t = SIZEOF_RND;
                    octetstr_wr( &t, 1 );

        break;
      }
      case COMMAND_ENCRYPT : {
        if( SIZEOF_BLK != octetstr_rd( m, SIZEOF_BLK ) ) {
          break;
        }
        if( SIZEOF_RND != octetstr_rd( r, SIZEOF_RND ) ) {
          break;
        }

        aes_init(       k, r );

        scale_gpio_wr( SCALE_GPIO_PIN_TRG,  true );
        aes     ( c, m, k, r );
        scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );

                          octetstr_wr( c, SIZEOF_BLK );

        break;
      }
      default : {
        break;
      }
    }
  }

  return 0;
}
