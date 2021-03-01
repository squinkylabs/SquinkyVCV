#pragma once

class TestDisplayBase : public widget::OpaqueWidget {
public:
    TestDisplayBase();
    std::string text;
	//std::string placeholder;
	bool multiline = true;
    void draw(const DrawArgs& args) override;

};


inline TestDisplayBase::TestDisplayBase() {
	box.size.y = BND_WIDGET_HEIGHT;
}

inline void TestDisplayBase::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	BNDwidgetState state;
	if (this == APP->event->selectedWidget)
		state = BND_ACTIVE;
	else if (this == APP->event->hoveredWidget)
		state = BND_HOVER;
	else
		state = BND_DEFAULT;

	//int begin = std::min(cursor, selection);
//	int end = std::max(cursor, selection);
    int begin = 0;
    int end = text.size();
	bndTextField(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);
	// Draw placeholder text
#if 0
	if (text.empty() && state != BND_ACTIVE) {
		bndIconLabelCaret(args.vg, 0.0, 0.0, box.size.x, box.size.y, -1, bndGetTheme()->textFieldTheme.itemColor, 13, placeholder.c_str(), bndGetTheme()->textFieldTheme.itemColor, 0, -1);
	}
    #endif

	nvgResetScissor(args.vg);
}

///////////////////////////////////////////////////////////

class TextDisplay : public TestDisplayBase  {

};
