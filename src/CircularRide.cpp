#include "RPJ.hpp"
#include "CircularRide.hpp"

CircularRide::CircularRide() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    
    configParam(PARAM_DELAY, 0.f, 2000.f,250.f, "Delay");
    configParam(PARAM_FEEDBACK, 0.f, 100.f,50.f, "Feedback");
    configParam(PARAM_RATIO, 0.f, 100.f,50.f, "Ratio");
    configParam(PARAM_DRY, -60.f, 12.f,-3.f, "Dry");
    configParam(PARAM_WET, -60.f, 12.f,-3.f, "Wet");
	configParam<DetectAlgorithmQuantity>(PARAM_ALGORITHM, 0.f, 1.f, 0.f, "Algorithm");
	configParam(PARAM_UP, 0.0, 1.0, 0.0);
	configParam(PARAM_DOWN, 0.0, 1.0, 0.0);
    configParam(PARAM_TYPE, 0.f, 1.f,0.f, "Delay Update Type");
	audioDelay.reset(44100);
    audioDelay.createDelayBuffers(44100,2000);
}

void CircularRide::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		adp.algorithm = delayAlgorithm::kPingPong;	
	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		adp.algorithm = delayAlgorithm::kNormal;
	adp.strAlgorithm = audioDelay.delayAlgorithmTxt[static_cast<int>(adp.algorithm)];
	audioDelay.setParameters(adp);
	
	if ((outputs[OUTPUT_LEFT].isConnected() || outputs[OUTPUT_RIGHT].isConnected()) && (inputs[INPUT_LEFT].isConnected() || inputs[INPUT_RIGHT].isConnected())) {
		adp.strAlgorithm = audioDelay.delayAlgorithmTxt[static_cast<int>(adp.algorithm)];
        adp.delayRatio_Pct = params[PARAM_RATIO].getValue();;
        adp.dryLevel_dB = params[PARAM_DRY].getValue();
		adp.feedback_Pct  = params[PARAM_FEEDBACK].getValue();
		adp.leftDelay_mSec=params[PARAM_DELAY].getValue();
		adp.rightDelay_mSec=params[PARAM_DELAY].getValue();
        adp.updateType = static_cast<delayUpdateType>(static_cast<int>(params[PARAM_TYPE].getValue()));
        adp.wetLevel_dB = params[PARAM_WET].getValue();
		audioDelay.setParameters(adp);

        float xnL = inputs[INPUT_LEFT].getVoltage();
        float xnR = inputs[INPUT_RIGHT].getVoltage();

        float i[2] = {xnL, xnR};
        float o[2] = {0.0, 0.0};

		audioDelay.processAudioFrame(i,o,2,2);

		outputs[OUTPUT_LEFT].setVoltage(o[0]);
        outputs[OUTPUT_RIGHT].setVoltage(o[1]);
	}
}

struct buttonPlusSmall : SvgSwitch  {
	buttonPlusSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonPlus_1.svg")));
	}
};

struct buttonMinSmall : SvgSwitch  {
	buttonMinSmall() {
		momentary=true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ButtonMin_1.svg")));
	}
};

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
			RPJTitle * title = new RPJTitle(box.size.x,MODULE_WIDTH);
			title->setText("CIRCULARRIDE");
			addChild(title);
		}
        {
			AlgorithmDisplay * ad = new AlgorithmDisplay(Vec(54,40));
			ad->module = module;
			addChild(ad);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 20),10,MODULE_WIDTH);
			tl->setText("Stereo Delay");
			addChild(tl);
		}	
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 60));
			tl->setText("DELAY");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(60, 60));
			tl->setText("FEEDBACK");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(40, 100));
			tl->setText("RATIO");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(20,160),10);
			tl->setText("Left+Ratio");
			addChild(tl);
		}	
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(20, 200),10);
			tl->setText("Left&Right");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(10, 200));
			tl->setText("DRY");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(85, 200));
			tl->setText("WET");
			addChild(tl);
		}		
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(13, 260));
			tl->setText("IN");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(82, 260));
			tl->setText("OUT");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 278));
			tl->setText("L");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 310));
			tl->setText("R");
			addChild(tl);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 290), module, CircularRide::INPUT_LEFT));
        addInput(createInput<PJ301MPort>(Vec(10, 320), module, CircularRide::INPUT_RIGHT));
		addOutput(createOutput<PJ301MPort>(Vec(82, 290), module, CircularRide::OUTPUT_LEFT));
        addOutput(createOutput<PJ301MPort>(Vec(82, 320), module, CircularRide::OUTPUT_RIGHT));
		
        addParam(createParam<RoundBlackKnob>(Vec(8,90),module, CircularRide::PARAM_DELAY));
        addParam(createParam<RoundBlackKnob>(Vec(82,90),module, CircularRide::PARAM_FEEDBACK));
        addParam(createParam<RoundBlackKnob>(Vec(45,130),module, CircularRide::PARAM_RATIO));

        addParam(createParam<buttonMinSmall>(Vec(20,55),module, CircularRide::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(91,55),module, CircularRide::PARAM_UP));
		addParam(createParam<RoundBlackKnob>(Vec(8, 230), module, CircularRide::PARAM_DRY));
       	addParam(createParam<RoundBlackKnob>(Vec(82, 230), module, CircularRide::PARAM_WET));
        addParam(createParam<Toggle2P>(Vec(42, 180), module, CircularRide::PARAM_TYPE)); 
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

void AlgorithmDisplay::draw(const DrawArgs &args) {
	char tbuf[11];

	if (module == NULL) return;

	std::snprintf(tbuf, sizeof(tbuf), "%s", &module->audioDelay.getParameters().strAlgorithm[0]);
		
	TransparentWidget::draw(args);
	drawBackground(args);
	drawValue(args, tbuf);
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

	if (paramQuantity->getValue() > 0.5f)
		paramQuantity->setValue(1.0f);
	else
		paramQuantity->setValue(0.0f);
}


// override the base randomizer as it sets switches to invalid values.
void Toggle2P::randomize() {
	SvgSwitch::randomize();

	if (paramQuantity->getValue() > 0.5f)
		paramQuantity->setValue(1.0f);
	else
		paramQuantity->setValue(0.0f);
}	

json_t *CircularRide::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_DELAY_ALGORITHM_KEY, json_integer(static_cast<int>(adp.algorithm)));
	return rootJ;
}

void CircularRide::dataFromJson(json_t *rootJ) {
	json_t *nAlgorithmJ = json_object_get(rootJ, JSON_DELAY_ALGORITHM_KEY);
	if (nAlgorithmJ) {
		adp.algorithm=static_cast<delayAlgorithm>(json_integer_value(nAlgorithmJ));
	}
}

Model * modelCircularRide = createModel<CircularRide, CircularRideModuleWidget>("CircularRide");