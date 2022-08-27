#include "rack.hpp"
#include "Display.hpp"

using namespace rack;

const int WIDTH=400;
const int HEIGHT=370;

struct Root;
struct GenieExpander;

struct XpanderPairs {
	std::pair<float, float> edges[4][2];
    double weight;
	int nrOfPendulums;
};

struct Joint: Widget {
	    private:
		NVGcolor jointColor;
        float thick;
		Vec positionBegin,positionEnd;
		GenieExpander* module;
	public:
		Joint(GenieExpander *);
    	void drawLayer(const DrawArgs &args,int) override;
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
        int getPendulums();
		int nrOfPendulums;
		int nrOfNodes;
		std::pair<float, float> edges[4][2];
		NVGcolor nodeColors[5];
		float weight;
		bool parentConnected;
		dsp::ClockDivider historyTimer;
        XpanderPairs* rdMsg;
        bool drawLines;
        int maxHistory;
        float swarmThickness;
		Vec XY[4];
		Vec prevXY[4];
		float _sampleR,_sampleG,_sampleB;
		float colors[4][3];
		float jointColor[3];
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

struct Mass : Widget {
    private:
		NVGcolor massColor;
	public:
		Mass(GenieExpander*,int,int);
    	void drawLayer(const DrawArgs &args,int) override;
		//void onDragHover(const DragHoverEvent &) override;
		void onDragEnd(const DragEndEvent &) override;
    	void setColor(NVGcolor massColor);
		void step() override;
        int history;
        int elapsed;
		int node; //Is this really needed?
		float weight;
};

struct Root : SvgWidget {
    private:
		NVGcolor massColor;
	public:
		Root(GenieExpander*,int);
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

struct ColorQuantity : Quantity {
	GenieExpander* _module;
	int node;
	int _rgb;

	ColorQuantity(GenieExpander* m,int n, int rgb);
	void setValue(float value) override;
	float getValue() override;
	std::string getLabel() override;
	float getMinValue() override { return 0.0f; }
	float getMaxValue() override { return 255.0f; }
	float getDefaultValue() override { return 100.0f; }
	float getDisplayValue() override { return roundf(getValue()); }
	void setDisplayValue(float displayValue) override { setValue(displayValue); }
	std::string getUnit() override { return ""; }
};

struct ColorSlider : ui::Slider {
	int colorPos;
	NVGcolor sliderColor;
	int node;

	ColorSlider(GenieExpander* module,int n,int rgb); 

	void draw(const DrawArgs &args) override;

	virtual ~ColorSlider();
};

struct colorMenuSlider : MenuItem {
	GenieExpander* _module;
	int node;

	colorMenuSlider(GenieExpander*, const char*,int n);
	colorMenuSlider(GenieExpander*, const char*);

	Menu* createChildMenu() override; 
};
