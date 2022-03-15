#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int MODULE_WIDTH=26;
const int WIDTH=375;
const int HEIGHT=350;
const int EDGES=4;
const int MAXPENDULUMS = 4;

struct xpanderPairs {
	std::pair<double, double> edges[MAXPENDULUMS][EDGES];
	double mass;
	int nrOfPendulums;
};

struct GenieExpander : Module {

	enum ParamIds {
		PARAM_HISTORY,
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
	void doHistory(float);
    std::pair<double, double> edges[MAXPENDULUMS][EDGES];
	std::vector<std::pair<double, double>> oldEdges[MAXPENDULUMS][EDGES];
	double mass;
	bool parentConnected;
	int nrOfPendulums;
	double historyTimer;
	int maxHistory;
};

struct GenieDisplay : TransparentWidget {
	GenieDisplay() {
		xpos = (WIDTH/2)+75;
		ypos = HEIGHT/2;
	}
	void process();
	void drawLayer(const DrawArgs &args,int) override;
	void drawMass(NVGcontext*,NVGcolor,float,float);
	void drawSwarm(int,int,NVGcontext *,NVGcolor,float,float);
	float xpos, ypos;
    GenieExpander * module;
};

struct GenieExpanderModuleWidget : ModuleWidget {
	GenieExpanderModuleWidget(GenieExpander*);
};

