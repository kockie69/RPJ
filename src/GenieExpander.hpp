#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int MODULE_WIDTH=26;
const int WIDTH=375;
const int HEIGHT=350;

struct xpanderPairs {
	std::pair<double, double> edges[2];
	double mass;
};

struct GenieExpander : Module {

	enum ParamIds {

		NUM_PARAMS,
	};

	enum InputIds {

		NUM_INPUTS,
	};

	enum OutputIds {

		NUM_OUTPUTS,
	};

	enum LightIds {
		NUM_LIGHTS
	};


	GenieExpander();
	void process(const ProcessArgs &) override;
	void setDisplay(void);
    std::pair<double, double> first_edge;
    std::pair<double, double> second_edge;
	double mass;
};

struct GenieDisplay : TransparentWidget {
	GenieDisplay() {
		xpos = WIDTH/2;
		ypos = HEIGHT/2;
	}
	void process();
	void drawLayer(const DrawArgs &args,int) override;
	float xpos, ypos;
    GenieExpander * module;
};

struct GenieExpanderModuleWidget : ModuleWidget {
	GenieExpanderModuleWidget(GenieExpander*);
};

