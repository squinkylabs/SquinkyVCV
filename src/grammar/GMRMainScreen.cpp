
#include "GMRMainScreen.h"

#include "../seq/SqGfx.h"

#include "../ctrl/SqHelper.h"

void GMRMainScreen::draw(const DrawArgs &args) {
     auto vg = args.vg;
     SqGfx::drawText2(vg, 30, 130, "main screen", 18, SqHelper::COLOR_WHITE);
}
