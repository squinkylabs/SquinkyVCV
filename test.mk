# makefile fragment to make test.exe, the unit test program.
TEST_SOURCES = $(wildcard test/*.cpp)
TEST_SOURCES += $(wildcard dsp/**/*.cpp)
TEST_SOURCES += $(wildcard dsp/third-party/falco/*.cpp)
TEST_SOURCES += $(wildcard sqsrc/**/*.cpp)
TEST_SOURCES += dsp/third-party/kiss_fft130/tools/kiss_fftr.c
TEST_SOURCES += dsp/third-party/kiss_fft130/kiss_fft.c

## This is a list of full paths to the .o files we want to build
TEST_OBJECTS = $(patsubst %, build_test/%.o, $(TEST_SOURCES))

build_test/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build_test/%.c.o: %.c
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Always define _PERF for the perf tests.
perf.exe : PERFFLAG = -D _PERF

# Turn off asserts for perf, unless user overrides on command line
perf.exe : FLAGS += $(ASSERTOFF)

FLAGS += $(PERFFLAG)

# this probably goes away with 0.6.1
# but for now our execs need to dynlink to pthread
test.exe : LDFLAGS += -lpthread

test : test.exe

## Note that perf and test targets both used build_test for object files,
## So you need to be careful to delete/clean when switching between the two.
## Consider fixing this in the future.
perf : perf.exe

## cleantest will clean out all the test and perf build products
cleantest :
	rm -rfv build_test
	rm -fv test.exe
	rm -fv perf.exe

test.exe : $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

perf.exe : $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)