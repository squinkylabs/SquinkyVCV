

#include "RingBuffer.h"
#include "asserts.h"


static void testConstruct()
{
	RingBuffer<int, 4> rb;
    assert(rb.empty());
    assert(!rb.full());


    RingBuffer<char *, 1> rb2;
}

static void testSimpleAccess()
{
    RingBuffer<int, 4> rb;
    rb.push(55);
    assert(!rb.empty());
    assert(!rb.full());

    int x = rb.pop();
    assertEQ(x, 55);

    assert(rb.empty());
    assert(!rb.full());
}

static void testMultiAccess()
{
    RingBuffer<int, 4> rb;
    rb.push(1234);
    rb.push(5678);
    assert(!rb.empty());
    assert(!rb.full());

    int x = rb.pop();
    assertEQ(x, 1234);

    assert(!rb.empty());
    assert(!rb.full());

    x = rb.pop();
    assertEQ(x, 5678);

    assert(rb.empty());
    assert(!rb.full());
}

static void testWrap()
{
    RingBuffer<int, 4> rb;
    rb.push(1234);
    rb.push(5678);
    rb.pop();
    rb.pop();
    rb.push(1);
    rb.push(2);
    rb.push(3);

    assertEQ(rb.pop(), 1);
    assertEQ(rb.pop(), 2);
    assertEQ(rb.pop(), 3);
   
    assert(rb.empty());
}

static void testFull()
{
    RingBuffer<int, 4> rb;
    rb.push(1234);
    rb.push(5678);
    rb.pop();
    rb.pop();
    rb.push(1);
    rb.push(2);
    rb.push(3);
    rb.push(4);
    assert(rb.full());
    assert(!rb.empty());

    assertEQ(rb.pop(), 1);
    assertEQ(rb.pop(), 2);
    assertEQ(rb.pop(), 3);
    assertEQ(rb.pop(), 4);

    assert(rb.empty());
    assert(!rb.full());
}

static void testOne()
{
    char * p = "foo";
    RingBuffer<const char*, 1> rb;
    rb.push(p);
    assert(!rb.empty());
    assert(rb.full());

    assertEQ(rb.pop(), p);
    assert(rb.empty());
    assert(!rb.full());
}


void testRingBuffer()
{
	testConstruct();
    testSimpleAccess();
    testMultiAccess();
    testWrap();
    testFull();
    testOne();
}