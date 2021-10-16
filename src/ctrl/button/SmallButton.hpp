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
