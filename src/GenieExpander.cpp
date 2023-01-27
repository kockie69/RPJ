#include "RPJ.hpp"
#include <random>
#include "ctrl/RPJPorts.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "GenieExpander.hpp"

GenieExpander::GenieExpander() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_HISTORY, 1.f, 100.f,10.f, "Swarm length");
	configParam(PARAM_HISTORYTIMER,5.f, 1.f, 1.f, "Swarm thickness");
	configParam(PARAM_PEND_1_X,-INFINITY, INFINITY,0, "X-pos Pendulum 1");
	configParam(PARAM_PEND_1_Y,-INFINITY, INFINITY,0, "Y-pos Pendulum 1");
	configParam(PARAM_PEND_2_X,-INFINITY, INFINITY,0, "X-pos Pendulum 2");
	configParam(PARAM_PEND_2_Y,-INFINITY, INFINITY,0, "Y-pos Pendulum 2");
	configParam(PARAM_PEND_3_X,-INFINITY, INFINITY,0, "X-pos Pendulum 3");
	configParam(PARAM_PEND_3_Y,-INFINITY, INFINITY,0, "Y-pos Pendulum 3");
	configParam(PARAM_PEND_4_X,-INFINITY, INFINITY,0, "X-pos Pendulum 4");
	configParam(PARAM_PEND_4_Y,-INFINITY, INFINITY,0, "Y-pos Pendulum 4");
	rdMsg = new XpanderPairs();
	nrOfNodes = 2;
	for (int i=0;i<4;i++) {
		XY[i] = {WIDTH/2,HEIGHT/2};
		prevXY[i] = {0,0};
	}
	weight=5;
	nrOfPendulums=4;
	std::mt19937 generator((std::random_device())());
	std::uniform_real_distribution<> rnd(0, 255);
	for (int i=0;i<=4;i++)
		for (int j=0;j<3;j++) {
			colors[i][j]=rnd(generator);
			jointColor[j]=rnd(generator);
		}
}

json_t *GenieExpander::dataToJson() {
	json_t *rootJ=json_object();
	json_t *cJ = json_array();
	for (int i=0;i<4;i++)
		for (int j=0;j<3;j++)
			json_array_insert_new(cJ, (i*3) + j, json_real(colors[i][j]));
	json_object_set_new(rootJ, "JSON_COLORS", cJ);

	json_t *jcJ = json_array();
		for (int i = 0; i < 3; i++)
			json_array_insert_new(jcJ, i, json_real(jointColor[i]));
	json_object_set_new(rootJ, "JSON_JOINTCOLOR", jcJ);

	json_object_set_new(rootJ, "JSON_DRAWLINES", json_boolean(static_cast<bool>(drawLines)));


	json_t *rootsJ = json_array();
		for (int i = 0; i < 4; i++) {
			json_array_insert_new(rootsJ, (2*i), json_real(XY[i].x));
			json_array_insert_new(rootsJ, (2*i)+1, json_real(XY[i].y));
		}
	json_object_set_new(rootJ, "JSON_ROOTS", rootsJ);
	return rootJ;
}

void GenieExpander::dataFromJson(json_t *rootJ) {

	json_t *cJ = json_object_get(rootJ, "JSON_COLORS");
	if (cJ) {
		for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 3; j++)
				{
					json_t *cArray1J = json_array_get(cJ, (i*3) + j);
					if (cArray1J)
						colors[i][j] = json_real_value(cArray1J);
				}	
		}		
	}

	json_t *jcJ = json_object_get(rootJ, "JSON_JOINTCOLOR");
	if (jcJ) {
		for (int i = 0; i < 3; i++)
			{
				json_t *jcArrayJ = json_array_get(jcJ, i);
				if (jcArrayJ)
					jointColor[i] = json_real_value(jcArrayJ);
			}			
	}
	
	json_t *rootsJ = json_object_get(rootJ, "JSON_ROOTS");
	if (rootsJ) {
		for (int i = 0; i < 4; i++)
			{
				json_t *rootsArrayJ = json_array_get(rootsJ, (2*i));
				if (rootsArrayJ) 
					XY[i].x = json_real_value(rootsArrayJ);
				rootsArrayJ = json_array_get(rootsJ, (2*i)+1);
				if (rootsArrayJ) 
					XY[i].y = json_real_value(rootsArrayJ);
			}			
	}

	json_t *nDrawLinesJ = json_object_get(rootJ, "JSON_DRAWLINES");

	if (nDrawLinesJ) 
		drawLines = static_cast<bool>(json_boolean_value(nDrawLinesJ));
}

void GenieDisplay::onDragHover(const DragHoverEvent& e) {
	OpaqueWidget::onDragHover(e);
	if (e.isConsumed())
		return;
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		e.consume(this);
	}	
}


GenieDisplay::GenieDisplay() {
}

void GenieDisplay::step() {
	bool timer;
	if (module) {
		Mass *newMass = new Mass(module,0,0);
		for (int n=0;n < module->nrOfPendulums;n++) {
			Root *root = new Root(module,n);
			roots[n]=root;
			root->rootPos=&module->XY[n];
			Joint *joint1 = new Joint(module);

			joint1->setBegin({7+root->getPosition().x,7+root->getPosition().y});

			// Add a root
			addChild(root);

			if (n==0)
				timer = module->historyTimer.process();
			if (timer) {
				for (int i=0; i < module->nrOfNodes;i++) {
					newMass = new Mass(module,n,i);
					Joint *joint2 = new Joint(module);
					// If the node is below the root
					joint1->setEnd({newMass->getPosition().x,newMass->getPosition().y});
					addChild(joint1);

					joint2->setBegin({newMass->getPosition().x,newMass->getPosition().y});
					joint1=joint2;
					addChild(newMass);
				}
				joint1->setEnd({newMass->getPosition().x,newMass->getPosition().y});
				addChild(joint1);
			}
		}
	}
	OpaqueWidget::step();
}


void GenieDisplay::drawLayer(const DrawArgs &args,int layer) {

    if (layer == 1) {
		// Remove everything outside display
		nvgScissor(args.vg, RECT_ARGS(args.clipBox));
		OpaqueWidget::drawLayer(args,layer);
	}
}

Joint::Joint(GenieExpander *m) {
	module=m;
	setPosition({0,0});
	thick=module->weight;
	elapsed=0;
}

void Joint::setEnd(Vec e) {
	positionEnd = e;
}

void Joint::setBegin(Vec b) {
	positionBegin = b;
}

void Joint::drawLayer(const DrawArgs &args,int layer) {
	if (layer == 1) {
		if (module) {
			if (module->drawLines) {
				if (elapsed!=1) {
					NVGcolor lineColor = nvgRGB(module->jointColor[0], module->jointColor[1],module->jointColor[2]);
					nvgFillColor(args.vg, lineColor);
					nvgStrokeColor(args.vg, lineColor);
					nvgStrokeWidth(args.vg, thick);
					nvgLineCap(args.vg, NVG_ROUND);
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
		}
	}
	Widget::drawLayer(args,layer);
}

Root::Root(GenieExpander *m,int p) {
	setPosition(m->XY[p]);
	history = m->params[m->PARAM_HISTORY].getValue();
	node = 0;
	weight = m->weight;
	elapsed=0;
	setColor(nvgRGB(m->colors[0][0],m->colors[0][1],m->colors[0][2]));
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/ButtonLarge_1.svg")));
}


void Root::step() {
	if (elapsed==1)
		requestDelete();
	SvgWidget::step();
}

void Root::setColor(NVGcolor massColor) {
	this->massColor = massColor;
}

void Root::drawLayer(const DrawArgs &args,int layer) {
	if (layer==1) {
		NVGcolor transColor = nvgLerpRGBA(nvgRGB(0xFF, 0xFF, 0xFF), massColor, 0.75);
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, 7,7, weight);
		nvgFillPaint(args.vg,nvgRadialGradient(args.vg,0,0, 1, weight, transColor ,massColor));
		nvgFill(args.vg);
		nvgClosePath(args.vg);
		elapsed++;
		}
	SvgWidget::drawLayer(args,layer);
}

void Root::onDragHover(const DragHoverEvent &e) {
	if (!e.isConsumed()) {
		if (node==0) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT ) {
				e.consume(this);
				float x = getPosition().x+e.pos.x+e.mouseDelta.x-7;
				float y = getPosition().y+e.pos.y+e.mouseDelta.y-7;
				setPosition({x,y});
				*(rootPos)=getPosition();
			}
		}
	}
}

Mass::Mass(GenieExpander *m,int p,int n) {
	setPosition({m->XY[p].x+m->edges[p][n].first*10,m->XY[p].y+m->edges[p][n].second*10});
	elapsed=0;
	setColor(nvgRGB(m->colors[n+1][0],m->colors[n+1][1],m->colors[n+1][2]));
	history = m->params[m->PARAM_HISTORY].getValue();
	node=(n+1);
	weight = m->weight;
}

void Mass::step() {
	elapsed++;
	if (elapsed == history)
		requestDelete();
	//Widget::step();
}

void Mass::drawLayer(const DrawArgs &args,int layer) {
	NVGcolor transColor;
	if (layer==1) {
		NVGcolor massColorFaded = nvgTransRGBA(massColor, (history-elapsed)*255/history);
		if (elapsed==1)
			transColor = nvgLerpRGBA(nvgRGB(0xFF, 0xFF, 0xFF), massColor, 0.75);
		else
			transColor = nvgLerpRGBA(nvgRGB(0x00, 0x00, 0x00), massColor, 0.25);
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, 0,0, weight);
		nvgFillPaint(args.vg,nvgRadialGradient(args.vg,0,0, 1, weight, transColor ,massColorFaded));
		nvgFill(args.vg);
		nvgClosePath(args.vg);
	}
	Widget::drawLayer(args,layer);
}

void Mass::setColor(NVGcolor massColor) {
	this->massColor = massColor;
}

void Mass::onDragEnd(const DragEndEvent &e) {
	if (!e.isConsumed()) {
		if (node==0) {
			e.consume(this);
			setPosition(APP->scene->getMousePos());
		}
	}
}

void GenieExpander::process(const ProcessArgs &args) {
	
	// Firs check if the positions are changed by the knobs
	for (int i=0;i<4;i++) {
		
		float deltaX = params[PARAM_PEND_1_X+(2*i)].getValue() - prevXY[i].x;	
		XY[i].x += deltaX*(WIDTH/2);
		XY[i].x = std::max(XY[i].x,0.f);
		XY[i].x = std::min(XY[i].x,float(WIDTH));
		prevXY[i].x = params[PARAM_PEND_1_X+(2*i)].getValue();

		float deltaY = params[PARAM_PEND_1_Y+(2*i)].getValue() - prevXY[i].y;
		XY[i].y += deltaY*(HEIGHT/2);
		XY[i].y = std::max(XY[i].y,0.f);
		XY[i].y = std::min(XY[i].y,float(HEIGHT));
		prevXY[i].y = params[PARAM_PEND_1_Y+(2*i)].getValue();
	}

	maxHistory = params[PARAM_HISTORY].getValue();

	swarmThickness = params[PARAM_HISTORYTIMER].getValue();
	historyTimer.setDivision(swarmThickness);

	nrOfPendulums=getPendulums();
}

int GenieExpander::getPendulums() {
	int pendulums=0;
	parentConnected = leftExpander.module && leftExpander.module->model == modelGenie;

	if (parentConnected) {
		if (rdMsg!=NULL) {
	    	rdMsg = (XpanderPairs*)leftExpander.module->rightExpander.consumerMessage;
			pendulums = rdMsg->nrOfPendulums;
			if (pendulums<=4) {
				weight = rdMsg->weight;
				nrOfNodes = 2;
			}
			else {
				pendulums =0;
			}
		}
	}
	else {
		// There is no genie source connected, maybe some other source
		// Read the inputs
		pendulums=1;
		weight=5;
	}
	for (int i=0;i<pendulums;i++) {
		if (parentConnected) {
			for (int n=0;n<4;n++)	
				edges[i][n]=rdMsg->edges[i][n];
		}
		else {
			nrOfNodes=0;
			for (int n=0;n<4;n++)
				if (inputs[INPUT_1_X+2*n].isConnected() && inputs[INPUT_1_Y+2*n].isConnected()) {
					nrOfNodes++;
					edges[i][n]={inputs[INPUT_1_X+2*n].getVoltage()*(n+1),-inputs[INPUT_1_Y+2*n].getVoltage()*(n+1)};
				}
				else {
					if (n==0)
						return(0);
					else
						return(1);
				}
		}
	}
	return(pendulums);
}


colorMenuSlider::colorMenuSlider(GenieExpander* m, const char* label, int n) : _module(m) {
	node = n;
	this->text = label;
	this->rightText = "▸";
}

colorMenuSlider::colorMenuSlider(GenieExpander* m, const char* label) : _module(m) {
	node = -1;
	this->text = label;
	this->rightText = "▸";
}

Menu* colorMenuSlider::createChildMenu() {
	Menu* menu = new Menu;

	menu->addChild(new ColorSlider(_module,node,0));
	menu->addChild(new ColorSlider(_module,node,1));
	menu->addChild(new ColorSlider(_module,node,2));
	
	return menu;
}

ColorSlider::ColorSlider(GenieExpander* module,int n,int rgb) {
	quantity = new ColorQuantity(module,n,rgb);
	box.size.x = 200.0f;
	colorPos=rgb+1;
}

void ColorSlider::draw(const DrawArgs &args) {
	ui::Slider::draw(args);
	nvgBeginPath(args.vg);
	nvgRect(args.vg,box.pos.x, 0, box.pos.x+box.size.x, box.size.y);
	switch (colorPos) {
		case 1:
			sliderColor = nvgRGBA(int(quantity->getValue()),0, 0, 160);
			break;
		case 2:
			sliderColor = nvgRGBA(0,int(quantity->getValue()), 0, 160);
			break;
		case 3:
			sliderColor = nvgRGBA(0,0,int(quantity->getValue()), 160);
			break;
		default:
			return;
	}
	nvgFillColor(args.vg,sliderColor);
	nvgFill(args.vg);
	nvgClosePath(args.vg);
}

ColorSlider:: ~ColorSlider() {
	delete quantity;
}

ColorQuantity::ColorQuantity(GenieExpander* m,int n, int rgb) : _module(m) {
	node=n;
	_rgb=rgb;
}

void ColorQuantity::setValue(float value) {
	value = clamp(value, getMinValue(), getMaxValue());
	if (_module) {
		if (node>=0)
			_module->colors[node][_rgb] = value;
		else
			_module->jointColor[_rgb] = value;
	}
}

float ColorQuantity::getValue() {
	if (_module) {
		if (node>=0)	
			return (node >=0) ? _module->colors[node][_rgb] : _module->jointColor[_rgb];
		else
			return _module->jointColor[_rgb];
	}
	return getDefaultValue();
}

std::string ColorQuantity::getLabel() { 
	switch (_rgb) { 
		case 0:
			return "Red Color";
		case 1:
			return "Green Color";
		case 2:
			return "Blue Color";
		default:
			return "Color Undefined"; 
	}
}
	
void GenieExpanderModuleWidget::appendContextMenu(Menu *menu) {
	GenieExpander * module = dynamic_cast<GenieExpander*>(this->module);

	menu->addChild(new MenuSeparator());
	menu->addChild(createBoolPtrMenuItem("Draw Lines between Nodes","", &module->drawLines));
	menu->addChild(new MenuSeparator());
	menu->addChild(new colorMenuSlider(module, "Color Root",0));
	menu->addChild(new colorMenuSlider(module, "Color Node 1",1));
	menu->addChild(new colorMenuSlider(module, "Color Node 2",2));
	menu->addChild(new colorMenuSlider(module, "Color Node 3",3));
	menu->addChild(new colorMenuSlider(module, "Color Node 4",4));
	menu->addChild(new colorMenuSlider(module, "Color Lines"));
}

void GenieExpanderModuleWidget::onDragHover(const DragHoverEvent& e) {
	ModuleWidget::onDragHover(e);
	if (!e.isConsumed())
		e.consume(this);
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