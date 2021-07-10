
#include "GMRScreenHolder.h"


GMRScreenHolder::GMRScreenHolder(const Vec &pos, const Vec &size) {
    this->box.pos = pos;
    this->box.size = size;

   // Vec pos2(40, 10);
    child1 = new FakeScreen(pos, size, false);
    child2 = new FakeScreen(pos, size, true);
}

void
GMRScreenHolder::draw(const DrawArgs &args) {
    auto vg = args.vg;
    nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);

    childPos += .002;
    if (childPos > 1) {
        childPos = 0;
    }
    //
    float width = this->box.size.x;
  //  float x = 2 * (-width/2 + childPos * width);
  //  float x2 = x + width;
    const float x1 = (childPos -1) * width;
    const float x2 = childPos * width;

    const float ch1ClipX = width * (1 - childPos);
    const float ch1ClipW = width * childPos;

    const float ch2ClipX = 0;
    const float ch2ClipW = width * (1 - childPos);
#if 0
    {
        static int count = 0;
        if (!count) {
            SQINFO("x=%f %f", x1, x2);
            ++count;
            if (count > 20) {
                count = 0;
            }
        }
    }
#endif

    nvgSave(vg);
    nvgTranslate(vg, x1, 0);
    nvgScissor(vg, ch1ClipX, 0, ch1ClipW, this->box.size.y);
    child1->draw(args);
    nvgRestore(vg);

    nvgSave(vg);
    nvgTranslate(vg, x2, 0);
    nvgScissor(vg, ch2ClipX, 0, ch2ClipW, this->box.size.y);
    child2->draw(args);
    nvgRestore(vg);

    OpaqueWidget::draw(args);
}