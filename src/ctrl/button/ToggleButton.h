#include "rack.hpp"

class ToggleButton : public ::rack::app::SvgSwitch
{
public:
    ::rack::app::CircularShadow* shadowToDelete = nullptr;

    ~ToggleButton()
    {
        if (shadowToDelete) {
            delete shadowToDelete;
        }
    }
    
    ToggleButton()
    {
        // The default shadow gives a look we don't want 
        auto shadowToDelete = this->shadow;
        this->fb->removeChild(shadowToDelete);

        // old one had default size 0
        this->box.size.y = 0;
        this->box.size.x = 0;
    }

    // Old switch took relative paths into plugin bundle
    void addSvg(const char* resPath)
    {
        addFrame(APP->window->loadSvg(resPath));
    }

    void draw(const DrawArgs& args) override {
	    nvgGlobalTint(args.vg, rack::color::WHITE);
        ::rack::app::SvgSwitch::draw(args);
    }
};
