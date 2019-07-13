
SLUG = squinkylabs-plug1

VERSION = 1.0.2

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -I./dsp/generators -I./dsp/utils -I./dsp/filters
FLAGS += -I./dsp/third-party/falco -I./dsp/third-party/kiss_fft130 
FLAGS += -I./dsp/third-party/kiss_fft130/tools -I./dsp/third-party/src
FLAGS += -I./sqsrc/thread -I./dsp/fft -I./composites
FLAGS += -I./sqsrc/noise -I./sqsrc/util -I./sqsrc/clock -I./sqsrc/grammar -I./sqsrc/delay
FLAGS += -I./midi/model -I./midi/view -I./midi/controller -I./util
FLAGS += -I./src/third-party -I.src/ctrl
CFLAGS +=
CXXFLAGS +=

# compile for V1 vs 0.6
FLAGS += -D __V1x

# Command line variable to turn on "experimental" modules
ifdef _EXP
	FLAGS += -D _EXP
endif

# Macro to use on any target where we don't normally want asserts
ASSERTOFF = -D NDEBUG

# Make _ASSERT=true will nullify our ASSERTOFF flag, thus allowing them
ifdef _ASSERT
	ASSERTOFF =
endif

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS += -lpthread

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard dsp/**/*.cpp)
SOURCES += $(wildcard dsp/third-party/falco/*.cpp)
xxSOURCES += dsp/third-party/src/minblep.cpp
SOURCES += dsp/third-party/kiss_fft130/kiss_fft.c
SOURCES += dsp/third-party/kiss_fft130/tools/kiss_fftr.c
SOURCES += $(wildcard sqsrc/**/*.cpp)
SOURCES += $(wildcard midi/**/*.cpp)
SOURCES += $(wildcard src/third-party/*.cpp)
SOURCES += $(wildcard src/seq/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# If RACK_DIR is not defined when calling the Makefile, default to two levels above
RACK_DIR ?= ../..

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# This turns asserts off for make (plugin), not for test or perf
$(TARGET) :  FLAGS += $(ASSERTOFF)

$(TARGET) : FLAGS += -D __PLUGIN

# mac does not like this argument
ifdef ARCH_WIN
	FLAGS += -fmax-errors=5
endif

include test.mk

