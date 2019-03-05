

#include "SqClipboard.h"


std::shared_ptr<SqClipboard::Track> SqClipboard::getTrackData()
{
    return trackData;
}

void  SqClipboard::putTrackData(std::shared_ptr<Track> newData)
{
    clear();
    trackData = newData;
}

void  SqClipboard::clear()
{
    trackData = nullptr;
}

//private:

std::shared_ptr<SqClipboard::Track> SqClipboard::trackData;
