#pragma once

// Helper class to make ports compatible between 0.6 and 1.0
class SqPort
{
public:
    static bool isConnected(Port& input);
};

inline bool SqPort::isConnected(Port& input)
{
    return input.isConnected();
}