#pragma once

#include <memory>
#include <string>

class SqCommand
{
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    std::string name = "Seq++";
};

using CommandPtr = std::shared_ptr<SqCommand>;