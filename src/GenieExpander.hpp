#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int WIDTH=400;
const int HEIGHT=370;

struct XpanderPairs {
	std::pair<float, float> edges[4][2];
    double weight;
	int nrOfPendulums;
};


struct Root : SvgWidget {
    private:
		NVGcolor massColor;
	public:
		Root();
		void onDragHover(const DragHoverEvent &) override;
		void drawLayer(const DrawArgs &,int) override;
		void step() override;
    	void setColor(NVGcolor massColor);
		Vec* rootPos;
        int history;
        int elapsed;
		int node;
		float weight;
};



struct Mass : Widget {
    private:
		NVGcolor massColor;
	public:
		Mass();
    	void drawLayer(const DrawArgs &args,int) override;
		//void onDragHover(const DragHoverEvent &) override;
		void onDragEnd(const DragEndEvent &) override;
    	void setColor(NVGcolor massColor);
		void step() override;
        int history;
        int elapsed=0;
		int node;
		float weight;
};

struct Joint: Widget {
	    private:
		NVGcolor jointColor;
        float thick;
		Vec positionBegin,positionEnd;
	public:
		Joint();
    	void drawLayer(const DrawArgs &args,int) override;
        void setWeight(float);
		void setBegin(Vec);
		void setEnd(Vec);
        int elapsed=0;
};

struct GenieExpander : Module {

	enum ParamIds {
		PARAM_HISTORY,
		PARAM_HISTORYTIMER,
		PARAM_PEND_1_X,
		PARAM_PEND_1_Y,
		PARAM_PEND_2_X,
		PARAM_PEND_2_Y,
		PARAM_PEND_3_X,
		PARAM_PEND_3_Y,
		PARAM_PEND_4_X,
		PARAM_PEND_4_Y,
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

	public:
		GenieExpander();
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void process(const ProcessArgs &) override;
        void getPendulums();
		dsp::ClockDivider historyTimer;
        XpanderPairs* rdMsg;
        bool drawLines;
        int maxHistory;
        float swarmThickness;
		Vec XY[4];
		Vec prevXY[4];
	private:
};

struct GenieDisplay : OpaqueWidget {
	GenieDisplay();
	void process();
	void drawLayer(const DrawArgs &args,int) override;
	void onDragHover(const DragHoverEvent&) override;
	void step() override;
	void root2Mass(Root*);
	float xpos, ypos;
    GenieExpander* module;
	Root* roots[4];
};

struct GenieExpanderModuleWidget : ModuleWidget {
	GenieExpanderModuleWidget(GenieExpander*);
	void appendContextMenu(Menu *) override;
	void onDragHover(const DragHoverEvent &) override;
	GenieDisplay *display;
};
