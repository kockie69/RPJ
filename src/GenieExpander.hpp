#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int WIDTH=395;
const int HEIGHT=370;

struct XpanderPairs {
	std::pair<float, float> edges[4][2];
    double weight;
};

struct Mass : Widget {
    private:
		NVGcolor massColor;
        float weight;
	public:
    	void drawLayer(const DrawArgs &args,int) override;
    	void setColor(NVGcolor massColor);
        void setWeight(float);
        int history;
        int elapsed=0;
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
		float X[4],Y[4];
		float prevX[4],prevY[4];
	private:
};

struct GenieDisplay : TransparentWidget {
	GenieDisplay() {};
	void process();
	void drawLayer(const DrawArgs &args,int) override;
	void onDragStart(const DragStartEvent&) override;
	float xpos, ypos;
    GenieExpander * module;
};

struct GenieExpanderModuleWidget : ModuleWidget {
	GenieExpanderModuleWidget(GenieExpander*);
	void appendContextMenu(Menu *) override;
	GenieDisplay *display;
};
