/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "benchmark.h"

int main( int argc, char* argv[] ) {
  // initialise the development board, using the default configuration
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }

  uint32_t A[ 1024 ], r;

  while( true ) {
    // delay for 1000 ms = 1 s
    scale_delay_ms( 1000 );

    // write the trigger pin, and hence LED    : TRG <- 1 (positive edge)
    scale_gpio_wr( SCALE_GPIO_PIN_TRG, true  );

    // execute kernel #1: 1000 * nop (or "no operation")
    for( int i = 0; i < 1000; i++ ) {
      __asm__ __volatile__( "nop"                                             );
    }
    // execute kernel #2: 1000 * ldr (or "indexed load from memory into register")
    for( int i = 0; i < 1000; i++ ) {
      __asm__ __volatile__( "ldr %0, [%1,%2]" : "=&l" (r) : "l" (&A), "l" (i) );
    }

    // execute kernel #3: 1000 * mul (or "integer multiplication")
    for( int i = 0; i < 1000; i++ ) {
      __asm__ __volatile__( "mul %0, %1, %2"  : "=&l" (r) : "l"  (i), "l" (i) );
    }

    // write the trigger pin, and hence LED    : TRG <- 0 (negative edge)
    scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );
  }

  return 0;
}
