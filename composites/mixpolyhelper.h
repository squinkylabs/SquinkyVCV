#pragma once

template <class TMixComposite>
class MixPolyHelper
{
public:
    MixPolyHelper()
    {

    }

    void updatePolyphony(TMixComposite*);
    float getNormalizedInputSum(TMixComposite*, int channel);
private:
};
