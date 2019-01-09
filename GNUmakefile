# This makefile uses GNU "make" & GCC (both free) on both Windows & Linux.
# To "make" from Windows:  gmake -C \\SMB-1\Usr\dev\cpp\radio

.SECONDARY:
# Even with the strict compile checking and warnings selected,
# the program should compile with no warnings issued.

ifeq	"$(OS)" "Windows_NT"	# Includes Windows NT & later
  CL	 = cl.exe #		# Visual Studio 2005 Express (free)
  CDEBUG =-DNDEBUG
  CFLAGS =-nologo -J -Zap1 -GFry -Oxsi -Wall
  LFLAGS =-MT -Fe
  RM	= del
else
  RM	= rm -f
endif

CL	= gcc
CFLAGS	=-funsigned-char -pedantic -Wpadded -Wall -Wextra -Wno-unused-parameter $(CDEBUG)
LFLAGS	=-lstdc++ -static -s -o 

all:
	$(CL)	-m32 $(CFLAGS) Radio.cpp I*.cpp Th*.cpp $(LFLAGS)Radio2csv-x86
ifeq	"$(OS)" "Windows_NT"
	$(CL)	-m64 $(CFLAGS) Radio.cpp I*.cpp Th*.cpp $(LFLAGS)Radio2csv-x64
else
	@$(RM)	*.obj
endif
