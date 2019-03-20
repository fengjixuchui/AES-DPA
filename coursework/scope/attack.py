# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

import numpy, struct, sys

## Load  a trace data set from an on-disk file.
## 
## \param[in] f the filename to load  trace data set from
## \return    t the number of traces
## \return    s the number of samples in each trace
## \return    M a t-by-16 matrix of AES-128  plaintexts
## \return    C a t-by-16 matrix of AES-128 ciphertexts
## \return    T a t-by-s  matrix of samples, i.e., the traces

def traces_ld( f ) :
  fd = open( f, "rb" )

  def rd( x ) :
    ( r, ) = struct.unpack( x, fd.read( struct.calcsize( x ) ) ) ; return r

  t = rd( '<I' )
  s = rd( '<I' )

  M = numpy.zeros( ( t, 16 ), dtype = numpy.uint8 )
  C = numpy.zeros( ( t, 16 ), dtype = numpy.uint8 )
  T = numpy.zeros( ( t,  s ), dtype = numpy.int16 )

  for i in range( t ) :
    for j in range( 16 ) :
      M[ i, j ] = rd( '<B' )

  for i in range( t ) :
    for j in range( 16 ) :
      C[ i, j ] = rd( '<B' )

  for i in range( t ) :
    for j in range( s  ) :
      T[ i, j ] = rd( '<h' )

  fd.close()

  return t, s, M, C, T

## Store a trace data set into an on-disk file.
## 
## \param[in] f the filename to store trace data set into
## \param[in] t the number of traces
## \param[in] s the number of samples in each trace
## \param[in] M a t-by-16 matrix of AES-128  plaintexts
## \param[in] C a t-by-16 matrix of AES-128 ciphertexts
## \param[in] T a t-by-s  matrix of samples, i.e., the traces

def traces_st( f, t, s, M, C, T ) :
  fd = open( f, "wb" )

  def wr( x, y ) :
    fd.write( struct.pack( x, y ) )

  wr( '<I', t,  )
  wr( '<I', s,  )

  for i in range( t ) :
    for j in range( 16 ) :
      wr( '<B', M[ i, j ] )

  for i in range( t ) :
    for j in range( 16 ) :
      wr( '<B', C[ i, j ] )

  for i in range( t ) :
    for j in range( s  ) :
      wr( '<h', T[ i, j ] )

  fd.close()

## Attack implementation, as invoked from main after checking command line
## arguments.
##
## \param[in] argc number of command line arguments
## \param[in] argv           command line arguments

def attack( argc, argv )
  pass

if ( __name__ == '__main__' ) :
  attack( len( sys.argv ), sys.argv )
