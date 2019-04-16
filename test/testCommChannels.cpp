
#include "asserts.h"
#include "CommChannels.h"

uint32_t buffer[1];

#if 0
static void debug()
{
    CommChannelSend ch;
    ch.send(100);
    ch.send(200);
    ch.send(300);
    for (int i = 0; i < 15; ++i) {
        ch.go(buffer);
        printf("output[%d] = %d\n", i, buffer[0]);
    }
}
#endif

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
    uint32_t test3 = 0xaa551124;

    CommChannelSend ch;
    ch.go(buffer);
    assertEQ(buffer[0], 0);

    ch.send(test1);
    ch.send(test2);
    ch.send(test3);
    ch.go(buffer);
    assertEQ(buffer[0], test1);

    for (int i = 0; i < CommChannelSend::zeroPad; ++i) {
        ch.go(buffer);
        assertEQ(buffer[0], 0);
    }

    ch.go(buffer);
    assertEQ(buffer[0], test2);

    for (int i = 0; i < CommChannelSend::zeroPad; ++i) {
        ch.go(buffer);
        assertEQ(buffer[0], 0);
    }

    ch.go(buffer);
    assertEQ(buffer[0], test3);

    for (int i = 0; i < CommChannelSend::zeroPad +10; ++i) {
        ch.go(buffer);
        assertEQ(buffer[0], 0);
    }

}

static void testRx0()
{
    CommChannelReceive ch;
    buffer[0] = 0;
    for (int i = 0; i < 20; ++i) {
        assertEQ(ch.rx(buffer), 0);
    }
}


static void testRx1()
{
    CommChannelReceive ch;
    buffer[0] = 0;
    for (int i = 0; i < 4; ++i) {
        assertEQ(ch.rx(buffer), 0);
    }

    buffer[0] = 55;
    assertEQ(ch.rx(buffer), 55);
}


void testCommChannels()
{
   // debug();
    testSend1();
    testSend2();
    testRx0();
    testRx1();
}