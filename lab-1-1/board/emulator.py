# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

import argparse, binascii, logging, os, select, signal, socket, struct, sys

import  unicorn           as  unicorn
import  unicorn.arm_const as  unicorn_arm
import capstone           as capstone
import capstone.arm       as capstone_arm

import intelhex           as intelhex

def mmio_syscon_rd( emu, mode, addr, size, val, data ):
  if   ( addr == 0x4004800C ) : # SYSPLLSTAT
      emu.mem_write( 0x4004800C, chr( 1 ) )
  elif ( addr == 0x40048044 ) : # SYSPLLCLKUEN
      emu.mem_write( 0x40048044, chr( 1 ) )
  elif ( addr == 0x40048074 ) : # MAINCLKUEN
      emu.mem_write( 0x40048074, chr( 1 ) )

def mmio_syscon_wr( emu, mode, addr, size, val, data ):
  pass

def mmio_uart_can_rd( x ) :
  ( t, _, _ ) = select.select( [ x ], [], [], 0.0 ) ; return x in t
def mmio_uart_can_wr( x ) :
  ( _, t, _ ) = select.select( [], [ x ], [], 0.0 ) ; return x in t

def mmio_uart_rd( emu, mode, addr, size, val, data ):
  if   ( addr == 0x40008000 ) : # U0RBR
    if ( mmio_uart_can_rd( client ) ) :
      emu.mem_write( 0x40008000, client.read( 1 ) )
    else :
      emu.mem_write( 0x40008000, ''.join( [ chr( x ) for x in [ 0 ] ] ) )
  elif ( addr == 0x40008014 ) : # U0LSR
    if ( mmio_uart_can_rd( client ) ) :
      emu.mem_write( 0x40008014, chr( 0x21 ) ) # => no error, can   read, can   write
    else :
      emu.mem_write( 0x40008014, chr( 0x20 ) ) # => no error, can't read, can   write

def mmio_uart_wr( emu, mode, addr, size, val, data ):
  if   ( addr == 0x40008000 ) : # U0THR
    if ( mmio_uart_can_wr( client ) ) :
      client.send( chr( val ) )
    else :
      pass
  elif ( addr == 0x40008014 ) : # U0LSR
    pass

def fetch( emu, addr, size, data ):
  pass

if ( __name__ == '__main__' ) :
  # parse command line arguments

  parser = argparse.ArgumentParser()

  parser.add_argument( '--target', dest = 'target',              action = 'store', choices = [ 'lpc1114fn28', 'lpc1313fbd48' ], default = 'lpc1313fbd48' )

  parser.add_argument( '--host',   dest = 'host',   type =  str, action = 'store' )
  parser.add_argument( '--port',   dest = 'port',   type =  int, action = 'store' )
  parser.add_argument( '--file',   dest = 'file',   type =  str, action = 'store' )

  parser.add_argument( '--debug',  dest = 'debug',               action = 'store_true' )

  args = parser.parse_args()

  if ( args.debug ) :
    l = logging.DEBUG
  else :
    l = logging.INFO

  logging.basicConfig( stream = sys.stdout, level = l, format = '%(filename)s : %(asctime)s : %(message)s', datefmt = '%d/%m/%y @ %H:%M:%S' )

  # create emulator and disassembler

  emu =  unicorn.Uc(  unicorn.UC_ARCH_ARM,  unicorn.UC_MODE_THUMB )
  asm = capstone.Cs( capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB )
  
  if   ( args.target == 'lpc1114fn28'  ) :
    emu.mem_map( 0x00000000,  32 * 1024        ) # Figure  6:  32 kB => flash 
    emu.mem_map( 0x10000000,   4 * 1024        ) # Figure  6:   4 kB => SRAM
    emu.mem_map( 0x1FFF0000,  16 * 1024        ) # Figure  6:  16 kB => boot ROM
    emu.mem_map( 0x40000000, 512 * 1024        ) # Figure  6: 512 kB => APB peripherals
    emu.mem_map( 0x50000000,   2 * 1024 * 1024 ) # Figure  6:   2 MB => AHB peripherals

  elif ( args.target == 'lpc1313fbd48' ) :
    emu.mem_map( 0x00000000,  32 * 1024        ) # Figure 14:  32 kB => flash 
    emu.mem_map( 0x10000000,   8 * 1024        ) # Figure 14:   8 kB => SRAM
    emu.mem_map( 0x1FFF0000,  16 * 1024        ) # Figure 14:  16 kB => boot ROM
    emu.mem_map( 0x40000000, 512 * 1024        ) # Figure 14: 512 kB => APB peripherals
    emu.mem_map( 0x50000000,   2 * 1024 * 1024 ) # Figure 14:   2 MB => AHB peripherals

  # hook instruction fetch, and handle ctrl-c as forced exit

  emu.hook_add( unicorn.UC_HOOK_CODE, fetch )

  def handler( s, f ) :
    exit( 0 )

  signal.signal( signal.SIGINT, handler )

  # hook access to memory mapped SYSCON

  emu.hook_add( unicorn.UC_HOOK_MEM_READ,  mmio_syscon_rd, begin = 0x40048000, end = 0x4004C000 - 1 )
  emu.hook_add( unicorn.UC_HOOK_MEM_WRITE, mmio_syscon_wr, begin = 0x40048000, end = 0x4004C000 - 1 )

  # book access to memory mapped UART, and start server to model read and write

  emu.hook_add( unicorn.UC_HOOK_MEM_READ,  mmio_uart_rd,   begin = 0x40008000, end = 0x4000C000 - 1 )
  emu.hook_add( unicorn.UC_HOOK_MEM_WRITE, mmio_uart_wr,   begin = 0x40008000, end = 0x4000C000 - 1 )

  server = socket.socket( socket.AF_INET, socket.SOCK_STREAM ) ; server.bind( ( args.host, args.port ) ) ; server.listen( 1 ) ; ( client, _ ) = server.accept()

  # program emulator

  img = intelhex.IntelHex( args.file )
  
  for addr in img.addresses() :
    emu.mem_write( addr, chr( img[ addr ] ) )
  
  # reset   emulator

  ( tos, ) = struct.unpack( '<I', emu.mem_read( 0x00000000, 4 ) )
  ( rst, ) = struct.unpack( '<I', emu.mem_read( 0x00000004, 4 ) )

  emu.reg_write( unicorn_arm.UC_ARM_REG_R0,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R1,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R2,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R3,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R4,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R5,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R6,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R7,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R8,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R9,  0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R10, 0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R11, 0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R12, 0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R13, 0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R14, 0   )
  emu.reg_write( unicorn_arm.UC_ARM_REG_R15, 0   )
  
  emu.reg_write( unicorn_arm.UC_ARM_REG_SP,  tos )

  # start   emulator

  emu.emu_start( rst, 2 ** 32 - 1 )  

