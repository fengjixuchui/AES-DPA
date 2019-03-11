/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "target.h"

bool match( int* t, const char* x, const char* y ) {
  int l_x = strlen( x );
  int l_y = strlen( y );

  *t = 0;

  if( l_x != l_y ) {
    return false;
  }

  *t = 1;

  for( int i = 0; i < l_x; i++, *t = *t + 1 ) {
    if( x[ i ] != y[ i ] ) {
      return false;
    }
  }

  return true;
}

int main( int argc, char* argv[] ) {
  char G[ 8 + 1 ], P[] = "password";

  while( true ) {
    if( 1 != fscanf( stdin, "%8s", G ) ) {
      abort();
    }

    if( feof( stdin ) ) {
      break;
    }

    int t, r = match( &t, P, G );

    fprintf( stdout, "%d\n", t );
    fprintf( stdout, "%d\n", r );

    fflush( stdout );
    fflush( stderr );
  }

  return 0;
}
