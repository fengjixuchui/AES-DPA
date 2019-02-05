/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "acquire.h"

uint16_t    scope;
PICO_STATUS scope_status; 

 int16_t    scope_adc_min;
 int16_t    scope_adc_max;
 float_t    scope_range_chan_a;
 float_t    scope_range_chan_b;

float_t scope_adc2volts( float_t range, int16_t x ) {
  return ( ( float_t )( x ) / ( float_t )( scope_adc_max ) ) * range;
}

int16_t scope_volts2adc( float_t range, float_t x ) {
  return ( ( float_t )( x ) * ( float_t )( scope_adc_max ) ) / range;
}

int main( int argc, char* argv[] ) {

  /* Phase 1 follows Section 2.7.1.1 of the 2206B programming guide, producing
   * a 1-shot block mode acquisition process: it configures the 2206B to
   * 
   * - wait for a trigger signal (a positive edge exceeding 2 V) on channel A,
   * - sample from both channel A and B, using appropriate voltage ranges and
   *   for an appropriate amount of time (i.e., ~2 ms),
   * - store the resulting data in buffers with no post-processing (e.g., with
   *   no downsampling).
   */

  uint32_t timebase = 1; float_t timebase_delta = 4.0e-9, timebase_duration = 2.0e-3; int32_t samples = timebase_duration / timebase_delta, samples_max;

    // Section 3.32, Page 60; Step  1: open  the oscilloscope
    TRY_SCOPE( ps2000aOpenUnit, &scope, NULL );
  
    // Section 3.28, Page 56
    TRY_SCOPE( ps2000aMinimumValue, scope, &scope_adc_min );
    // Section 3.30, Page 58
    TRY_SCOPE( ps2000aMaximumValue, scope, &scope_adc_max );
  
    // Section 3.39, Page 69; Step  2: configure channels
    TRY_SCOPE( ps2000aSetChannel, scope, PS2000A_CHANNEL_A, true, PS2000A_DC, PS2000A_5V,    0 );
    scope_range_chan_a =   5.0e-0;
    TRY_SCOPE( ps2000aSetChannel, scope, PS2000A_CHANNEL_B, true, PS2000A_DC, PS2000A_500MV, 0 );
    scope_range_chan_b = 500.0e-3;
    
    // Section 3.13, Page 36; Step  3: configure timebase
    TRY_SCOPE( ps2000aGetTimebase2, scope, timebase, samples, NULL, 0, &samples_max, 0 ); 

    // Section 3.56, Page 93; Step  4: configure trigger
    TRY_SCOPE( ps2000aSetSimpleTrigger, scope, true, PS2000A_CHANNEL_A, scope_volts2adc( scope_range_chan_a, 2.0e-0 ), PS2000A_RISING, 0, 0 );
  
    // Section 3.37, Page 65; Step  5: start acquisition
    TRY_SCOPE( ps2000aRunBlock, scope, 0, samples, timebase, 0, NULL, 0, NULL, NULL );

  uint16_t ready = false, overflow;

  do {
    // Section 3.26, Page 54; Step  6: wait for acquisition to complete
    TRY_SCOPE( ps2000aIsReady, scope, &ready );
  } while( !ready );

  int16_t* A = malloc( samples * sizeof( int16_t ) );
  int16_t* B = malloc( samples * sizeof( int16_t ) );

    // Section 3.40, Page 71; Step  7: configure buffers
    TRY_SCOPE( ps2000aSetDataBuffer, scope, PS2000A_CHANNEL_A, A, samples, 0, PS2000A_RATIO_MODE_NONE );
    TRY_SCOPE( ps2000aSetDataBuffer, scope, PS2000A_CHANNEL_B, B, samples, 0, PS2000A_RATIO_MODE_NONE );
  
    // Section 3.18, Page 43; Step  8; transfer  buffers
    TRY_SCOPE( ps2000aGetValues, scope, 0, &samples, 1, PS2000A_RATIO_MODE_NONE, 0, &overflow );

    // Section 3.2,  Page 25; Step 10: stop  acquisition
    TRY_SCOPE( ps2000aStop, scope );
  
    // Section 3.2,  Page 25; Step 13: close the oscilloscope
    TRY_SCOPE( ps2000aCloseUnit, scope );

  /* Phase 2 simply stores the acquired data (both channels A *and* B) into a
   * CSV-formated file named on the command line.
   */
  
  FILE* fd = fopen( argv[ 1 ], "w" );

  for( int i = 0; i < samples; i++ ) {
    fprintf( fd, "%d, %.4f, %.4f\n", i, scope_adc2volts( scope_range_chan_a, A[ i ] ),
                                        scope_adc2volts( scope_range_chan_b, B[ i ] ) );
  }

  fclose( fd );

  free( A );
  free( B );

  return 0;
}
