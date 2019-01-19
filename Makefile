# Simple Makefile for Cygwin with gcc 7

P = radio2csv
CFLAGS = -funsigned-char -pedantic
CFLAGS += -Wpadded -Wall -Wextra -Wno-unused-parameter -Wimplicit-fallthrough=0

all $P:	*.cpp
	$(CXX) $(CFLAGS) $(DEFINES) $^ -o $P

clean:
	rm -f $P $P.exe $P-*.exe
