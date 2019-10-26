#include "InputControls.h"


float InputPopupMenuParamWidget::getValue() const
{
    int index = 0;
    for (auto label : labels) {
        if (this->text == label) {
            return index;
        }
        ++index;
    }
    assert(false);
    return 0;
}

float CheckBox::getValue() const
{
    return value ? 1.f : 0.f;
}

void CheckBox::draw(const Widget::DrawArgs &args)
{
    NVGcontext *const ctx = args.vg;

    nvgShapeAntiAlias(ctx, true);

    nvgBeginPath(ctx);
#if 1
    nvgMoveTo(ctx, 0, 0);
    nvgLineTo(ctx, box.size.x, 0);
    nvgLineTo(ctx, box.size.x, box.size.y);
    nvgLineTo(ctx, 0, box.size.y);
    nvgLineTo(ctx, 0, 0);
#else
    nvgMoveTo(ctx, box.pos.x, box.pos.y);
    nvgLineTo(ctx, box.pos.x + box.size.x, box.pos.y);
    nvgLineTo(ctx, box.pos.x + box.size.x, box.pos.y + box.size.y);
    nvgLineTo(ctx, box.pos.x, box.pos.y + box.size.y);
    nvgLineTo(ctx, box.pos.x, box.pos.y);
#endif
    
    nvgStrokeColor(ctx, UIPrefs::TIME_LABEL_COLOR);
    //  nvgStrokePaint
    nvgStrokeWidth(ctx, 1);

    nvgStroke(ctx);
}

void CheckBox::onDragStart(const ::rack::event::DragStart& e) 
{
    DEBUG("start");
}
void CheckBox::onDragEnd(const ::rack::event::DragEnd& e)
{
 DEBUG("end");
}
void CheckBox::onDragDrop(const ::rack::event::DragDrop& e)
{
 DEBUG("drop");
}