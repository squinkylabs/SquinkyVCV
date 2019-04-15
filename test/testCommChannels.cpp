
#include "asserts.h"
#include "CommChannels.h"

uint32_t buffer[1];

static void testSend1()
{
    buffer[0] = 13;
  
    uint32_t test = 0xaa551122;
    CommChannelSend ch;
    ch.go(buffer);
    assertEQ(buffer[0], 0);
    ch.send(test);
    ch.go(buffer);
    assertEQ(buffer[0], test);

    ch.go(buffer);
    assertEQ(buffer[0], 0);
    
}

static void testSend2()
{
    buffer[0] = 13;

    uint32_t test1 = 0xaa551122;
    uint32_t test2 = 0xaa551123;

    CommChannelSend ch;
    ch.go(buffer);
    assertEQ(buffer[0], 0);

    ch.send(test1);
    ch.send(test2);
    ch.go(buffer);
    assertEQ(buffer[0], test1);

    ch.go(buffer);
    assertEQ(buffer[0], 0);

    ch.go(buffer);
    assertEQ(buffer[0], test2);

    ch.go(buffer);
    assertEQ(buffer[0], 0);
}

void testCommChannels()
{
    testSend1();
    testSend2();
}