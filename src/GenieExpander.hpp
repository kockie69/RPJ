#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int MODULE_WIDTH=26;
const int WIDTH=375;
const int HEIGHT=350;
const int NODES=5;
const int MAXPENDULUMS = 4;

std::pair<double, double> operator +(const std::pair<double, double>& x, const std::pair<double, double>& y) {
    return std::make_pair(x.first + y.first, x.second + y.second);
}

struct mass {
    private:
		std::pair<double,double> position;
    	int size;
		NVGcolor massColor;
	public:
    	void draw(NVGcontext *vg);
    	void setSize(int size);
    	void setPosition(std::pair<double,double> position);
		std::pair<double,double> getPosition(void);
    	void setColor(NVGcolor massColor);
};

struct swarm {
	private:
		int history;
		std::vector<mass> masses;
		void deleteOldestMass();
    	void addNewestMass(mass);
	public:
		swarm();
    	void draw(NVGcontext *vg,NVGcolor massColor);
    	void update(mass newMass, int maxHistory);
};

struct xpanderPairs {
	std::pair<double, double> edges[4][2];
	double mass;
	int nrOfPendulums;
};

struct node {
	private:
		NVGcolor nodeColor;
		int maxHistory;
	public:
		mass newMass;
		swarm nodeSwarm;
		void setColor(NVGcolor color);
		void setMaxhistory(int maxHistory);
		void draw(NVGcontext *vg);
};


struct line {
	private:
		std::pair<double,double> positionBegin, positionEnd;
		NVGcontext *vg;
	public:
		void setBegin(std::pair<double,double>);
		void setEnd(std::pair<double,double>);
		void draw(NVGcontext *);
};

struct pendulum {
	public:
		void draw(NVGcontext *vg);
		void setPosition(std::pair<double,double> position);
		std::pair<double,double> getPosition();
		void setNrOfNodes(int nrOfNodes);
		node nodes[NODES];
		line lines[NODES-1];
	private:
		std::pair<double,double> position;
		int nrOfNodes;
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
	//void setDisplay(void);
	//void doHistory(float);
    //std::pair<double, double> edges[MAXPENDULUMS][EDGES];
	//std::vector<std::pair<double, double>> oldEdges[MAXPENDULUMS][EDGES];
	pendulum pendulums[MAXPENDULUMS];
	double size;
	bool parentConnected;
	int nrOfPendulums;
	dsp::ClockDivider historyTimer;
	int maxHistory;
};

struct GenieDisplay : TransparentWidget {
	GenieDisplay() {
		xpos = (WIDTH/2)+75;
		ypos = HEIGHT/2;
	}
	void process();
	void drawLayer(const DrawArgs &args,int) override;
	//void drawMass(NVGcontext*,NVGcolor,float,float);
	//void drawMass(NVGcontext*,NVGcolor,float,float);
	//void drawSwarm(int,int,NVGcontext *,NVGcolor,float,float);
	//void drawLine(NVGcontext *,float,float,float,float);
	float xpos, ypos;
    GenieExpander * module;
};

struct GenieExpanderModuleWidget : ModuleWidget {
	GenieExpanderModuleWidget(GenieExpander*);
	GenieDisplay *display;
};

