#pragma once

#include <memory>

class NoiseServer
{
public:
    NoiseServer(std::shared_ptr<NoiseSharedState> state) : sharedState(state)
    {
    }

    const NoiseServer& operator= (const NoiseServer&) = delete;
    NoiseServer(const NoiseServer&) = delete;

private:
    std::shared_ptr<NoiseSharedState> sharedState;
};