#include "rack.hpp"
#include "AudioDelay.hpp"

using namespace rack;
		
struct CircularRide : Module {

	enum ParamIds {
        PARAM_DELAY,
        PARAM_FEEDBACK,
        PARAM_RATIO,
		PARAM_DRY,
		PARAM_WET,
        PARAM_DOWN,
		PARAM_ALGORITHM,
        PARAM_UP,
        PARAM_TYPE,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_LEFT,
        INPUT_RIGHT,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_LEFT,
        OUTPUT_RIGHT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};
		CircularRide();
		AudioDelay audioDelay;
		void process(const ProcessArgs &) override;
        dsp::SchmittTrigger upTrigger,downTrigger;
		AudioDelayParameters adp;
};

struct DetectAlgorithmQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;

	private:
		int value;
		std::string v;
};

struct AlgorithmDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	CircularRide* module;
	const int fh = 12; // font height

	AlgorithmDisplay(Vec );

	AlgorithmDisplay(Vec , unsigned char , unsigned char , unsigned char , unsigned char );

	void setColor(unsigned char , unsigned char , unsigned char , unsigned char );

	void draw(const DrawArgs &) override;

	void drawBackground(const DrawArgs &);

	void drawValue(const DrawArgs &, const char * );
};

struct Toggle2P : SvgSwitch {
	int pos;
	int neg;

	Toggle2P();

	// handle the manually entered values
	void onChange(const event::Change &) override;

	// override the base randomizer as it sets switches to invalid values.
	void randomize() override;
};