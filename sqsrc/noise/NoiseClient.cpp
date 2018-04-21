
#include "NoiseClient.h"
#include "NoiseServer.h"
#include "NoiseSharedState.h"

std::atomic<int> NoiseSharedState::_dbgCount=0;

NoiseClient::~NoiseClient()
{

}