#include "RPJ.hpp"
#include "CircularRide.hpp"
#include "ctrl/RPJButtons.hpp"
#include "ctrl/RPJKnobs.hpp"

CircularRide::CircularRide() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    
    configParam(PARAM_DELAYL, 0.f, 2000.f,250.f, "Delay");
	configParam(PARAM_DELAYC, 0.f, 2000.f,250.f, "Delay");
	configParam(PARAM_DELAYR, 0.f, 2000.f,250.f, "Delay");
    configParam(PARAM_FEEDBACK, 0.f, 100.f,50.f, "Feedback");
    configParam(PARAM_RATIO, 0.f, 100.f,50.f, "Ratio");
    configParam(PARAM_DRY, -60.f, 12.f,-3.f, "Dry","dB");
    configParam(PARAM_WET, -60.f, 12.f,-3.f, "Wet","dB");
	configParam<DetectAlgorithmQuantity>(PARAM_ALGORITHM, 0.f, 1.f, 0.f, "Algorithm");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
    configParam(PARAM_TYPE, 0.f, 1.f,0.f, "Delay Update Type");
	configParam(PARAM_LPFFC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	configParam(PARAM_HPFFC, 20.f, 20480.f, 1000.f, "fc"," Hz");
	for (int i=0;i<4;i++) {
		audioDelay[i].reset(APP->engine->getSampleRate());
		audioDelay[i].createDelayBuffers(APP->engine->getSampleRate(),2000);
	}
	strAlgorithm = "Normal";
	prevSampleTime = 0;
	delta = 0;
}

void CircularRide::onSampleRateChange() {
	for (int i=0;i<4;i++) {
		audioDelay[i].reset(APP->engine->getSampleRate());
	}
}

template <typename T>
void CircularRide::processChannel(Input& inl, Input& inr, Output& outl, Output& outr) {
		
	// Get input
	int channels = std::max(inl.getChannels(), inr.getChannels());
	T xnL,xnR;
	T i[2];
    T o[2] = {0.0, 0.0};
	outl.setChannels(channels);
	outr.setChannels(channels);

	for (int c = 0; c < channels; c += 4) {
		if (outl.isConnected() || outr.isConnected()) {
			audioDelay[c/4].setParameters(adp);
			xnL = simd::float_4::load(inl.getVoltages(c));
    		xnR = simd::float_4::load(inr.getVoltages(c));
			i[0] = xnL;
			i[1] = xnR;
			audioDelay[c/4].processAudioFrame(i,o,2,2);

			o[0].store(outl.getVoltages(c));
			o[1].store(outr.getVoltages(c));
		}
	}
}

void CircularRide::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		if (static_cast<int>(adp.algorithm)+1 < static_cast<int>(delayAlgorithm::numDelayAlgorithms))
			adp.algorithm = static_cast<delayAlgorithm>(static_cast<int>(adp.algorithm) + 1);

	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		if (static_cast<int>(adp.algorithm) - 1 >= 0)
			adp.algorithm = static_cast<delayAlgorithm>(static_cast<int>(adp.algorithm) - 1);
	
	strAlgorithm = delayAlgorithmTxt[static_cast<int>(adp.algorithm)];
	
	if ((outputs[OUTPUT_LEFT].isConnected() || outputs[OUTPUT_RIGHT].isConnected()) && (inputs[INPUT_LEFT].isConnected() || inputs[INPUT_RIGHT].isConnected())) {
		adp.delayRatio_Pct = params[PARAM_RATIO].getValue();
		if (inputs[INPUT_RATIOCV].isConnected())
			adp.delayRatio_Pct = params[PARAM_RATIO].getValue()*(inputs[INPUT_RATIOCV].getVoltage()/10.f);
        adp.dryLevel_dB = params[PARAM_DRY].getValue();
		if (inputs[INPUT_DRYCV].isConnected())
			adp.feedback_Pct = params[PARAM_DRY].getValue()*(inputs[INPUT_DRYCV].getVoltage()/10.f);
		adp.feedback_Pct  = params[PARAM_FEEDBACK].getValue();
		if (inputs[INPUT_FBCV].isConnected())
			adp.feedback_Pct = params[PARAM_FEEDBACK].getValue()*(inputs[INPUT_FBCV].getVoltage()/10.f);
		if (inputs[INPUT_SYNC].isConnected()) {
			delta++;
			if (syncTrigger.process(rescale(inputs[INPUT_SYNC].getVoltage(), 1.f, 0.1f, 0.f, 1.f))) {
				delta = delta * args.sampleTime * 1000;
				adp.leftDelay_mSec=delta;
				adp.centreDelay_mSec=delta;
				adp.rightDelay_mSec=delta;
				delta=0;
			}
		}
		else {
			adp.leftDelay_mSec=params[PARAM_DELAYL].getValue();
			if (inputs[INPUT_DELAYLCV].isConnected())
				adp.leftDelay_mSec=params[PARAM_DELAYL].getValue()*(inputs[INPUT_DELAYLCV].getVoltage()/10.f);
			adp.centreDelay_mSec=params[PARAM_DELAYC].getValue();
			if (inputs[INPUT_DELAYCCV].isConnected())
				adp.centreDelay_mSec=params[PARAM_DELAYC].getValue()*(inputs[INPUT_DELAYCCV].getVoltage()/10.f);
			adp.rightDelay_mSec=params[PARAM_DELAYR].getValue();
			if (inputs[INPUT_DELAYRCV].isConnected())
				adp.rightDelay_mSec=params[PARAM_DELAYR].getValue()*(inputs[INPUT_DELAYRCV].getVoltage()/10.f);
		}
        adp.updateType = static_cast<delayUpdateType>(static_cast<int>(params[PARAM_TYPE].getValue()));
        adp.wetLevel_dB = params[PARAM_WET].getValue();
		if (inputs[INPUT_WETCV].isConnected())
			adp.wetLevel_dB = params[PARAM_WET].getValue()*(inputs[INPUT_WETCV].getVoltage()/10.f);
 		adp.lpfFc = params[PARAM_LPFFC].getValue();
		if (inputs[INPUT_LPFCV].isConnected())
			adp.lpfFc = params[PARAM_LPFFC].getValue()*(inputs[INPUT_LPFCV].getVoltage()/10.f); 
		adp.hpfFc = params[PARAM_HPFFC].getValue();
		if (inputs[INPUT_HPFCV].isConnected())
			adp.hpfFc = params[PARAM_HPFFC].getValue()*(inputs[INPUT_HPFCV].getVoltage()/10.f);		
		adp.useLPF = enableLPF;
		adp.useHPF = enableHPF;

		processChannel<rack::simd::float_4>(inputs[INPUT_LEFT], inputs[INPUT_RIGHT],outputs[OUTPUT_LEFT], outputs[OUTPUT_RIGHT] );
	}
}

struct CircularRideModuleWidget : ModuleWidget {
	CircularRideModuleWidget(CircularRide* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CircularRide.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        {
			AlgorithmDisplay * ad = new AlgorithmDisplay(Vec(76,20));
			ad->module = module;
			addChild(ad);
		}

		addInput(createInput<PJ301MPort>(Vec(11, 320), module, CircularRide::INPUT_LEFT));
        addInput(createInput<PJ301MPort>(Vec(11, 350), module, CircularRide::INPUT_RIGHT));
		addInput(createInput<PJ301MPort>(Vec(49, 320), module, CircularRide::INPUT_SYNC));
		addInput(createInput<PJ301MPort>(Vec(11, 105), module, CircularRide::INPUT_DELAYLCV));
		addInput(createInput<PJ301MPort>(Vec(69, 105), module, CircularRide::INPUT_DELAYCCV));
		addInput(createInput<PJ301MPort>(Vec(127, 105), module, CircularRide::INPUT_DELAYRCV));
		addInput(createInput<PJ301MPort>(Vec(11, 190), module, CircularRide::INPUT_RATIOCV));
		addInput(createInput<PJ301MPort>(Vec(127, 190), module, CircularRide::INPUT_FBCV));
		addInput(createInput<PJ301MPort>(Vec(11, 272), module, CircularRide::INPUT_LPFCV));
		addInput(createInput<PJ301MPort>(Vec(49, 272), module, CircularRide::INPUT_DRYCV));
		addInput(createInput<PJ301MPort>(Vec(89, 272), module, CircularRide::INPUT_WETCV));
		addInput(createInput<PJ301MPort>(Vec(127, 272), module, CircularRide::INPUT_HPFCV));

		addOutput(createOutput<PJ301MPort>(Vec(126, 320), module, CircularRide::OUTPUT_LEFT));
        addOutput(createOutput<PJ301MPort>(Vec(126, 350), module, CircularRide::OUTPUT_RIGHT));
		
        addParam(createParam<RPJKnob>(Vec(9,70),module, CircularRide::PARAM_DELAYL));
		addParam(createParam<RPJKnob>(Vec(67,70),module, CircularRide::PARAM_DELAYC));
		addParam(createParam<RPJKnob>(Vec(125,70),module, CircularRide::PARAM_DELAYR));
        addParam(createParam<RPJKnob>(Vec(125,155),module, CircularRide::PARAM_FEEDBACK));
        addParam(createParam<RPJKnob>(Vec(9,155),module, CircularRide::PARAM_RATIO));
        addParam(createParam<buttonMinSmall>(Vec(42,35),module, CircularRide::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(113,35),module, CircularRide::PARAM_UP));
        addParam(createParam<RPJKnob>(Vec(9,237),module, CircularRide::PARAM_LPFFC));
		addParam(createParam<RPJKnob>(Vec(47, 237), module, CircularRide::PARAM_DRY));
		addParam(createParam<RPJKnob>(Vec(87, 237), module, CircularRide::PARAM_WET));
		addParam(createParam<RPJKnob>(Vec(125,237),module, CircularRide::PARAM_HPFFC));
        addParam(createParam<Toggle2P>(Vec(65, 170), module, CircularRide::PARAM_TYPE));
	}

	void appendContextMenu(Menu *menu) override {
		CircularRide *module = dynamic_cast<CircularRide*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(rack::createBoolPtrMenuItem("Enable Low Pass Filter", "", &module->enableLPF));
		menu->addChild(rack::createBoolPtrMenuItem("Enable High Pass Filter", "", &module->enableHPF));
	}
};

std::string DetectAlgorithmQuantity::getDisplayValueString() {
	if ( module == NULL) 
		v = "";
	else switch (static_cast<int>(module->params[CircularRide::PARAM_ALGORITHM].getValue())) {
		case 0: 
			v = "Normal";
			break;
		case 1: 
			v = "PingPong";
			break;
		default:
			break;
	}
	return v;
}

AlgorithmDisplay::AlgorithmDisplay(Vec pos) {
	box.pos = pos;
	box.size.y = fh;
	box.size.x = fh;
	setColor(0xff, 0xff, 0xff, 0xff);
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
}

AlgorithmDisplay::AlgorithmDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	box.pos = pos;
	box.size.y = fh;
	box.size.x = fh;
	setColor(r, g, b, a);
	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
}

void AlgorithmDisplay::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	txtCol.r = r;
	txtCol.g = g;
	txtCol.b = b;
	txtCol.a = a;
}

void AlgorithmDisplay::drawLayer(const DrawArgs &args,int layer) {
	if (layer == 1) {
		char tbuf[11];

		if (module == NULL) return;

		std::snprintf(tbuf, sizeof(tbuf), "%s", &module->strAlgorithm[0]);
		
		TransparentWidget::draw(args);
		drawBackground(args);
		drawValue(args, tbuf);
	}
	TransparentWidget::drawLayer(args,layer);
}

void AlgorithmDisplay::drawBackground(const DrawArgs &args) {
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

void AlgorithmDisplay::drawValue(const DrawArgs &args, const char * txt) {
	Vec c = Vec(box.size.x/2, box.size.y);

	nvgFontSize(args.vg, fh);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, -2);
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
	nvgText(args.vg, c.x, c.y+fh-1, txt, NULL);
}

Toggle2P::Toggle2P() {
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SW_Toggle_0.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SW_Toggle_2.svg")));

	// no shadow for switches
	shadow->opacity = 0.0f;
	neg = pos = 0;
}

// handle the manually entered values
void Toggle2P::onChange(const event::Change &e) {

	SvgSwitch::onChange(e);

	if (getParamQuantity()->getValue() > 0.5f)
		getParamQuantity()->setValue(1.0f);
	else
		getParamQuantity()->setValue(0.0f);
}

json_t *CircularRide::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_DELAY_ALGORITHM_KEY, json_integer(static_cast<int>(adp.algorithm)));
	json_object_set_new(rootJ, JSON_ENABLE_LPF_KEY, json_boolean(enableLPF));
	json_object_set_new(rootJ, JSON_ENABLE_HPF_KEY, json_boolean(enableHPF));
	return rootJ;
}

void CircularRide::dataFromJson(json_t *rootJ) {
	json_t *nAlgorithmJ = json_object_get(rootJ, JSON_DELAY_ALGORITHM_KEY);
	json_t *nLPFJ = json_object_get(rootJ, JSON_ENABLE_LPF_KEY);
	json_t *nHPFJ = json_object_get(rootJ, JSON_ENABLE_HPF_KEY);
	if (nAlgorithmJ) {
		adp.algorithm=static_cast<delayAlgorithm>(json_integer_value(nAlgorithmJ));
	}
	if (nLPFJ) {
		enableLPF=json_boolean_value(nLPFJ);
	}
	if (nHPFJ) {
		enableHPF=json_boolean_value(nHPFJ);
	}
}

Model * modelCircularRide = createModel<CircularRide, CircularRideModuleWidget>("CircularRide");