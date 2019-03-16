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

int octetstr_rd(uint8_t* r, int n_r) {
  int i = 0;
  while (true) {
    if (i >= n_r) break;
    uint8_t t = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    if (t == '\x0D') {
      break;
    } else {
      r[i] = t;
      i++;
    }
  }
  return i;
}

void octetstr_wr(const uint8_t* x, int n_x) {
  uint8_t n1 = hex_to_char(n_x / 0x10);
  uint8_t n2 = hex_to_char(n_x % 0x10);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, n1);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, n2);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, ':');
  for (int i = 0; i < n_x; i++) {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i]);
  }
}

int main(int argc, char* argv[]) {
  // initialise the development board, using the default configuration
  if (!scale_init(&SCALE_CONF)) {
    return -1;
  }

  // char x[] = "hello world";
  // int n = 10;
  uint8_t x[10] = {0x65,0x66,0x67,0x68,0x69,0x70,0x71,0x72,0x73,0x74};

  while (true) {
    // read  the GPI     pin, and hence switch : t   <- GPI
    bool t = scale_gpio_rd( SCALE_GPIO_PIN_GPI        );
    // write the GPO     pin, and hence LED    : GPO <- t
             scale_gpio_wr( SCALE_GPIO_PIN_GPO, t     );

    // write the trigger pin, and hence LED    : TRG <- 1 (positive edge)
             scale_gpio_wr( SCALE_GPIO_PIN_TRG, true  );
    // delay for 500 ms = 1/2 s
    scale_delay_ms( 500 );
    // write the trigger pin, and hence LED    : TRG <- 0 (negative edge)
             scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );
    // delay for 500 ms = 1/2 s
    scale_delay_ms( 500 );

    // int n = strlen( x );

    // uint8_t x[10];
    // octetstr_rd(x, 10);
    octetstr_wr(x, 10);
  }

  return 0;
}
