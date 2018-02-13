/**
  * Unit test entry point
  */

#include <stdio.h>
#include <assert.h>

extern void testBiquad();
extern void testSaw();
extern void testLookupTable();
extern void testSinOscillator();
extern void testHilbert();
extern void testAudioMath();

void foo(int, int);

int main(int, char *)
{
    // While this code may work in 32 bit applications, it's not tested for that.
    // Want to be sure we are testing the case we care about.
    assert(sizeof(size_t)==8);

    testAudioMath();
    testBiquad();
    testSaw();
    testLookupTable();
    testSinOscillator();
    testHilbert();
    printf("press any key to continue\n");
    getchar();
}