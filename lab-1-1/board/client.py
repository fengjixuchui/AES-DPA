# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

import argparse, binascii, select, serial, socket, sys

## Convert a string (e.g., string, or bytearray) into a list (or sequence).
## 
## \param[in] x  a  string
## \return       a  list   r st. r[ i ] = ord( x[ i ] )

def str2seq( x ) :
  return          [ ord( t ) for t in x ]

## Convert a list (or sequence) into a string (e.g., string, or bytearray).
## 
## \param[in] x  a  list
## \return       a  string r st. r[ i ] = chr( x[ i ] )

def seq2str( x ) :
  return ''.join( [ chr( t ) for t in x ] )

## Convert a length-prefixed, hexadecimal octet string into a string.
## 
## \param[in] x  an octet string
## \return       a  string
## \throw        ValueError if the length prefix and data do not match

def octetstr2str( x ) :
  t = x.split( ':' ) ; n = int( t[ 0 ], 16 ) ; x = binascii.a2b_hex( t[ 1 ] )

  if( n != len( x ) ) :
    raise ValueError
  else :
    return x

## Convert a string into a length-prefixed, hexadecimal octet string.
## 
## \param[in] x  an octet string
## \return       a  string

def str2octetstr( x ) :
  return ( '%02X' % ( len( x ) ) ) + ':' + ( binascii.b2a_hex( x ) )

## Open  (or start)  communication with SCALE development board.
##
## \return    fd a communication end-point

def board_open() :
  if   ( args.mode == 'uart'   ) :
    fd = serial.Serial( port = args.uart, baudrate = 9600, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = None )
  elif ( args.mode == 'socket' ) :
    fd = socket.socket( socket.AF_INET, socket.SOCK_STREAM ) ; fd.connect( ( args.host, args.port ) ) ; fd = fd.makefile( mode = 'rwb', bufsize = 1024 )

  return fd

## Close (or finish) communication with SCALE development board.
##
## \param[in] fd a communication end-point

def board_close( fd ) :
  fd.close()

## Read  (or recieve) a string from SCALE development board, automatically 
## managing CR-only EOL semantics.
##
## \param[in] fd a communication end-point
## \return    r  a string (e.g., string, or bytearray)

def board_rdln( fd    ) :
  r = ''

  while( True ):
    t = fd.read( 1 )

    if( t == '\x0D' ) :
      break
    else:
      r += t

  return r

## Write (or send)    a string to   SCALE development board, automatically 
## managing CR-only EOL semantics.
##
## \param[in] fd a communication end-point
## \param[in] x  a string (e.g., string, or bytearray)

def board_wrln( fd, x ) :
  fd.write( x + '\x0D' ) ; fd.flush()

## Client implementation: the idea is to send an octet string x, and verify 
## the octet string received in response matches what we expect (i.e., that 
## we get an r = f_i( x )).

def client() :
  fd = board_open() ; x = args.data

  t_0 =                                  x
  t_1 =                    octetstr2str( x )
  t_2 = ''.join( reversed( octetstr2str( x ) ) )

  print 't_0 =                    x     = %s' % ( repr( t_0 ) )
  print 't_1 =      octetstr2str( x )   = %s' % ( repr( t_1 ) )
  print '      len( octetstr2str( x ) ) = %d' % (  len( t_1 ) )
  print 't_2 = rev( octetstr2str( x ) ) = %s' % ( repr( t_2 ) )

  board_wrln( fd, x ) ; r = board_rdln( fd )

  t_3 =                                  r
  t_4 =                    octetstr2str( r )

  print 't_3 =                    r     = %s' % ( repr( t_3 ) )
  print 't_4 =      octetstr2str( r )   = %s' % ( repr( t_4 ) )
  print '      len( octetstr2str( r ) ) = %d' % (  len( t_4 ) )

  if( t_4 == t_2 ) :
    print 't_2 = rev( octetstr2str( x ) ) == octetstr2str( r ) = t_4 => success'
  else :
    print 't_2 = rev( octetstr2str( x ) ) != octetstr2str( r ) = t_4 => failure'

  board_close( fd )

if ( __name__ == '__main__' ) :
  # parse command line arguments

  parser = argparse.ArgumentParser()

  parser.add_argument( '--mode', dest = 'mode',             action = 'store', choices = [ 'uart', 'socket' ], default = 'uart'             )

  parser.add_argument( '--uart', dest = 'uart', type = str, action = 'store',                                 default = '/dev/scale-board' )
  parser.add_argument( '--host', dest = 'host', type = str, action = 'store' )
  parser.add_argument( '--port', dest = 'port', type = int, action = 'store' )

  parser.add_argument( '--data', dest = 'data', type = str, action = 'store' )

  args = parser.parse_args()

  # execute client implementation

  client()
