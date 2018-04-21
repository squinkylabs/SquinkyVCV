#pragma once
#include <memory>

class NoiseSharedState;
class NoiseServer;

class NoiseClient
{
public:
    NoiseClient(std::shared_ptr<NoiseSharedState> state, std::unique_ptr<NoiseServer> server);
    ~NoiseClient();

    const NoiseClient& operator= (const NoiseClient&) = delete;
    NoiseClient(const NoiseClient&) = delete;

private:
    std::shared_ptr<NoiseSharedState> sharedState;
    std::unique_ptr<NoiseServer> _server;
};