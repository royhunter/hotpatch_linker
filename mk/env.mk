PLATFORM = HOST



ifeq ($(PLATFORM), HOST)
	CROSS_COMPILE =
endif


CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld
STRIP = $(CROSS_COMPILE)strip
OBJDUMP = $(CROSS_COMPILE)objdump
NM = $(CROSS_COMPILE)nm


BUILD_VERBOSE = y
DEBUG_VERSION = y

ifeq ($(BUILD_VERBOSE),y)
    Q =
	MAKE_DEBUG = 
else
    Q = @
	MAKE_DEBUG = -s
endif

ifeq ($(DEBUG_VERSION),y)
    DEBUG_FLAGS = -g
else
    DEBUG_FLAGS = -Os
endif



PROJ_CFLAGS = -I$(TOP)/include


CFLAGS += $(DEBUG_FLAGS) -Wall $(PROJ_CFLAGS)






