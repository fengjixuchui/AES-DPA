# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

import picoscope.ps2000a as ps2000a, sys, time

PS2000A_RATIO_MODE_NONE      = 0 # Section 3.18.1
PS2000A_RATIO_MODE_AGGREGATE = 1 # Section 3.18.1
PS2000A_RATIO_MODE_DECIMATE  = 2 # Section 3.18.1
PS2000A_RATIO_MODE_AVERAGE   = 4 # Section 3.18.1

def scope_adc2volts( range, x ) :
  return ( float( x ) / float( scope_adc_max ) ) * range;

def scope_volts2adc( range, x ) :
  return ( float( x ) * float( scope_adc_max ) ) / range;

if ( __name__ == '__main__' ) :
  # Phase 1 follows Section 2.7.1.1 of the 2206B programming guide, producing
  # a 1-shot block mode acquisition process: it configures the 2206B to
  # 
  # - wait for a trigger signal (a positive edge exceeding 2 V) on channel A,
  # - sample from both channel A and B, using appropriate voltage ranges and
  #   for an appropriate amount of time (i.e., ~2 ms),
  # - store the resulting data in buffers with no post-processing (e.g., with
  #   no downsampling).

  try :
    # Section 3.32, Page 60; Step  1: open  the oscilloscope
    scope = ps2000a.PS2000a()

    # Section 3.28, Page 56
    scope_adc_min = scope.getMinValue()
    # Section 3.30, Page 58  
    scope_adc_max = scope.getMaxValue()

    # Section 3.39, Page 69; Step  2: configure channels
    scope.setChannel( channel = 'A', enabled = True, coupling = 'DC', VRange =   5.0E-0 )
    scope_range_chan_a =   5.0e-0
    scope.setChannel( channel = 'B', enabled = True, coupling = 'DC', VRange = 500.0E-3 )
    scope_range_chan_b = 500.0e-3

    # Section 3.13, Page 36; Step  3: configure timebase
    ( _, samples, samples_max ) = scope.setSamplingInterval( 4.0E-9, 2.0E-3 )
  
    # Section 3.56, Page 93; Step  4: configure trigger
    scope.setSimpleTrigger( 'A', threshold_V = 2.0E-0, direction = 'Rising', timeout_ms = 0 ) 
  
    # Section 3.37, Page 65; Step  5: start acquisition
    scope.runBlock()

    # Section 3.26, Page 54; Step  6: wait for acquisition to complete  
    while ( not scope.isReady() ) : time.sleep( 1 )
  
    # Section 3.40, Page 71; Step  7: configure buffers
    # Section 3.18, Page 43; Step  8; transfer  buffers
    ( A, _, _ ) = scope.getDataRaw( channel = 'A', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )
    ( B, _, _ ) = scope.getDataRaw( channel = 'B', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )

    # Section 3.2,  Page 25; Step 10: stop  acquisition
    scope.stop()

    # Section 3.2,  Page 25; Step 13: close the oscilloscope
    scope.close()

  except Exception as e :
    raise e

  # Phase 2 simply stores the acquired data (both channels A *and* B) into a
  # CSV-formated file named on the command line.
  
  try :
    fd = open( sys.argv[ 1 ], 'w' )

    for i in range( samples ) :
      print >> fd, '%d, %.4f, %.4f' % ( i, scope_adc2volts( scope_range_chan_a, A[ i ] ),
                                           scope_adc2volts( scope_range_chan_b, B[ i ] ) )
    fd.close()

  except Exception as e :
    raise e
