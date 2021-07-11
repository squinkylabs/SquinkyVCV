
#include "GMRMainScreen.h"

#include "../seq/SqGfx.h"

#include "../ctrl/SqHelper.h"

GMRMainScreen::GMRMainScreen(StochasticGrammarPtr g) : grammar(g) {

}


void GMRMainScreen::draw(const DrawArgs &args) {
     auto vg = args.vg;
     SqGfx::drawText2(vg, 30, 130, "main screen", 18, SqHelper::COLOR_WHITE);
}
