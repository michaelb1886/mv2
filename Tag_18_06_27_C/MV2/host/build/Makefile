# File:
#	Makefile
#
# Description:
#	Makefile for MV2 host software
#
# Requires:
#	make
#
# Usage:
#	make [OPT=[DEBUG],[RELEASE]] [LIBXML_DIR=<path>] [all | clean]
#
# Change log:
# 	13.01.16 SD	Original version
#	24.03.16 SD Add MV2_DIR to VPATH
#	29.03.16 SD Add HostUtility.cpp
#	22.04.16 SD Add MXRUtility.cpp
#	28.04.16 SD Add HostResults.cpp, HostScript.cpp
#	15.07.16 PK	Combine Rules and Makefile
#				Allow LIBXML_DIR to be overridden
#				Eliminate computation of WIN32
#				Add DEBUG option
#				Clean target: remove MVHost.exe, ignore errors
#				Clean up warning options
#				Add handling of dependency files
#				Add RELEASE option to optionally link libraries statically
#				Compile static versions for 32-bit architecture
#	16.08.16 SD Add -std=c++0x compile flag
#				Add/Remove source files
#	25.05.20 PK	Update for 64-bit, MSYS2
#
# Tools.
CPP := g++

# Directories.
PROJROOT		?= ./..
MV2_DIR			:= $(PROJROOT)/../instrument/MV2
SRC_DIR 		:= $(PROJROOT)/source
INC_DIR			:= $(PROJROOT)/include
BLD_DIR			:= $(PROJROOT)/build
LIBXML_DIR		?= $(PROJROOT)/libxml

# Directory and file names
VPATH = $(SRC_DIR) $(MV2_DIR)
SRC := MV2Host.cpp
SRC += MV2HostCommands.cpp
SRC += CMxrFile.cpp
SRC += CHostScript.cpp
SRC += CArduinoSerialPort.cpp
OBJ = $(SRC:.cpp=.o)

# Set optimization and symbol options according to DEBUG option
ifneq (,$(findstring DEBUG, $(OPT)))
	CFLAGS := -O0 -g
else
	CFLAGS := -O3
endif

# Set the compile flags
CFLAGS += -Wall -MD -std=c++0x
CFLAGS += -I${INC_DIR}
CFLAGS += -I${MV2_DIR}
CFLAGS += `${LIBXML_DIR}/bin/xml2-config --cflags`

# Detect OS.
OS := $(shell uname)

# Set compile and link flags for the RELEASE option.
ifneq (,$(findstring RELEASE, $(OPT)))

	# Define static mode for libxml2.
	CFLAGS += -DLIBXML_STATIC
	
	# Mac OS.
	ifneq (,$(findstring Darwin, $(OS)))
		LDFLAGS := `${LIBXML_DIR}/bin/xml2-config --libs`
		
	else
		# MSYS2.
		ifneq (,$(findstring MSYS_NT, $(OS)))
			LDFLAGS := -static
			LDFLAGS += -lxml2 -lz -llzma -liconv -lpthread -lm
			LDFLAGS += -licui18n.dll -licuuc.dll -licudata.dll

		# Linux.
		else
			LDFLAGS += -static
			LDFLAGS += `${LIBXML_DIR}/bin/xml2-config --libs`
			LDFLAGS += -static-libgcc
			LDFLAGS += -static-libstdc++

		endif
	endif

# Set compile and link flags if not RELEASE.
else
	LDFLAGS += -L${LIBXML_DIR}/lib -lxml2 
	LDFLAGS += -Wl,-rpath ${LIBXML_DIR}/lib
endif

# Default rule
all: MV2Host

# Main program
MV2Host: ${OBJ}
	${CPP} -o $@ $^ ${LDFLAGS}
	
# Include the dependency files
-include $(OBJ:.o=.d)

# Compilation rule
%.o:%.cpp
	${CPP} -o $@ -c $< ${CFLAGS}

# Clean target
clean:
	-rm *.o
	-rm *.d
	-rm MV2Host*
