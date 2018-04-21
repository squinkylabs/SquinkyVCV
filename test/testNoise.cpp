
#include "asserts.h"
#include "NoiseSharedState.h"
#include "NoiseServer.h"
#include "NoiseClient.h"
#include <memory>



// test that we can build and tear down.
static void test0()
{
    printf("in test0\n"); fflush(stdout);
    assertEQ(NoiseSharedState::_dbgCount, 0);
    {
        printf("in test1\n"); fflush(stdout);
        std::shared_ptr<NoiseSharedState> noise = std::make_shared<NoiseSharedState>();
        std::unique_ptr<NoiseServer> server(new NoiseServer(noise));
        printf("in test2\n"); fflush(stdout);
        std::unique_ptr<NoiseClient> client(new NoiseClient(noise, std::move(server)));
        printf("in test3\n"); fflush(stdout);
    }
    assertEQ(NoiseSharedState::_dbgCount, 0);
}

void testNoise()
{
    test0();
}