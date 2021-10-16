#include "rack.hpp"

struct RPJKnob : RoundKnob {
	RPJKnob() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/RPJKnob.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/RPJKnob_bg.svg")));
	}
};


struct RPJKnobSnap : RPJKnob{
	RPJKnobSnap() {
		snap = true;
	}
};