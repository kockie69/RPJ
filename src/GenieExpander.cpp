#include "RPJ.hpp"
#include "ctrl/RPJPorts.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "GenieExpander.hpp"

GenieExpander::GenieExpander() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_HISTORY, 1.f, 100.f,10.f, "Swarm length");
	configParam(PARAM_HISTORYTIMER,1.f, 10.f, 1.f, "Swarm thickness");
	configParam(PARAM_PEND_1_X,-INFINITY, INFINITY,WIDTH/4, "X-pos Pendulum 1");
	configParam(PARAM_PEND_1_Y,-INFINITY, INFINITY,HEIGHT/4, "Y-pos Pendulum 1");
	configParam(PARAM_PEND_2_X,-INFINITY, INFINITY,WIDTH/4, "X-pos Pendulum 2");
	configParam(PARAM_PEND_2_Y,-INFINITY, INFINITY,HEIGHT/4, "Y-pos Pendulum 2");
	configParam(PARAM_PEND_3_X,-INFINITY, INFINITY,WIDTH/4, "X-pos Pendulum 3");
	configParam(PARAM_PEND_3_Y,-INFINITY, INFINITY,HEIGHT/4, "Y-pos Pendulum 3");
	configParam(PARAM_PEND_4_X,-INFINITY, INFINITY,WIDTH/4, "X-pos Pendulum 4");
	configParam(PARAM_PEND_4_Y,-INFINITY, INFINITY,HEIGHT/4, "Y-pos Pendulum 4");
	rdMsg = new XpanderPairs();
}

json_t *GenieExpander::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, "JSON_DRAWLINES", json_boolean(static_cast<bool>(drawLines)));
    return rootJ;
}

void GenieExpander::dataFromJson(json_t *rootJ) {
	json_t *nDrawLinesJ = json_object_get(rootJ, "JSON_DRAWLINES");
	if (nDrawLinesJ) 
		drawLines = static_cast<bool>(json_boolean_value(nDrawLinesJ));
}

void GenieDisplay::onDragStart(const DragStartEvent& e) {
	TransparentWidget::onDragStart(e);
}

void GenieDisplay::drawLayer(const DrawArgs &args,int layer) {

	if (module)
        if (layer == 1) {
            if (module->rdMsg) {
                for (int n=0;n < 4;n++) {
                    Mass *newMass = new Mass();
					Joint *joint1 = new Joint();
					Joint *joint2 = new Joint();
                    newMass->setPosition({module->X[n],module->Y[n]});
					newMass->setColor(nvgRGB(0xAE, 0x1C, 0x28));
                    newMass->history = module->params[module->PARAM_HISTORY].getValue();
					newMass->setWeight(module->rdMsg->weight);
                    addChild(newMass);
					joint1->setBegin({newMass->box.pos.x+newMass->getPosition().x,newMass->box.pos.y+newMass->getPosition().y});
                    for (int i=0; i < 2;i++) {
                        Mass *newMass = new Mass();
                        newMass->setPosition({module->X[n]+module->rdMsg->edges[n][i].first*10,module->Y[n]+module->rdMsg->edges[n][i].second*10});
                        // If the node is below the root
						if (i==0) {
                            newMass->setColor(nvgRGB(0xFF, 0xFF, 0xFF));
							joint1->setEnd({newMass->box.pos.x+newMass->getPosition().x,newMass->box.pos.y+newMass->getPosition().y});
							joint2->setBegin({newMass->box.pos.x+newMass->getPosition().x,newMass->box.pos.y+newMass->getPosition().y});
							joint1->setWeight(module->rdMsg->weight);
							addChild(joint1);
						}
						// two levels below the root
                        if (i==1) {
                            newMass->setColor(nvgRGB(0x21, 0x46, 0x8B));
							joint2->setEnd({newMass->box.pos.x+newMass->getPosition().x,newMass->box.pos.y+newMass->getPosition().y});
							joint2->setWeight(module->rdMsg->weight);
							addChild(joint2);
						}
						newMass->history = module->params[module->PARAM_HISTORY].getValue();
                        newMass->setWeight(module->rdMsg->weight);
                        addChild(newMass);
                    }
                }
            }
        }
	// Remove everything outside display
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
	TransparentWidget::drawLayer(args,layer);
}

Joint::Joint() {
	elapsed=0;
}

void Joint::setWeight(float t) {
	thick = t;
}

void Joint::setEnd(Vec e) {
	positionEnd = e;
}

void Joint::setBegin(Vec b) {
	positionBegin = b;
}

void Joint::drawLayer(const DrawArgs &args,int layer) {
	if (layer == 1) {
		if (elapsed!=1) {
			NVGcolor lineColor = nvgRGBA(0xFF, 0x7F, 0x00, 0xA0);
			nvgFillColor(args.vg, lineColor);
			nvgStrokeColor(args.vg, lineColor);
			nvgStrokeWidth(args.vg, thick);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg,positionBegin.x,positionBegin.y);
			nvgLineTo(args.vg,positionEnd.x,positionEnd.y);
			nvgStroke(args.vg);
			nvgClosePath(args.vg);
			elapsed++;
		}
		else
			requestDelete();
	}
	Widget::drawLayer(args,layer);
}

void Mass::setWeight(float w) {
    weight = w;
}

void Mass::drawLayer(const DrawArgs &args,int layer) {
    if (layer==1) {
        NVGcolor massColorFaded = nvgTransRGBA(massColor, (history-elapsed)*200/history);
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, getPosition().x, getPosition().y, weight);
        nvgFillPaint(args.vg,nvgRadialGradient(args.vg,getPosition().x, getPosition().y, 1, weight, massColorFaded ,massColor));
        nvgFill(args.vg);
        nvgClosePath(args.vg);
        elapsed++;
        if (elapsed == history)
            requestDelete();
    }
	Widget::drawLayer(args,layer);
}

void Mass::setColor(NVGcolor massColor) {
	this->massColor = massColor;
}


void GenieExpander::process(const ProcessArgs &args) {
	
	for (int i=0;i<4;i++) {
		float delta1X = params[PARAM_PEND_1_X+(2*i)].getValue() - prevX[i];
		X[i] += delta1X*(WIDTH/2);
		X[i] = std::max(X[i],0.f);
		X[i] = std::min(X[i],float(WIDTH/2));
		prevX[i] = params[PARAM_PEND_1_X+(2*i)].getValue();

		float delta1Y = params[PARAM_PEND_1_Y+(2*i)].getValue() - prevY[i];
		Y[i] += delta1Y*(HEIGHT/2);
		Y[i] = std::max(Y[i],0.f);
		Y[i] = std::min(Y[i],float(HEIGHT/2));
		prevY[i] = params[PARAM_PEND_1_Y+(2*i)].getValue();
	}

	maxHistory = params[PARAM_HISTORY].getValue();

	swarmThickness = params[PARAM_HISTORYTIMER].getValue();
	historyTimer.setDivision(swarmThickness);

	bool parentConnected = leftExpander.module && leftExpander.module->model == modelGenie;
	if (parentConnected)
		getPendulums();
}

void GenieExpander::getPendulums() {
    rdMsg = (XpanderPairs*)leftExpander.module->rightExpander.consumerMessage;
}

void GenieExpanderModuleWidget::appendContextMenu(Menu *menu) {
	GenieExpander * module = dynamic_cast<GenieExpander*>(this->module);

//	menu->addChild(new MenuSeparator());
//	menu->addChild(createIndexPtrSubmenuItem("Number of Pendulums", {"1", "2", "3", "4"}, &module->nrOfPendulums));

	menu->addChild(new MenuSeparator());

	menu->addChild(createBoolPtrMenuItem("Draw Lines between Nodes","", &module->drawLines));
}

GenieExpanderModuleWidget::GenieExpanderModuleWidget(GenieExpander* module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GenieExpander.svg")));

	addChild(createWidget<ScrewSilver>(Vec(0, 0)));
	//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15,0)));
	//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(0, 365)));

	if (module) {
        display = new GenieDisplay();
        display->box.pos = Vec(84, 6);
        display->box.size = Vec(WIDTH, HEIGHT);
        display->module = module;
        addChild(display);	
    }
		// Next do the Jacks
	const float jackX1 = 14.5;
	const float jackX2 = 51;		

	const float jackY1 = 242;
	const float jackY2 = 272;
	const float jackY3 = 302;
	const float jackY4 = 331;


	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY1), module, GenieExpander::INPUT_1_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY1), module, GenieExpander::INPUT_1_Y));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY2), module, GenieExpander::INPUT_2_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY2), module, GenieExpander::INPUT_2_Y));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY3), module, GenieExpander::INPUT_3_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY3), module, GenieExpander::INPUT_3_Y));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX1, jackY4), module, GenieExpander::INPUT_4_X));
	addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY4), module, GenieExpander::INPUT_4_Y));

		// Then do the knobs
	const float knobX1 = 16;
	const float knobX2 = 32;
	const float knobX3 = 52;

	const float knobY1 = 33;
	const float knobY2 = 80;
	const float knobY3 = 122;
	const float knobY4 = 150;
	const float knobY5 = 178;
	const float knobY6 = 206;

	addParam(createParam<RPJKnob>(Vec(knobX2, knobY1), module, GenieExpander::PARAM_HISTORY));
	addParam(createParam<RPJKnob>(Vec(knobX2, knobY2), module, GenieExpander::PARAM_HISTORYTIMER));
	addParam(createParam<RPJKnobSmall>(Vec(knobX1, knobY3), module, GenieExpander::PARAM_PEND_1_X));
	addParam(createParam<RPJKnobSmall>(Vec(knobX3, knobY3), module, GenieExpander::PARAM_PEND_1_Y));
	addParam(createParam<RPJKnobSmall>(Vec(knobX1, knobY4), module, GenieExpander::PARAM_PEND_2_X));
	addParam(createParam<RPJKnobSmall>(Vec(knobX3, knobY4), module, GenieExpander::PARAM_PEND_2_Y));
	addParam(createParam<RPJKnobSmall>(Vec(knobX1, knobY5), module, GenieExpander::PARAM_PEND_3_X));
	addParam(createParam<RPJKnobSmall>(Vec(knobX3, knobY5), module, GenieExpander::PARAM_PEND_3_Y));
	addParam(createParam<RPJKnobSmall>(Vec(knobX1, knobY6), module, GenieExpander::PARAM_PEND_4_X));
	addParam(createParam<RPJKnobSmall>(Vec(knobX3, knobY6), module, GenieExpander::PARAM_PEND_4_Y));
}

Model * modelGenieExpander = createModel<GenieExpander, GenieExpanderModuleWidget>("GenieExpander");