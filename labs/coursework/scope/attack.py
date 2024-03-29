import argparse
import binascii
import random
import select
import socket
import struct
import sys
import time
import timeit

import Crypto.Cipher.AES as AES
import numpy

import picoscope.ps2000a as ps2000a
import serial

numpy.seterr(divide='ignore', invalid='ignore')

PS2000A_RATIO_MODE_NONE = 0 # Section 3.18.1
DURATION                = 0.2E-3
INTERVAL                = 4.0E-9

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

sbox = [0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
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
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16]

hamming_weights = [0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8]

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

## Open  (or start)  communication with SCALE development board.
##
## \return    fd a communication end-point

def board_open() :
    if   ( args.mode == 'uart'   ) :
        fd = serial.Serial( port = args.uart, baudrate = 9600, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = None )
    elif ( args.mode == 'socket' ) :
        fd = socket.socket( socket.AF_INET, socket.SOCK_STREAM ) ; fd.connect( ( args.host, args.port ) ) ; fd = fd.makefile( mode = 'rwb' )

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

def client() :

    fd = board_open()

    # Record 150 traces
    t = 150

    # Number of samples is the duration divided by the interval between samples.
    s = DURATION / INTERVAL

    T = numpy.zeros((t,  s))
    M = numpy.zeros((t, 16), dtype = numpy.uint8)
    C = numpy.zeros((t, 16), dtype = numpy.uint8)

    # Section 3.39, Page 69; Step  2: configure channels
    scope.setChannel( channel = 'A', enabled = True, coupling = 'DC', VRange =   5.0E-0 )
    scope.setChannel( channel = 'B', enabled = True, coupling = 'DC', VRange = 500.0E-3 )

    # Section 3.13, Page 36; Step  3: configure timebase
    ( _, samples, _ ) = scope.setSamplingInterval( INTERVAL, DURATION )

    # Wait some time to prevent the board from hanging when reading.
    time.sleep( 0.2 )

    print("Acquisition started...")

    for i in range(t):

        original_m = m = []
        r = []

        # Call the INSPECT command to get the number of random bytes to supply.
        board_wrln( fd, "01:00" )
        board_rdln( fd )
        board_rdln( fd )
        random_size = board_rdln( fd )

        random_size = str2seq( octetstr2str( random_size ) )

        # Fill m and r with random numbers.
        for _ in range(16) :
            m.append(random.randint(0,255))

        for _ in range(random_size[0]) :
            r.append(random.randint(0,255))

        m = str2octetstr( seq2str( m ) )
        r = str2octetstr( seq2str( r ) )

        # Section 3.56, Page 93; Step  4: configure trigger
        scope.setSimpleTrigger( 'A', threshold_V = 2.0E-0, direction = 'Rising', timeout_ms = 0 )

        # Section 3.37, Page 65; Step  5: start acquisition
        scope.runBlock()

        board_wrln( fd, "01:01" )
        board_wrln( fd,  m      )
        board_wrln( fd,  r      )

        # Section 3.26, Page 54; Step  6: wait for acquisition to complete
        while ( not scope.isReady() ) : time.sleep( 0.1 )

        # Read back the ciphertext from the board.
        c = board_rdln( fd )
        c = str2seq( octetstr2str( c ) )

        M[i] = original_m
        C[i] = c

        # Section 3.40, Page 71; Step  7: configure buffers
        # Section 3.18, Page 43; Step  8; transfer  buffers
        ( B, _, _ ) = scope.getDataRaw( channel = 'B', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )

        T[i] = B

        # Section 3.2,  Page 25; Step 10: stop  acquisition
        scope.stop()

    board_close( fd )

    scope.close()

    print("Acquisition complete.")

    return t, samples, M, C, T

# Found online at https://stackoverflow.com/questions/30143417/computing-the-correlation-coefficient-between-two-multi-dimensional-arrays
# Calculates the correlation coefficient between two 2D arrays and returns a 2D array.
def correlation_coefficient(A,B):
    # Rowwise mean of input arrays & subtract from input arrays themeselves
    A_mA = A - A.mean(1)[:,None]
    B_mB = B - B.mean(1)[:,None]

    # Sum of squares across rows
    ssA = (A_mA**2).sum(1)
    ssB = (B_mB**2).sum(1)

    # Finally get corr coeff
    return numpy.dot(A_mA,B_mB.T)/numpy.sqrt(numpy.dot(ssA[:,None],ssB[None]))

def attack( argv ) :
    t, _, M, C, T = client()

    H = numpy.zeros((t, 256), dtype = numpy.uint8) # Hypothetical power consumption values

    key = []

    T = T[:, 0:10000]

    for k in range(16):
        print("Attacking byte {0}...".format(k))
        for i in range(t):
            for j in range(256):
                H[i,j] = hamming_weights[ sbox[M[i,k] ^ j] ]

        R = numpy.abs(correlation_coefficient(H.T, T.T))
        # Find the index of the key hypothesis with the highest correlation.
        key.append(numpy.argmax(numpy.nanmax(R, axis=1)))

    k = key
    m = M[0,:]
    c = C[0,:]

    k = struct.pack(16 * 'B', *k)
    m = struct.pack(16 * 'B', *m)
    c = struct.pack(16 * 'B', *c)

    test = AES.new(k).encrypt(m)

    if (test == c):
        print("Key successfully recovered.")
    else:
        print("Key recovery unsuccessful.")
    print("{0} traces used.".format(t))
    print("Key recovered: {0}".format(str2octetstr( seq2str( key ) )))

if ( __name__ == '__main__' ) :
    # parse command line arguments

    parser = argparse.ArgumentParser()

    parser.add_argument( '--mode', dest = 'mode',             action = 'store', choices = [ 'uart', 'socket' ], default = 'uart'             )

    parser.add_argument( '--uart', dest = 'uart', type = str, action = 'store',                                 default = '/dev/scale-board' )

    args = parser.parse_args()

    # execute client implementation

    # Section 3.32, Page 60; Step  1: open  the oscilloscope
    scope = ps2000a.PS2000a()

    # Section 3.28, Page 56
    scope_adc_min = scope.getMinValue()
    # Section 3.30, Page 58
    scope_adc_max = scope.getMaxValue()

    # Run the attack
    tic = timeit.default_timer()
    attack( sys.argv )
    toc = timeit.default_timer()
    print("Attack took {:.2f} seconds in total.".format(toc-tic))
