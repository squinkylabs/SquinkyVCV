#pragma once

class TextDisplayBase : public widget::OpaqueWidget {
public:
    TextDisplayBase();
    void draw(const DrawArgs& args) override;
protected:
    std::string text;
    //std::string placeholder;
    bool multiline = true;
   
};

inline TextDisplayBase::TextDisplayBase() {
    box.size.y = BND_WIDGET_HEIGHT;
}

inline void TextDisplayBase::draw(const DrawArgs& args) {
    INFO("TestDisplayBase");
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

class TextDisplay : public TextDisplayBase {
public:
  

protected:
    std::shared_ptr<Font> font;
    math::Vec textOffset;
    NVGcolor color;
};

////////////////////////////////////////////////

class StyledTextDisplay : public TextDisplay {
public:
    void draw(const DrawArgs& args) override;
};

#if 0
void StyledTextDisplay::draw(const DrawArgs& args) {
    INFO("styled draw");
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5.0);
    nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
    nvgFill(args.vg);

    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
   // Widget::draw(args);
    TextDisplayBase::draw(args);
    nvgResetScissor(args.vg);
}
#endif

void StyledTextDisplay::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	// Background
	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5.0);
	nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
	nvgFill(args.vg);


	// Text
	if (font->handle >= 0) {
		bndSetFont(font->handle);

		NVGcolor highlightColor = color;
	//	highlightColor.a = 0.5;
    // TODO: handle no text
	    int begin = 1;
     //   int end = text.size();
        int end = 0;
		bndIconLabelCaret(args.vg, textOffset.x, textOffset.y,
		                  box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
	                     -1, color, 12, text.c_str(), highlightColor, begin, end);
     //   BNDwidgetState state = BND_ACTIVE;
     //   bndTextField(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);

		bndSetFont(APP->window->uiFont->handle);
	}


	nvgResetScissor(args.vg);
}

class TextDisplaySamp : public StyledTextDisplay {
public:
    TextDisplaySamp();
    void setText(const std::string& s) {
        text = s;
    }
};

inline TextDisplaySamp::TextDisplaySamp() {
	font = APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
//	color = nvgRGB(0xff, 0xd7, 0x14);

  //  float r = 255.0 / 238.0;
 //   color = nvgRGB(r * 48, r * 125, r * 238);
   // int add = 255 - 238;
 //   color = nvgRGB(add + 48, add + 125, add + 238);
    int add = 75;
     color = nvgRGB(add + 48, add + 125, 255);
	textOffset = math::Vec(5, 5);
}
