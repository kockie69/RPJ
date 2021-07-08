#include "rack.hpp"

using namespace rack;

const double kSqrtTwo = pow(2.0, 0.5);
const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */


struct ATextLabel : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	char text[128];
	const int fh = 14;

	ATextLabel(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
		setText(" ");
	}

	ATextLabel(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		setColor(r, g, b, a);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
		setText(" ");
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void setText(const char * txt) {
		strncpy(text, txt, sizeof(text));
		box.size.x = strlen(text) * 8;
	}

	void drawBG(const DrawArgs &args) {
		Vec c = Vec(box.size.x/2, box.size.y);
		const int whalf = box.size.x/2;

		// Draw rectangle
		nvgFillColor(args.vg, nvgRGBA(0xF0, 0xF0, 0xF0, 0xFF));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, c.x -whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y +2);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);
	}

	void drawTxt(const DrawArgs &args, const char * txt) {

		Vec c = Vec(box.size.x/2, box.size.y);

		nvgFontSize(args.vg, fh);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));

		nvgText(args.vg, c.x, c.y+fh, txt, NULL);
	}

	void draw(const DrawArgs &args) override {
		TransparentWidget::draw(args);
		drawBG(args);
		drawTxt(args, text);
	}
};

enum biquadAlgorithm { kDirect, kCanonical, kTransposeDirect, kTransposeCanonical }; //  4 types of biquad calculations, constants (k)

enum class filterAlgorithm {
	kLPF1P, kLPF1, kHPF1, kLPF2, kHPF2, kBPF2, kBSF2, kButterLPF2, kButterHPF2, kButterBPF2,
	kButterBSF2, kMMALPF2, kMMALPF2B, kLowShelf, kHiShelf, kNCQParaEQ, kCQParaEQ, kLWRLPF2, kLWRHPF2,
	kAPF1, kAPF2, kResonA, kResonB, kMatchLP2A, kMatchLP2B, kMatchBP2A, kMatchBP2B,
	kImpInvLP1, kImpInvLP2, numFilterAlgorithms
}; 

std::string filterAlgorithmTxt[static_cast<int>(filterAlgorithm::numFilterAlgorithms)] = { "LPF1P", "LPF1", "HPF1", "LPF2", "HPF2", "BPF2", "BSF2", 
		"ButterLPF2", "ButterHPF2", "ButterBPF2", "ButterBSF2", "MMALPF2", "MMALPF2B", "LowShelf",
		"HiShelf", "NCQParaEQ", "CQParaEQ", "LWRLPF2", "LWRHPF2", "APF1", "APF2", "ResonA", "ResonB",
		"MatchLP2A", "MatchLP2B", "MatchBP2A", "MatchBP2B", "ImpInvLP1", "ImpInvLP2" };

enum filterCoeff { a0, a1, a2, b1, b2, c0, d0, numCoeffs };

enum {x_z1, x_z2, y_z1, y_z2, numStates };

struct BiquadParameters
{
	BiquadParameters () {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	/*BiquadParameters& operator=(const BiquadParameters& params)
	{
		if (this == &params)
			return *this;

		biquadCalcType = params.biquadCalcType;
		return *this;
	}*/

	biquadAlgorithm biquadCalcType = biquadAlgorithm::kDirect; ///< biquad structure to use
};

struct Biquad {
	public:
	Biquad() {}		/* C-TOR */
	~Biquad() {}	/* D-TOR */

	// --- IAudioSignalProcessor FUNCTIONS --- //
	//
	/** reset: clear out the state array (flush delays); can safely ignore sampleRate argument - we don't need/use it */
	virtual bool reset(double);

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through biquad to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn);

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return BiquadParameters custom data structure
	*/
	BiquadParameters getParameters() { return parameters ; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param BiquadParameters custom data structure
	*/
	void setParameters(const BiquadParameters& _parameters){ parameters = _parameters; }

	// --- MUTATORS & ACCESSORS --- //
	/** set the coefficient array NOTE: passing by pointer to array; allows us to use "array notation" with pointers i.e. [ ] */
	void setCoefficients(double* coeffs){
		// --- fast block memory copy:
		memcpy(&coeffArray[0], &coeffs[0], sizeof(double)*numCoeffs);
	}

	/** get the coefficient array for read/write access to the array (not used in current objects) */
	double* getCoefficients()
	{
		// --- read/write access to the array (not used)
		return &coeffArray[0];
	}

	/** get the state array for read/write access to the array (used only in direct form oscillator) */
	double* getStateArray()
	{
		// --- read/write access to the array (used only in direct form oscillator)
		return &stateArray[0];
	}

	/** get the structure G (gain) value for Harma filters; see 2nd Ed FX book */
	double getG_value() { return coeffArray[a0]; }

	/** get the structure S (storage) value for Harma filters; see 2nd Ed FX book */
	double getS_value();// { return storageComponent; }

protected:
	/** array of coefficients */
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	/** array of state (z^-1) registers */
	double stateArray[numStates] = { 0.0, 0.0, 0.0, 0.0 };

	/** type of calculation (algorithm  structure) */
	BiquadParameters parameters;

	/** for Harma loop resolution */
	double storageComponent = 0.0;
};


struct AudioFilterParameters
{
	AudioFilterParameters(){}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	/*AudioFilterParameters& operator=(const AudioFilterParameters& params)	// need this override for collections to work
	{
		if (this == &params)
			return *this;
		algorithm = params.algorithm;
		fc = params.fc;
		Q = params.Q;
		boostCut_dB = params.boostCut_dB;

		return *this;
	}*/

	// --- individual parameters
	filterAlgorithm algorithm = filterAlgorithm::kMatchLP2A; ///< filter algorithm
	std::string strAlgorithm = "kMatchLP2A";

	double fc = 100.0; ///< filter cutoff or center frequency (Hz)
	double Q = 0.707; ///< filter Q
	double boostCut_dB = 0.0; ///< filter gain; note not used in all types
};



struct AudioFilter {
public:
	AudioFilter() {}		/* C-TOR */
	~AudioFilter() {}		/* D-TOR */

	// --- IAudioSignalProcessor
	/** --- set sample rate, then update coeffs */
	virtual bool reset(double _sampleRate)
	{
		BiquadParameters bqp = biquad.getParameters();

		// --- you can try both forms - do you hear a difference?
		bqp.biquadCalcType = biquadAlgorithm::kTransposeCanonical; //<- this is the default operation
	//	bqp.biquadCalcType = biquadAlgorithm::kDirect; //<- this is the direct form that implements the biquad directly
		biquad.setParameters(bqp);

		sampleRate = _sampleRate;
		return biquad.reset(_sampleRate);
	}

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double);

	/** --- sample rate change necessarily requires recalculation */
	virtual void setSampleRate(double _sampleRate)
	{
		if (sampleRate!=_sampleRate) {
			sampleRate = _sampleRate;
			calculateFilterCoeffs();
		}
	}

	/** --- get parameters */
	AudioFilterParameters getParameters() { return audioFilterParameters; }

	/** --- set parameters */
	void setParameters(const AudioFilterParameters& parameters)
	{
		if (audioFilterParameters.algorithm != parameters.algorithm ||
			audioFilterParameters.boostCut_dB != parameters.boostCut_dB ||
			audioFilterParameters.fc != parameters.fc ||
			audioFilterParameters.Q != parameters.Q)
		{
			// --- save new params
			audioFilterParameters = parameters;
		}
		else
			return;

		// --- don't allow 0 or (-) values for Q
		if (audioFilterParameters.Q <= 0)
			audioFilterParameters.Q = 0.707;

		// --- update coeffs
		calculateFilterCoeffs();
	}

	/** --- helper for Harma filters (phaser) */
	double getG_value() { return biquad.getG_value(); }

	/** --- helper for Harma filters (phaser) */
	double getS_value() { return biquad.getS_value(); }

protected:
	// --- our calculator
	Biquad biquad; ///< the biquad object

	// --- array to hold coeffs (we need them too)
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; ///< our local copy of biquad coeffs

	// --- object parameters
	AudioFilterParameters audioFilterParameters; ///< parameters
	double sampleRate = 44100.0; ///< current sample rate

	/** --- function to recalculate coefficients due to a change in filter parameters */
	bool calculateFilterCoeffs();
};

struct LadyNina : Module {

	enum ParamIds {
		PARAM_UP,
		PARAM_DOWN,
		PARAM_FC,
		PARAM_Q,
		PARAM_BOOSTCUT_DB,
		PARAM_DRY,
		PARAM_WET,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_MAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		LadyNina();
		AudioFilter audioFilter;
		void process(const ProcessArgs &) override;
		dsp::SchmittTrigger upTrigger,downTrigger;
		AudioFilterParameters afp;
};

struct AFilterNameDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	LadyNina* module;
	const int fh = 12; // font height


	AFilterNameDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(0xff, 0xff, 0xff, 0xff);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	AFilterNameDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(r, g, b, a);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}

	void draw(const DrawArgs &args) override {
		char tbuf[11];

		if (module == NULL) return;

		std::snprintf(tbuf, sizeof(tbuf), "%s", &module->audioFilter.getParameters().strAlgorithm[0]);
		
		TransparentWidget::draw(args);
		drawBackground(args);
		drawValue(args, tbuf);

	}

	void drawBackground(const DrawArgs &args) {
		Vec c = Vec(box.size.x/2, box.size.y);
		int whalf = 2.25*box.size.x;
		int hfh = floor(fh / 2);

		// Draw rounded rectangle
		nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, c.x -whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y +2);
			nvgQuadTo(args.vg, c.x +whalf +5, c.y +2+hfh, c.x +whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(args.vg, c.x -whalf -5, c.y +2+hfh, c.x -whalf, c.y +2);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0x0F));
		nvgStrokeWidth(args.vg, 1.f);
		nvgStroke(args.vg);
	}

	void drawValue(const DrawArgs &args, const char * txt) {
		Vec c = Vec(box.size.x/2, box.size.y);

		nvgFontSize(args.vg, fh);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		nvgText(args.vg, c.x, c.y+fh-1, txt, NULL);
	}
};

