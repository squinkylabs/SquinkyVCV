
#include "asserts.h"
#include "CommChannels.h"

uint32_t buffer[1];
size_t payload[1];

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
  
   // uint32_t test = 0xaa551122;
    CommChannelSend ch;
    ch.go(buffer, payload);
    assertEQ(buffer[0], 0);

    CommChannelMessage test;
    test.commandId = 0xaa551122;
    ch.send(test);
    ch.go(buffer, payload);
    assertEQ(buffer[0], test.commandId);

    ch.go(buffer, payload);
    assertEQ(buffer[0], 0);
    
}


static void testSend2()
{
    buffer[0] = 13;
    payload[0] = 99;
    
    uint32_t test1 = 0xaa551122;
    size_t payload1 = 0x221155aa334455;
    uint32_t test2 = 0xaa551123;
    size_t payload2 = 0x221155aa334456;
    uint32_t test3 = 0xaa551124;
    size_t payload3 = 0x221155aa334457;



    CommChannelSend ch;
    ch.go(buffer, payload);
    assertEQ(buffer[0], 0);

    CommChannelMessage test;
    test.commandId = test1;
    test.commandPayload = payload1;
    ch.send(test);

    test.commandId = test2;
    test.commandPayload = payload2;
    ch.send(test);

    test.commandId = test3;
    test.commandPayload = payload3;
    ch.send(test);

    ch.go(buffer, payload);
    assertEQ(buffer[0], test1);

    for (int i = 0; i < CommChannelSend::zeroPad; ++i) {
        ch.go(buffer, payload);
        assertEQ(buffer[0], 0);
    }

    ch.go(buffer, payload);
    assertEQ(buffer[0], test2);
    assertEQ(payload[0], payload2);

    for (int i = 0; i < CommChannelSend::zeroPad; ++i) {
        ch.go(buffer, payload);
        assertEQ(buffer[0], 0);
    }

    ch.go(buffer, payload);
    assertEQ(buffer[0], test3);
    assertEQ(payload[0], payload3);

    for (int i = 0; i < CommChannelSend::zeroPad +10; ++i) {
        ch.go(buffer, payload);
        assertEQ(buffer[0], 0);
    }

}


static void testRx0()
{
    CommChannelReceive ch;
    CommChannelMessage msg;
    msg.commandId = 33;
    msg.commandPayload = 55;
    buffer[0] = 0;
    for (int i = 0; i < 20; ++i) {
        //assertEQ(ch.rx(buffer, payload), 0);
        bool b = ch.rx(buffer, payload, msg);
        assert(!b);
        assert(msg.commandId == 33);
        assert(msg.commandPayload == 55);
    }
}


static void testRx1()
{
    CommChannelReceive ch;
    CommChannelMessage msg;
    buffer[0] = 0;
    payload[0] = 0;
    for (int i = 0; i < 4; ++i) {
       // assertEQ(ch.rx(buffer), 0);
        bool b = ch.rx(buffer, payload, msg);
        assert(!b);
        assert(msg.commandId == 0);
        assert(msg.commandPayload == 0);
    }

    buffer[0] = 55;
    payload[0] = 1234567;

    bool b = ch.rx(buffer, payload, msg);
    assert(b);
    assert(msg.commandId == 55);
    assert(msg.commandPayload == 1234567);
}

static void testRxTwoInARowNoZeros()
{
    CommChannelReceive ch;
    CommChannelMessage msg;
    buffer[0] = 0;
    for (int i = 0; i < 4; ++i) {
        //assertEQ(ch.rx(buffer), 0);
        bool isMessage = ch.rx(buffer, payload, msg);
        assert(!isMessage);
    }

    buffer[0] = 55;
   // assertEQ(ch.rx(buffer), 55);
    bool isMessage = ch.rx(buffer, payload, msg);
    assert(isMessage);
    assertEQ(msg.commandId, 55);


  //  assertEQ(ch.rx(buffer), 0);
    isMessage = ch.rx(buffer, payload, msg);
    assert(!isMessage);
}

void testCommChannels()
{
   // debug();
    testSend1();

    testSend2();
    testRx0();
    testRx1();
    testRxTwoInARowNoZeros();

}