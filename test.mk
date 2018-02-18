TEST_SOURCES = $(wildcard test/*.cpp)
TEST_SOURCES += $(wildcard dsp/**/*.cpp)
TEST_SOURCES += $(wildcard dsp/third-party/falco/*.cpp)

## This is a list of full paths to the .o files we want to build
TEST_OBJECTS = $(patsubst %, build_test/%.o, $(TEST_SOURCES))

build_test/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test : test.exe

test.exe : $(TEST_OBJECTS)
	echo "about to link with " $(LDFLAGS)
	$(CXX) -o $@ $^
