
SLUG = squinkylabs-plug1

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -I./dsp/generators -I./dsp/utils -I./dsp/filters
FLAGS += -I./dsp/third-party/falco -I./dsp/third-party/kiss_fft130 
FLAGS += -I./dsp/third-party/kiss_fft130/tools -I./dsp/third-party/src -I./dsp/third-party/midifile
FLAGS += -I./dsp/third-party/flac/include
FLAGS += -I./dsp -I./dsp/samp -I./dsp/third-party/pugixml
FLAGS += -I./sqsrc/thread -I./dsp/fft -I./composites
FLAGS += -I./sqsrc/noise -I./sqsrc/util -I./sqsrc/clock -I./sqsrc/grammar -I./sqsrc/delay
FLAGS += -I./midi/model -I./midi/view -I./midi/controller -I./util
FLAGS += -I./src/third-party -I.src/ctrl -I./src/kbd
CFLAGS +=
CXXFLAGS +=

# compile for V1 vs 0.6
FLAGS += -D __V1x 
FLAGS += -D _SEQ
FLAGS += -D FLAC__NO_DLL -D HAVE_FSEEKO -D HAVE_LROUND

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

# this doesn't work on mac. Let's back this experiment out for now.
#LDFLAGS += -latomic

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard dsp/**/*.cpp)
SOURCES += $(wildcard dsp/third-party/falco/*.cpp)
SOURCES += $(wildcard dsp/third-party/midifile/*.cpp)
SOURCES += dsp/third-party/kiss_fft130/kiss_fft.c
SOURCES += dsp/third-party/kiss_fft130/tools/kiss_fftr.c
SOURCES += $(wildcard sqsrc/**/*.cpp)
SOURCES += $(wildcard midi/**/*.cpp)
SOURCES += $(wildcard src/third-party/*.cpp)
SOURCES += $(wildcard src/seq/*.cpp)
SOURCES += $(wildcard src/kbd/*.cpp)
SOURCES += $(wildcard dsp/third-party/flac/src/*.c)
# SOURCES += dsp/third-party/flac/src/*.c

# include res and presets folder
DISTRIBUTABLES += $(wildcard LICENSE*) res presets

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

#  -flto
FLAGS += -finline-limit=500000 -finline-functions-called-once -flto
LDFLAGS += -flto

# ---- for ASAN on linux, just uncomment
# using the static library is a hack for getting the libraries to load.
# Trying other hacks first is a good idea. Building Rack.exe with asan is a pretty easy way
# ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -Wno-format-security
# FLAGS += $(ASAN_FLAGS)
# LDFLAGS += $(ASAN_FLAGS)
# LDFLAGS += -static-libasan

include test.mk

