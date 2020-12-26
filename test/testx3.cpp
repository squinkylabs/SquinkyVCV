
#include "asserts.h"
#include "VelSwitch.h"

static void testVelSwitch1()
{
    VelSwitch v;

    v._addIndex(0, 0);           // index zero starts at vel 0
    v._addIndex(1, 100);         // and goes to vel 100
    v._addIndex(2, 128);         // intex 1 goes all the way
    
    assertEQ(v.mapVelToIndex(0), 0);
    assertEQ(v.mapVelToIndex(1), 0);
    assertEQ(v.mapVelToIndex(99), 0);
    assertEQ(v.mapVelToIndex(100), 1);
}

void testx3()
{
    testVelSwitch1();
}