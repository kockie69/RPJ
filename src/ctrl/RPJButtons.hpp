#include "rack.hpp"

struct buttonPlusSmall : SvgSwitch  {
	buttonPlusSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/ButtonPlus_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/ButtonPlus_1.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

struct buttonMinSmall : SvgSwitch  {
	buttonMinSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/ButtonMin_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/ButtonMin_1.svg")));
	}
	
	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

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

class RedButton : public ToggleButton {
public:
    RedButton() {
		rack::app::Switch::momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/oval-button-up-grey.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/oval-button-down.svg")));
    }
};

struct StartButton : ToggleButton  {
	
	StartButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Start_line_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Start_line_on.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

struct StopButton : ToggleButton  {

	StopButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Stop_line_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Stop_line_on.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

struct PauseButton : ToggleButton  {

	PauseButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Pause_line_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Pause_line_on.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};


struct FwdButton : ToggleButton  {

	FwdButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Fwd_line_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Fwd_line_on.svg")));
	}

		void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

struct RwdButton : ToggleButton  {

	RwdButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Rwd_line_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Rwd_line_on.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};

struct EjectButton : ToggleButton  {

	EjectButton() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Ejct_line_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons/Ejct_line_on.svg")));
	}

	void draw(const DrawArgs &args)override {
		nvgGlobalTint(args.vg, color::WHITE);
		SvgSwitch::draw(args);
	}
};
