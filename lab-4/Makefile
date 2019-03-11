# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

SOURCES_C    = $(wildcard *.c   )
TARGETS_C    = $(patsubst %.c,    %,       ${SOURCES_C}   )
SOURCES_JAVA = $(wildcard *.java)
TARGETS_JAVA = $(patsubst %.java, %.class, ${SOURCES_JAVA})

${TARGETS_C}    : %       : %.c %.h
	@gcc -Wall -std=gnu99 -O3 -o ${@} $(filter %.c, ${^}) -lgmp -lcrypto

${TARGETS_JAVA} : %.class : %.java
	@javac ${^}

all   :             ${TARGETS_C} ${TARGETS_JAVA}

clean :
	@rm -f core ${TARGETS_C} ${TARGETS_JAVA} *.pyo *.pyc
