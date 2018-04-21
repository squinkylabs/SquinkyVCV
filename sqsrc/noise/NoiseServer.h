#pragma once

#include <memory>
#include <thread>

class NoiseSharedState;
class NoiseMessage;

class NoiseServer
{
public:
    NoiseServer(std::shared_ptr<NoiseSharedState> state);
    ~NoiseServer();
    void start();

    const NoiseServer& operator= (const NoiseServer&) = delete;
    NoiseServer(const NoiseServer&) = delete;

private:
    std::shared_ptr<NoiseSharedState> sharedState;
    std::unique_ptr<std::thread> thread;

    void threadFunction();

    /**
     * returns true to exit thread
     */
    bool procMessage(const NoiseMessage&);
};