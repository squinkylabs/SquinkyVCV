#pragma once

#include <string>
#include <vector>
class SqWaveFile
{
public:
    SqWaveFile();
    bool load(const std::string& path);
    float getAt(int index) const
    {
        return data[index];
    }
    int size() const
    {
        return (int) data.size();
    }
private:
    std::vector<float> data;
    //class Impl;
    //std::unique_ptr<Impl> pimp;
};