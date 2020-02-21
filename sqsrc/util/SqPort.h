#pragma once

// Helper class to make ports compatible between 0.6 and 1.0
// GET RID OF THIS

#ifdef __V1x
class SqPort
{
public:
    static bool isConnected(Port& input);
};

inline bool SqPort::isConnected(Port& input)
{
    return input.isConnected();
}
#else

class SqPort
{
public:
    static bool isConnected(Input& input);
    static bool isConnected(Output& input);
};

inline bool SqPort::isConnected(Input& input)
{
   // return input.active;
    return input.isConnected();
}

inline bool SqPort::isConnected(Output& output)
{
   // return output.active;
    return output.isConnected();
}

#endif