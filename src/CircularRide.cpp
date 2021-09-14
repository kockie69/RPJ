#include "RPJ.hpp"
#include "CircularRide.hpp"


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
	audioDelay.reset(44100);
    audioDelay.createDelayBuffers(44100,2000);
}

void CircularRide::process(const ProcessArgs &args) {

	if (upTrigger.process(rescale(params[PARAM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		if (static_cast<int>(adp.algorithm)+1 < static_cast<int>(delayAlgorithm::numDelayAlgorithms))
			adp.algorithm = static_cast<delayAlgorithm>(static_cast<int>(adp.algorithm) + 1);

	if (downTrigger.process(rescale(params[PARAM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		if (static_cast<int>(adp.algorithm) - 1 >= 0)
			adp.algorithm = static_cast<delayAlgorithm>(static_cast<int>(adp.algorithm) - 1);
	
	adp.strAlgorithm = audioDelay.delayAlgorithmTxt[static_cast<int>(adp.algorithm)];
	audioDelay.setParameters(adp);
	
	if ((outputs[OUTPUT_LEFT].isConnected() || outputs[OUTPUT_RIGHT].isConnected()) && (inputs[INPUT_LEFT].isConnected() || inputs[INPUT_RIGHT].isConnected())) {
		adp.strAlgorithm = audioDelay.delayAlgorithmTxt[static_cast<int>(adp.algorithm)];
        adp.delayRatio_Pct = params[PARAM_RATIO].getValue();;
        adp.dryLevel_dB = params[PARAM_DRY].getValue();
		adp.feedback_Pct  = params[PARAM_FEEDBACK].getValue();
		adp.leftDelay_mSec=params[PARAM_DELAYL].getValue();
		adp.centreDelay_mSec=params[PARAM_DELAYC].getValue();
		adp.rightDelay_mSec=params[PARAM_DELAYR].getValue();
        adp.updateType = static_cast<delayUpdateType>(static_cast<int>(params[PARAM_TYPE].getValue()));
        adp.wetLevel_dB = params[PARAM_WET].getValue();
		adp.lpfFc = params[PARAM_LPFFC].getValue();
		adp.hpfFc = params[PARAM_HPFFC].getValue();
		adp.useLPF = enableLPF;
		adp.useHPF = enableHPF;
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
			RPJTextLabel * tl = new RPJTextLabel(Vec(21, 60));
			tl->setText("L");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(55, 60));
			tl->setText("C");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(90, 60));
			tl->setText("R");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(125, 60));
			tl->setText("DRY");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(42, 110),10);
			tl->setText("Delay");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(60, 120));
			tl->setText("FEEDBACK");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 120));
			tl->setText("RATIO");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(125, 120));
			tl->setText("WET");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(20,165),10);
			tl->setText("Left+Ratio");
			addChild(tl);
		}	
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(20, 200),10);
			tl->setText("Left&Right");
			addChild(tl);
		}
        {
			RPJTextLabel * tl = new RPJTextLabel(Vec(1, 205));
			tl->setText("LPF");
			addChild(tl);
		}
		{
			RPJTextLabel * tl = new RPJTextLabel(Vec(125, 205));
			tl->setText("HPF");
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
		
        addParam(createParam<RoundBlackKnob>(Vec(8,90),module, CircularRide::PARAM_DELAYL));
		addParam(createParam<RoundBlackKnob>(Vec(44,90),module, CircularRide::PARAM_DELAYC));
		addParam(createParam<RoundBlackKnob>(Vec(82,90),module, CircularRide::PARAM_DELAYR));
		addParam(createParam<RoundBlackKnob>(Vec(120, 90), module, CircularRide::PARAM_DRY));
        addParam(createParam<RoundBlackKnob>(Vec(82,150),module, CircularRide::PARAM_FEEDBACK));
        addParam(createParam<RoundBlackKnob>(Vec(8,150),module, CircularRide::PARAM_RATIO));
       	addParam(createParam<RoundBlackKnob>(Vec(120, 150), module, CircularRide::PARAM_WET));
        addParam(createParam<buttonMinSmall>(Vec(20,55),module, CircularRide::PARAM_DOWN));
		addParam(createParam<buttonPlusSmall>(Vec(91,55),module, CircularRide::PARAM_UP));
        addParam(createParam<RoundBlackKnob>(Vec(8,235),module, CircularRide::PARAM_LPFFC));
		addParam(createParam<RoundBlackKnob>(Vec(120,235),module, CircularRide::PARAM_HPFFC));
        addParam(createParam<Toggle2P>(Vec(42, 180), module, CircularRide::PARAM_TYPE)); 
	}

	void appendContextMenu(Menu *menu) override {
		CircularRide *module = dynamic_cast<CircularRide*>(this->module);

		menu->addChild(new MenuEntry);
		nLPFMenuItem *nLPFItem = new nLPFMenuItem();
		nLPFItem->text = "Enable Low Pass Filter";
		nLPFItem->module = module;
		nLPFItem->EnableLPF = !module->enableLPF;
		nLPFItem->rightText = CHECKMARK(!nLPFItem->EnableLPF);
		menu->addChild(nLPFItem);

		nHPFMenuItem *nHPFItem = new nHPFMenuItem();
		nHPFItem->text = "Enable High Pass Filter";
		nHPFItem->module = module;
		nHPFItem->EnableHPF = !module->enableHPF;
		nHPFItem->rightText = CHECKMARK(!nHPFItem->EnableHPF);
		menu->addChild(nHPFItem);
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

/* Context Menu Item for changing the Gain Compensation setting */
void nLPFMenuItem::onAction(const event::Action &e) {
	module->enableLPF=EnableLPF;
}

/* Context Menu Item for changing the Gain Compensation setting */
void nHPFMenuItem::onAction(const event::Action &e) {
	module->enableHPF=EnableHPF;
}

Model * modelCircularRide = createModel<CircularRide, CircularRideModuleWidget>("CircularRide");