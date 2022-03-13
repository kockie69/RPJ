#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int MODULE_WIDTH=26;
const int WIDTH=375;
const int HEIGHT=350;
const int EDGES=4;

struct xpanderPairs {
	std::pair<double, double> edges[EDGES];
	double mass;
};

struct GenieExpander : Module {

	enum ParamIds {

		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_1_X,
		INPUT_1_Y,
		INPUT_2_X,
		INPUT_2_Y,
		INPUT_3_X,
		INPUT_3_Y,
		INPUT_4_X,
		INPUT_4_Y,
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
    std::pair<double, double> edges[EDGES];
	double mass;
	bool parentConnected;
};

struct GenieDisplay : TransparentWidget {
	GenieDisplay() {
		xpos = (WIDTH/2)+75;
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

