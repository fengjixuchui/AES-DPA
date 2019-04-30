import argparse, binascii, select, serial, socket, sys, picoscope.ps2000a as ps2000a, time, numpy, random

PS2000A_RATIO_MODE_NONE      = 0 # Section 3.18.1

def scope_adc2volts( range, x ) :
    return ( float( x ) / float( scope_adc_max ) ) * range;

def scope_volts2adc( range, x ) :
    return ( float( x ) * float( scope_adc_max ) ) / range;

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
    fd = board_open() ;

    t = 10

    T = numpy.zeros((t, 10000))
    M = numpy.zeros((t, 16))
    C = numpy.zeros((t, 16))

    print("Acquisition started...")

    for i in range(t):

        original_m = m = []

        for b in range(16) :
            m.append(random.randint(0,255))

        m = seq2str( m )
        m = str2octetstr( m )

        board_wrln( fd, "01:01" )
        board_wrln( fd,  m      )
        board_wrln( fd, "00:"   )

        c = board_rdln( fd )

        print(c)
        c = octetstr2str( c )
        c = str2seq( c )

        for n in range(16):
            M[i,n] = original_m[n]
            C[i,n] = c[n]

    board_close( fd )

    print("Completed...")

if ( __name__ == '__main__' ) :
    # parse command line arguments

    parser = argparse.ArgumentParser()

    parser.add_argument( '--mode', dest = 'mode',             action = 'store', choices = [ 'uart', 'socket' ], default = 'uart'             )

    parser.add_argument( '--uart', dest = 'uart', type = str, action = 'store',                                 default = '/dev/scale-board' )

    args = parser.parse_args()

    # execute client implementation

    client()
