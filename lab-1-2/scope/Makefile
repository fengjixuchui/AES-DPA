# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

DEVICE := $(shell if [ -e "/dev/scale-scope" -a -r "/dev/scale-scope" -a -w "/dev/scale-scope" ] ; then echo "/dev/scale-scope" ; else echo "" ; fi)

ifeq "${DEVICE}" ""
  $(warning /dev/scale-scope does not exist, or cannot be read from or written to)
endif

TRACE = trace.dat

acquire        : acquire.c acquire.h
	@gcc -I /opt/picoscope/include/libps2000a-1.1/ -L /opt/picoscope/lib/ -std=gnu99 -o acquire acquire.c -lps2000a

acquire-app    :
	@picoscope
acquire-api-py : acquire.py
	@python acquire.py ${TRACE}
acquire-api-c  : acquire
	@./acquire         ${TRACE}

plot  :
	@gnuplot -e "set datafile separator ','" -e "plot '${TRACE}' using 1:3 with lines, '${TRACE}' using 1:2 with lines" -p

clean :
	@rm -f ${TRACE} acquire
