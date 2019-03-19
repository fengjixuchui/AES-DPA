/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "helloworld.h"

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

int octetstr_rd(uint8_t* r, int n_r) {
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

void octetstr_wr(const uint8_t* x, int n_x) {
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

void print(char* string) {
  int length = strlen(string);
  for (int i = 0; i < length; i++) {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, string[i]);
  }
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D'); // CR
}

void reverse_array(uint8_t* a, int length) {
  uint8_t temp;
  for (int i = 0; i < length/2; i++) {
    temp = a[i];
    a[i] = a[length-1-i];
    a[length-1-i] = temp;
  }
}

int main(int argc, char* argv[]) {
  // initialise the development board, using the default configuration
  if (!scale_init(&SCALE_CONF)) {
    return -1;
  }

  uint8_t r[128];

  while (true) {
    // write the trigger pin, and hence LED    : TRG <- 1 (positive edge)
    scale_gpio_wr( SCALE_GPIO_PIN_TRG, true  );

    int length = octetstr_rd(r, 128);
    reverse_array(r, length);
    octetstr_wr(r, length);

    scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );
  }

  return 0;
}
