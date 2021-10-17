//***********************************************************************************************
//Common constants and structs for Batumi
//***********************************************************************************************
#include "RPJ.hpp"

using simd::float_4;

const int MODULE_WIDTH=10;

template <typename T>
struct LFO {
	private:
		double phase;
		T frequency;
		T pw;
		T amplitude;
		T resetState;
	public:	
		LFO();
    	void process(float_4);
		T sin();
		T tri();
		T sqr();
		T sawup();
		T sawdown();
		T light();
		void step(float);
		void setPitch(T);
		void setPhase(T, T);
		void setPhase(T);
		void setAmplitude(T);
		void setFrequency(T);
		void setReset(T);
		T getPhase();
		T getAmplitude();
		T getFrequency();
};



struct ModeValuePair {
	float minValue, maxValue;
};

struct ModeValues {
	ModeValuePair prevModePair,modePair;
};

struct RPJLFO : Module {
	public:
		enum ParamIds {
			FREQ1_PARAM,
			FREQ2_PARAM,
			FREQ3_PARAM,
			FREQ4_PARAM,
			MODE_PARAM,
			NUM_PARAMS,
		};

		enum LightsIds {
			ENUMS(FREQ1_LIGHT,4),
			ENUMS(FREQ2_LIGHT,4),
			ENUMS(FREQ3_LIGHT,4),
			ENUMS(FREQ4_LIGHT,4),
			NUM_LIGHTS,
		};
    
		enum OutputIds {
			SQR_OUTPUT1,
			SQR_OUTPUT2,
			SQR_OUTPUT3,
			SQR_OUTPUT4,
			SAWUP_OUTPUT1,
			SAWUP_OUTPUT2,
			SAWUP_OUTPUT3,
			SAWUP_OUTPUT4,
			SAWDOWN_OUTPUT1,
			SAWDOWN_OUTPUT2,
			SAWDOWN_OUTPUT3,
			SAWDOWN_OUTPUT4,
			SIN_OUTPUT1,
			SIN_OUTPUT2,
			SIN_OUTPUT3,
			SIN_OUTPUT4,
			TRI_OUTPUT1,
			TRI_OUTPUT2,
			TRI_OUTPUT3,
			TRI_OUTPUT4,			
			NUM_OUTPUTS,
		};
    
		enum InputIds {
			FRQ_PH_DIV1_INPUT,
			FRQ_PH_DIV2_INPUT,
			FRQ_PH_DIV3_INPUT,
			FRQ_PH_DIV4_INPUT,
			RESET1_INPUT,
			RESET2_INPUT,
			RESET3_INPUT,
			RESET4_INPUT,		
			NUM_INPUTS,
		};
		enum ModeIds {
			FREE_MODE,
			QUAD_MODE,
			PHASE_MODE,
			DIVIDE_MODE,
			NUM_MODES,
		};
		void process(const ProcessArgs &) override;
		RPJLFO();
		ModeValues getModeValues(ModeIds, ModeIds);
		ModeValuePair getModeValuePair(ModeIds);
 		ModeIds mode;
	private:
		ModeIds prevMode;
		dsp::ClockDivider lightDivider;
		dsp::SchmittTrigger resetTrigger[4];
		LFO<float_4> oscillator[4];
		float freqParam;
		float_4 pitch, pitch0, cvInput, v;
		ParamQuantity* parameter[4];
		ModeValues modeValues;
		ModeValuePair mvp;
		ModeValues mv;
};

struct BGKnob : RoundKnob {
	std::string _svgBase;

	BGKnob(int dim);

	void redraw();
};

 struct Knob16 : BGKnob {
	Knob16();
};

struct ModeQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;
	private:
		int value;
		std::string v;
};

struct FreqQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;
	private:
		int value;
		std::string v;
};
