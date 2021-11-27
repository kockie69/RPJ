#include "rack.hpp"
#include "AudioDelay.hpp"

const int MODULE_WIDTH=11;

using namespace rack;

const char *JSON_DELAY_ALGORITHM_KEY="Algorithm";
const char *JSON_ENABLE_LPF_KEY="LPF";
const char *JSON_ENABLE_HPF_KEY="HPF";

std::string delayAlgorithmTxt[static_cast<int>(delayAlgorithm::numDelayAlgorithms)] = { "Normal", "PingPong", "LCRDelay", "TapDelay"};

struct CircularRide : Module {

	enum ParamIds {
        PARAM_DELAYL,
		PARAM_DELAYC,
		PARAM_DELAYR,
        PARAM_FEEDBACK,
        PARAM_RATIO,
		PARAM_DRY,
		PARAM_WET,
        PARAM_DOWN,
		PARAM_ALGORITHM,
        PARAM_UP,
        PARAM_TYPE,
		PARAM_LPFFC,
		PARAM_HPFFC,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_LEFT,
        INPUT_RIGHT,
		INPUT_SYNC,
		INPUT_DELAYLCV,
		INPUT_DELAYCCV,
		INPUT_DELAYRCV,
		INPUT_RATIOCV,
		INPUT_FBCV,
		INPUT_DRYCV,
		INPUT_LPFCV,
		INPUT_WETCV,
		INPUT_HPFCV,
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
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void process(const ProcessArgs &) override;
		template <typename T>
		void processChannel(Input&, Input&, Output&, Output&);
		void onSampleRateChange() override;

        dsp::SchmittTrigger upTrigger,downTrigger,syncTrigger;
		AudioDelayParameters adp;
		AudioDelay<rack::simd::float_4> audioDelay[4];
		bool enableLPF = true;
		bool enableHPF = true;
		std::string strAlgorithm;
		float prevSampleTime;
		float delta;
};

struct DetectAlgorithmQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;

	private:
		int value;
		std::string v;
};

struct AlgorithmDisplay : TransparentWidget {
	//std::shared_ptr<Font> font;
	NVGcolor txtCol;
	CircularRide* module;
	const int fh = 12; // font height

	AlgorithmDisplay(Vec );

	AlgorithmDisplay(Vec , unsigned char , unsigned char , unsigned char , unsigned char );

	void setColor(unsigned char , unsigned char , unsigned char , unsigned char );

	void drawLayer(const DrawArgs &, int) override;

	void drawBackground(const DrawArgs &);

	void drawValue(const DrawArgs &, const char * );
};

struct Toggle2P : SvgSwitch {
	int pos;
	int neg;

	Toggle2P();

	// handle the manually entered values
	void onChange(const event::Change &) override;
};

/* Context Menu Item for changing the LPF setting */
struct nLPFMenuItem : MenuItem {
	CircularRide *module;
	bool EnableLPF;
	void onAction(const event::Action &) override;
};

/* Context Menu Item for changing the HPF setting */
struct nHPFMenuItem : MenuItem {
	CircularRide *module;
	bool EnableHPF;
	void onAction(const event::Action &) override;
};