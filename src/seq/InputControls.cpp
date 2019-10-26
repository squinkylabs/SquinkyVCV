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
    drawBox(ctx);
    if (value) {
        drawX(ctx);
    }
 
}


void CheckBox::drawBox(NVGcontext* ctx)
{
    nvgBeginPath(ctx);
    nvgMoveTo(ctx, 0, 0);
    nvgLineTo(ctx, box.size.x, 0);
    nvgLineTo(ctx, box.size.x, box.size.y);
    nvgLineTo(ctx, 0, box.size.y);
    nvgLineTo(ctx, 0, 0);
    nvgStrokeColor(ctx, UIPrefs::TIME_LABEL_COLOR);
    //  nvgStrokePaint
    nvgStrokeWidth(ctx, 1);
    nvgStroke(ctx);
}

void CheckBox::drawX(NVGcontext* ctx)
{
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, UIPrefs::TIME_LABEL_COLOR);
    nvgStrokeWidth(ctx, 1);

    nvgMoveTo(ctx, 0, 0);
    nvgLineTo(ctx, box.size.x,  box.size.y);

    nvgMoveTo(ctx, box.size.x, 0);
    nvgLineTo(ctx, 0,  box.size.y);
 
    nvgStroke(ctx);
}

void CheckBox::onDragDrop(const ::rack::event::DragDrop& e)
{
    if (e.origin == this) {
		::rack::event::Action eAction;
		onAction(eAction);
	}
}

void CheckBox::onAction(const ::rack::event::Action& e)
{
    value = !value;
}