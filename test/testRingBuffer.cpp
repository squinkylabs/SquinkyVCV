

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


#if 0
// is the intial output correct
static void dt2()
{
	DelayLine<int, 4> dl(1);
	assertEQ (dl.get(), 0);
}


// is the intial output correct (non default)
static void dt3()
{
	DelayLine<int, 4> dl(1, 123);
	assert (dl.get() == 123);
}


//clock it and check results
static void dt4()
{

	printf("starting dt4\n");
	DelayLine<int, 4> dl(1);
	assert (dl.get() == 0);

	
	dl.clock(100);
	assert (dl.get() == 0);

	dl.clock(200);
	assert (dl.get() == 100);
}


//clock all the way around 2 delay
static void dt5()
{

	printf("starting dt4\n");
	DelayLine<int, 4> dl(2);
	assert (dl.get() == 0);

	dl.clock(100); 
	assert (dl.get() == 0);

	dl.clock(200);
	assert (dl.get() == 0);

	dl.clock(300);
	assert (dl.get() == 100);

	
	dl.clock(400);
	assert (dl.get() == 200);

	
	dl.clock(500);
	assert (dl.get() == 300);

	
	dl.clock(600);
	assert (dl.get() == 400);

}
#endif





void testRingBuffer()
{
	testConstruct();
    testSimpleAccess();
}