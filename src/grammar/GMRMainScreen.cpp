
#include "GMRMainScreen.h"

#include "../seq/SqGfx.h"

#include "../ctrl/SqHelper.h"

GMRMainScreen::GMRMainScreen(StochasticGrammarPtr g) : grammar(g) {
}


void GMRMainScreen::draw(const DrawArgs &args) {
     auto vg = args.vg;
     const char* text = "Main screen - TBD. Will have display of what bar and beat is playing, maybe something cool to show which rules kicked in.";
     SqGfx::drawTextBox(vg, 30, 130, 200, text, 12, SqHelper::COLOR_WHITE);
}
