
#include "asserts.h"
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "ThreadClient.h"

#include <assert.h>
#include <memory>



// test that we can build and tear down.
static void test0()
{
   // printf("testthread 0\n");
    assertEQ(ThreadSharedState::_dbgCount, 0);
    {
        std::shared_ptr<ThreadSharedState> noise = std::make_shared<ThreadSharedState>();
        std::unique_ptr<ThreadServer> server(new ThreadServer(noise));
        std::unique_ptr<ThreadClient> client(new ThreadClient(noise, std::move(server)));
    }
    assertEQ(ThreadSharedState::_dbgCount, 0);
  //  printf("testthread 0 done\n");
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
    void handleMessage(ThreadMessage* msg) override
    {
        switch (msg->type) {
            case ThreadMessage::Type::TEST1: 
            {
                Test1Message * tstMsg = static_cast<Test1Message *>(msg);
                assertEQ(tstMsg->payload, nextExpectedPayload);
                ++nextExpectedPayload;
                tstMsg->payload += 1000;
                sendMessageToClient(tstMsg);        // send back the modified one
            }
                break;
            default:
                assert(false);
        }
    }
    int nextExpectedPayload = 100;
};

static void test2()
{
    printf("Enter testThread2\n");
    // Set up all the objects
    std::unique_ptr<Test1Message> msg(new Test1Message());
    std::shared_ptr<ThreadSharedState> state = std::make_shared<ThreadSharedState>();
    std::unique_ptr<TestServer> server(new TestServer(state));
    std::unique_ptr<ThreadClient> client(new ThreadClient(state, std::move(server)));

    for (int count = 0; count < 50; ++count) {
        msg->payload = 100+count;
        const int expectedPayload = msg->payload + 1000;
        //printf("test loop iter: %d expect %d\n", count, expectedPayload);
        for (bool done = false; !done; ) {
            bool b = client->sendMessage(msg.get());
            if (b) {
                done = true;
            }
        }

        for (bool done = false; !done; ) {
            auto rxmsg = client->getMessage();
            if (rxmsg) {
                done = true;
                assert(rxmsg->type == ThreadMessage::Type::TEST1);
                Test1Message* tmsg = reinterpret_cast<Test1Message *>(rxmsg);
                //printf("test client reading from server %d\n", tmsg->payload);
                assertEQ(tmsg->payload, expectedPayload);
            }
        }
    }
    printf("test leaving testthread2");
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