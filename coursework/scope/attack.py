import binascii, select, serial, socket, sys, picoscope.ps2000a as ps2000a, time, numpy

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
    # if   ( args.mode == 'uart'   ) :
    fd = serial.Serial( port = "/dev/scale-board", baudrate = 9600, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = None )
    # elif ( args.mode == 'socket' ) :
    #     fd = socket.socket( socket.AF_INET, socket.SOCK_STREAM ) ; fd.connect( ( args.host, args.port ) ) ; fd = fd.makefile( mode = 'rwb', bufsize = 1024 )

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

def attack() :
    print("Attacking...")
    # Section 3.32, Page 60; Step  1: open  the oscilloscope
    scope = ps2000a.PS2000a()

    # Section 3.28, Page 56
    scope_adc_min = scope.getMinValue()
    # Section 3.30, Page 58
    scope_adc_max = scope.getMaxValue()

    fd = board_open()

    t = 200

    T = numpy.zeros((t, 10000))
    M = numpy.zeros((t, 16))
    C = numpy.zeros((t, 16))

    for i in range(t):
        print(i)

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

        print("test1")

        m = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08,
             0x09, 0x0A, 0x0B, 0X0C, 0X0D, 0X0E, 0X0F]

        print("test1.1")

        board_wrln( fd, "01:01" )
        print("test1.2")
        board_wrln( fd, str2octetstr( seq2str( m ) ) )
        print("test1.3")
        board_wrln( fd, "00:" )
        print("test1.4")

        # Section 3.26, Page 54; Step  6: wait for acquisition to complete
        while ( not scope.isReady() ) : time.sleep( 1 )

        print("test2")

        c = str2seq( octetstr2str( board_rdln( fd ) ) )

        print("test3")

        for n in range(16):
            M[i,n] = m[n]
            C[i,n] = c[n]

        print("test4")

        # Section 3.40, Page 71; Step  7: configure buffers
        # Section 3.18, Page 43; Step  8; transfer  buffers
        ( A, _, _ ) = scope.getDataRaw( channel = 'A', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )
        ( B, _, _ ) = scope.getDataRaw( channel = 'B', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )

        print("test5")

        # Section 3.2,  Page 25; Step 10: stop  acquisition
        scope.stop()

        print("test6")

        for j in range(samples):
            print(j)
            T[i,j] = scope_adc2volts( scope_range_chan_b, B[ i ] )

    print("test7")

    board_close( fd )

    # Section 3.2,  Page 25; Step 13: close the oscilloscope
    scope.close()

    return t, samples, M, C, T

attack()
