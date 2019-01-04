#pragma once

/**
 * Interface that must be implemented by all composites.
 * Enables compiling for v1
 */
class IComposite
{
public:
    class Config
    {
    public:
        Config(float a, float b, float c)
        {
            min=a;
            max=b;
            def=c;
        }
        float min=0;
        float max=0;
        float def=0;
    };
    virtual Config getParam(int i)=0;
    virtual int getNumParams()=0;
};