#include "rack.hpp"

class RedButton : public ToggleButton {
public:
    RedButton() {
		rack::app::Switch::momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/oval-button-up-grey.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/oval-button-down.svg")));
    }
};