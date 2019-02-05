/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __ACQUIRE_H
#define __ACQUIRE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <math.h>

#include <ps2000aApi.h>

#define TRY_SCOPE( f, ... ) {                                         \
  if( PICO_OK != ( scope_status = f( __VA_ARGS__ ) ) ) {              \
    printf( "%s failed (status=%08X)\n", #f, scope_status ); abort(); \
  }                                                                   \
}

#endif
