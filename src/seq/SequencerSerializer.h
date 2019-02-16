#pragma once

class json_t;
class SequencerSerializer
{

public:
    static json_t *toJson();
    static void fromJson(json_t *rootJ);
};