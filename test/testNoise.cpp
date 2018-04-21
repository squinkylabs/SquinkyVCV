
#include "asserts.h"
#include "NoiseSharedState.h"
#include "NoiseServer.h"
#include "NoiseClient.h"
#include <memory>



// test that we can build and tear down.
static void test0()
{
    assertEQ(NoiseSharedState::_dbgCount, 0);
    {
        std::shared_ptr<NoiseSharedState> noise = std::make_shared<NoiseSharedState>();
        std::unique_ptr<NoiseServer> server(new NoiseServer(noise));
        std::unique_ptr<NoiseClient> client(new NoiseClient(noise, std::move(server)));
    }
    assertEQ(NoiseSharedState::_dbgCount, 0);
}

static void test1()
{
    for (int i = 0; i < 50; ++i)
        test0();
}

void testNoise()
{
    test0();
    test1();
}