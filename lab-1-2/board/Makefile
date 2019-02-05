# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

DEVICE := $(shell if [ -e "/dev/scale-board" -a -r "/dev/scale-board" -a -w "/dev/scale-board" ] ; then echo "/dev/scale-board" ; else echo "" ; fi)

ifeq "${DEVICE}" ""
  $(warning /dev/scale-board does not exist, or cannot be read from or written to)
endif

ifeq "${TARGET}" ""
  $(warning TARGET environment variable is not set)
endif

PROJECT = benchmark

  build-${PROJECT}      :
	@make --no-builtin-rules -f ${TARGET}/build/lib/scale.mk BSP="${TARGET}/build" USB="/dev/scale-board" PROJECT="${PROJECT}" all
  clean-${PROJECT}      :
	@make --no-builtin-rules -f ${TARGET}/build/lib/scale.mk BSP="${TARGET}/build" USB="/dev/scale-board" PROJECT="${PROJECT}" clean

program-${PROJECT}      :
	@make --no-builtin-rules -f ${TARGET}/build/lib/scale.mk BSP="${TARGET}/build" USB="/dev/scale-board" PROJECT="${PROJECT}" program
program-${PROJECT}-term :
	@putty -serial -sercfg 9600,8,n,1,N /dev/scale-board
