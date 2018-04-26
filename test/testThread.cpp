
#include "asserts.h"
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "ThreadClient.h"

#include <assert.h>
#include <memory>



// test that we can build and tear down.
static void test0()
{
    assertEQ(ThreadSharedState::_dbgCount, 0);
    {
        std::shared_ptr<ThreadSharedState> noise = std::make_shared<ThreadSharedState>();
        std::unique_ptr<ThreadServer> server(new ThreadServer(noise));
        std::unique_ptr<ThreadClient> client(new ThreadClient(noise, std::move(server)));
    }
    assertEQ(ThreadSharedState::_dbgCount, 0);
}

static void test1()
{
    for (int i = 0; i < 200; ++i)
        test0();
}

/**************************************************************************/

// client will send to server
class Test1Message : public ThreadMessage
{
public:
    Test1Message() : ThreadMessage(Type::TEST1)
    {
    }
    int payload = 0;
};

// client will send to server
class Test2Message : public ThreadMessage
{
public:
    Test2Message() : ThreadMessage(Type::TEST2)
    {
    }
};

class TestServer : public ThreadServer
{
public:
    TestServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state)
    {
    }
    void handleMessage(const ThreadMessage& msg)
    {
        assert(false);
    }
};

static void test2()
{
    // Set up all the objects
    std::unique_ptr<Test1Message> msg(new Test1Message());
    std::shared_ptr<ThreadSharedState> state = std::make_shared<ThreadSharedState>();
    std::unique_ptr<TestServer> server(new TestServer(state));
    std::unique_ptr<ThreadClient> client(new ThreadClient(state, std::move(server)));

    // now pump some message through.
    msg->payload = 100;
    
    printf("will send\n");
    // poll until can send, then send
    for (bool done = false; !done; ) {
        bool b = client->sendMessage(msg.get());
        if (b) {
            done = true;
        }
    }

    printf("will wait ack\n");

    for (bool done = false; !done; ) {
        auto rxmsg = client->getMessage();
        if (rxmsg) {
            done = true;
            assert(rxmsg->type == ThreadMessage::Type::TEST1);
            Test1Message* tmsg = reinterpret_cast<Test1Message *>(rxmsg);
            assert(tmsg->payload == 1100);
        }
    }
}

/*****************************************************************/

void testThread()
{
   assertEQ(ThreadSharedState::_dbgCount, 0);
   assertEQ(ThreadMessage::_dbgCount, 0);
   test0();
   test1();
   test2();
   assertEQ(ThreadSharedState::_dbgCount, 0);
   assertEQ(ThreadMessage::_dbgCount, 0);
}