
#include "asserts.h"
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "ThreadClient.h"
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

void testThread()
{
   assertEQ(ThreadSharedState::_dbgCount, 0);
   assertEQ(ThreadMessage::_dbgCount, 0);
   test0();
   test1();
   assertEQ(ThreadSharedState::_dbgCount, 0);
   assertEQ(ThreadMessage::_dbgCount, 0);
}