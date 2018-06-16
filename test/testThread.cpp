
#include "asserts.h"
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "ThreadClient.h"
#include "ThreadPriority.h"

#include <assert.h>
#include <memory>
#include <vector>



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
    // Set up all the objects
    std::unique_ptr<Test1Message> msg(new Test1Message());
    std::shared_ptr<ThreadSharedState> state = std::make_shared<ThreadSharedState>();
    std::unique_ptr<TestServer> server(new TestServer(state));
    std::unique_ptr<ThreadClient> client(new ThreadClient(state, std::move(server)));

    for (int count = 0; count < 50; ++count) {
        msg->payload = 100 + count;
        const int expectedPayload = msg->payload + 1000;
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
                assertEQ(tmsg->payload, expectedPayload);
            }
        }
    }
}

// not a real test
static void test3()
{
    bool b = ThreadPriority::boostNormal();
    bool b2 = ThreadPriority::boostRealtime();
    ThreadPriority::restore();
}

std::atomic<bool> stopNow;
std::atomic<int> count;

double xxx, yyy;

std::atomic<int> slow;

std::atomic<int> fast;


void t4(bool iAmIt)
{
   // printf("t4 called with %d\n", iAmIt);

    if (iAmIt && true) {
        printf("boosting\n"); fflush(stdout);
        ThreadPriority::boostNormal();
    }
    while (!stopNow) {
        for (int i = 0; i < 100000; ++i) {
            yyy = yyy + (double) rand();
        }

        if (iAmIt) {
            ++fast;
        } else {
            ++slow;
        }
    }
}

static void test4()
{
    stopNow = false;
    count = 0;
    xxx = 0;
    yyy = 0;
    slow = 0;
    fast = 0;
    int numSlow = 0;
    std::vector< std::shared_ptr<std::thread>> threads;

    threads.push_back(std::make_shared<std::thread>(t4, true));
    for (int i = 0; i < 9; ++i) {
        threads.push_back(std::make_shared<std::thread>(t4, false));
        ++numSlow;
    }

    printf("started all\n");
    xxx = 0;
    yyy = 0;

    std::this_thread::sleep_for(std::chrono::seconds(20));
    stopNow = true;

    for (auto thread : threads) {
        thread->join();
    }

    printf("slow/fast was %f (%d) ratio=%d\n", (double) slow / (double) fast, (int) slow, numSlow);
}
/*****************************************************************/

void testThread(bool extended)
{
    assertEQ(ThreadSharedState::_dbgCount, 0);
    assertEQ(ThreadMessage::_dbgCount, 0);
    test0();
    test1();
    test2();
    if (extended) {
        test3();
        test4();
    }
    assertEQ(ThreadSharedState::_dbgCount, 0);
    assertEQ(ThreadMessage::_dbgCount, 0);
}