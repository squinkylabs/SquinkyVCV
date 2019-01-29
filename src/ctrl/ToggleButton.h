#pragma once

class ToggleButton : public ParamWidget
{
public:
    ToggleButton();
    /**
     * SVG Images must be added in order
     */
    void addSvg(const char* resourcePath);

    void draw(NVGcontext *vg) override;
    void onMouseDown(EventMouseDown &e) override;
private:
    using SvgPtr = std::shared_ptr<SVGWidget>;
    std::vector<SvgPtr> svgs;
};

inline ToggleButton::ToggleButton()
{
    this->box.size = Vec(0, 0);
}

inline void ToggleButton::addSvg(const char* resourcePath)
{
    auto svg = std::make_shared<SVGWidget>();
    svg->setSVG(SVG::load(assetPlugin(pluginInstance, resourcePath)));
    svgs.push_back(svg);
    this->box.size.x = std::max(this->box.size.x, svg->box.size.x);
    this->box.size.y = std::max(this->box.size.y, svg->box.size.y);
}

inline void ToggleButton::draw(NVGcontext *vg)
{
    int index = int(std::round(value));
    auto svg = svgs[index];
    svg->draw(vg);
}

inline void ToggleButton::onMouseDown(EventMouseDown &e)
{
   // printf("mouse down at %.2f,%.2f\n", e.pos.x, e.pos.y);

    e.consumed = false;
    const int index = int(std::round(value));
    const Vec pos(e.pos.x, e.pos.y);
#if 0
    printf(" -- svg x=%.2f, y=%.2f width=%.2f height = %.2f\n",
        svgs[index]->box.pos.x, svgs[index]->box.pos.y,
        svgs[index]->box.size.x, svgs[index]->box.size.y
    );
    printf(" -- button x=%.2f, y=%.2f width=%.2f height = %.2f\n",
        this->box.pos.x, this->box.pos.y,
        this->box.size.x, this->box.size.y
    );
    fflush(stdout);
#endif

    if (!svgs[index]->box.contains(pos)) {
        return;
    }
    e.consumed = true;
    auto v = this->value;
    if (++v >= svgs.size()) {
        v = 0;

    }
    setValue(v);
}