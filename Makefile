SLUG = squinkylabs-plug1
VERSION = 0.5.0

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -I./dsp/generators -I./dsp/utils -I./dsp/filters
FLAGS += -I./dsp/third-party/falco
CFLAGS += 
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard dsp/**/*.cpp)
SOURCES += $(wildcard dsp/third-party/falco/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# This doesn't work yet
# test: $(TARGET)
#     SOURCES = $(wildcard test/src/*.cpp)
#    TARGET = test.exe
    
# Include the VCV plugin Makefile framework
include ../../plugin.mk


    
