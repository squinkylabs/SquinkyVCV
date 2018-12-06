#pragma once

class ToggleButton : public ParamWidget
{
public:
    /**
     * SVG Images must be added in order
     */
    void addSvg(const char* resourcePath);

    void draw(NVGcontext *vg) override;
    void onMouseDown( EventMouseDown &e ) override;
private:
    using SvgPtr = std::shared_ptr<SVGWidget>;
    std::vector<SvgPtr> svgs;
};

inline void ToggleButton::addSvg(const char* resourcePath)
{
    auto svg = std::make_shared<SVGWidget>();
    svg->setSVG(SVG::load (assetPlugin(plugin, resourcePath)));
    svgs.push_back(svg);
}

inline void ToggleButton:: draw(NVGcontext *vg)
{
    int index = int( std::round(value));
    auto svg = svgs[index];
    svg->draw(vg);
}

inline void ToggleButton::onMouseDown( EventMouseDown &e )
{
    e.consumed = false;
    const int index = int( std::round(value));
    const Vec pos(e.pos.x, e.pos.y);
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