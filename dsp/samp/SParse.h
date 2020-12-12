#pragma once

#include <string>
#include <memory>


class SDataInstrument {

};


using SDataInstrumentPtr = std::shared_ptr<SDataInstrument>;
class SLex;
using SLexPtr = std::shared_ptr<SLex>;

class SParse
{
public:
    static SDataInstrumentPtr go(const std::string& s);

private:
    static SDataInstrumentPtr matchStart(SLexPtr);
    // must be called with next item is a tag
    static SDataInstrumentPtr finishMatchStart(SLexPtr);

    static SDataInstrumentPtr parseGlobal(SDataInstrumentPtr, SLexPtr);

    static SDataInstrumentPtr matchRegions(SDataInstrumentPtr, SLexPtr);
    // return 0 = got one, 1 = all done, 2 = error
    static int matchRegion(SDataInstrumentPtr, SLexPtr);

};