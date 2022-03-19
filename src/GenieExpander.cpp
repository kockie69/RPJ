#include "RPJ.hpp"
#include "ctrl/RPJPorts.hpp"
#include "ctrl/RPJKnobs.hpp"
#include <random>
#include "GenieExpander.hpp"

#define DEBUG_ONLY(x)

GenieExpander::GenieExpander() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_HISTORY, 1.f, 100.f,10.f, "Swarm length");
	configParam(PARAM_HISTORYTIMER,1.f, 10.f, 1.f, "Swarm thickness");
	configParam(PARAM_PEND_1_X,-((WIDTH/2)+75), (WIDTH/2)+75, 0, "X-pos Pendulum 1");
	configParam(PARAM_PEND_1_Y,-(HEIGHT/2), (HEIGHT/2), 0, "Y-pos Pendulum 1");
	configParam(PARAM_PEND_2_X,-((WIDTH/2)+75), (WIDTH/2)+75, 0, "X-pos Pendulum 2");
	configParam(PARAM_PEND_2_Y,-(HEIGHT/2), (HEIGHT/2), 0, "Y-pos Pendulum 2");
	configParam(PARAM_PEND_3_X,-((WIDTH/2)+75), (WIDTH/2)+75, 0, "X-pos Pendulum 3");
	configParam(PARAM_PEND_3_Y,-(HEIGHT/2), (HEIGHT/2), 0, "Y-pos Pendulum 3");
	configParam(PARAM_PEND_4_X,-((WIDTH/2)+75), (WIDTH/2)+75, 0, "X-pos Pendulum 4");
	configParam(PARAM_PEND_4_Y,-(HEIGHT/2), (HEIGHT/2), 0, "Y-pos Pendulum 4");
	maxHistory=10;
	for (int i=0;i<MAXPENDULUMS;i++) { 
		pendulums[i].setPosition({(WIDTH/2)+75,HEIGHT/2});
		pendulums[i].nodes[0].setColor(nvgRGB(0xAE, 0x1C, 0x28));	
		pendulums[i].nodes[1].setColor(nvgRGB(0xFF, 0xFF, 0xFF));
		pendulums[i].nodes[2].setColor(nvgRGB(0x21, 0x46, 0x8B));
		pendulums[i].nodes[3].setColor(nvgRGB(0xFF, 0x7F, 0x00));
		pendulums[i].nodes[4].setColor(nvgRGB(0xAE, 0x1C, 0x28));
		pendulums[i].nodes[0].setNodeType(node::STATIC);
		pendulums[i].nodes[1].setNodeType(node::ROTATING);
		pendulums[i].nodes[2].setNodeType(node::ROTATING);
		pendulums[i].nodes[3].setNodeType(node::ROTATING);
		pendulums[i].nodes[4].setNodeType(node::ROTATING);
	}
}

void GenieExpander::process(const ProcessArgs &args) {

	maxHistory = params[PARAM_HISTORY].getValue();
	// In future read the pendulum position parameters, for now all xpos,ypos
	for (int i=0;i<MAXPENDULUMS;i++) { 
		pendulums[i].nodes[0].setMaxhistory(maxHistory);
		pendulums[i].nodes[1].setMaxhistory(maxHistory);
		pendulums[i].nodes[2].setMaxhistory(maxHistory);
		pendulums[i].nodes[3].setMaxhistory(maxHistory);
		pendulums[i].nodes[4].setMaxhistory(maxHistory);
	}

	swarmThickness = params[PARAM_HISTORYTIMER].getValue();
	
	parentConnected = leftExpander.module && leftExpander.module->model == modelGenie;
	if (parentConnected) {
    	xpanderPairs* rdMsg = (xpanderPairs*)leftExpander.module->rightExpander.consumerMessage;
		for (int n=0;n < MAXPENDULUMS;n++) {
			pendulums[n].nodes[0].newMass.setPosition({params[PARAM_PEND_1_X+2*n].getValue()+pendulums[n].getPosition().first,params[PARAM_PEND_1_Y+n*2].getValue()+pendulums[n].getPosition().second});
			pendulums[n].nodes[0].newMass.setSize(10);
			pendulums[n].setNrOfNodes(3);


			for (int i=1; i < 3;i++) {
				pendulums[n].nodes[i].newMass.setPosition({rdMsg->edges[n][i-1].first*10+params[PARAM_PEND_1_X+2*n].getValue()+pendulums[n].getPosition().first,rdMsg->edges[n][i-1].second*10+params[PARAM_PEND_1_Y+2*n].getValue()+pendulums[n].getPosition().second});
				pendulums[n].nodes[i].newMass.setSize(10);
			}
		}
		nrOfPendulums = rdMsg->nrOfPendulums;
	}
	else {
		nrOfPendulums=1;
		pendulums[0].nodes[0].newMass.setPosition({params[PARAM_PEND_1_X].getValue()+pendulums[0].getPosition().first,params[PARAM_PEND_1_Y].getValue()+pendulums[0].getPosition().second});
		pendulums[0].nodes[0].newMass.setSize(10);
		pendulums[0].setNrOfNodes(5);
		for (int i=1;i<5;i++) {
			std::pair<double,double> newPair;
			if (inputs[INPUT_1_X+2*(i-1)].isConnected() && inputs[INPUT_1_Y+2*(i-1)].isConnected()) {
				newPair.first = params[PARAM_PEND_1_X].getValue()+inputs[INPUT_1_X+2*(i-1)].getVoltage()*10*i;
				newPair.second = params[PARAM_PEND_1_Y].getValue()+inputs[INPUT_1_Y+2*(i-1)].getVoltage()*10*i;
				pendulums[0].nodes[i].newMass.setPosition({newPair+pendulums[0].getPosition()});
				pendulums[0].nodes[i].newMass.setSize(10);
			}
			else {
				pendulums[0].setNrOfNodes(i);
				return;
			}
		}
	}
}

void node::setMaxhistory(int maxHistory) {
	this->maxHistory = maxHistory;
}

void node::setColor(NVGcolor nodeColor) {
	this->nodeColor = nodeColor;
}

void node::setNodeType(NodeType nodeType) {
	this->nodeType = nodeType;
}

node::node() {
	thickness=1;
}

void node::draw(NVGcontext *vg) {
	newMass.setColor(nodeColor);
	newMass.draw(vg);
	nodeSwarm.draw(vg,nodeColor);
	if (nodeType==ROTATING) {
		if (thickness!=swarmThickness) 
			historyTimer.setDivision(swarmThickness);
		if (historyTimer.process()) 
			nodeSwarm.update(newMass,maxHistory);
	}
}

swarm::swarm() {
	history=0;
}

void swarm::addNewestMass(mass newMass) {
	masses.insert(masses.begin(),newMass);
	history++;
}

void swarm::deleteOldestMass() {
	masses.pop_back();
	history--;
}

void swarm::update(mass newMass, int maxHistory) {
	if (maxHistory < history) {
		this->addNewestMass(newMass);
		this->deleteOldestMass();
		this->deleteOldestMass();
	}
	else {
		if (maxHistory > history)
			this->addNewestMass(newMass);
		else {
			this->addNewestMass(newMass);
			this->deleteOldestMass();
		}
	}
}

void swarm::draw(NVGcontext *vg,NVGcolor massColor) {

	for (int i=1;i<history;i++) {
		// The 50 in the next line can be a parameter		
		NVGcolor massColorFaded = nvgTransRGBA(massColor, (history-i)*200/history);
		masses[i].setColor(massColorFaded);
		masses[i].draw(vg);
	}
}

void line::setBegin(std::pair<double,double> positionBegin) {
	this->positionBegin = positionBegin;
}

void line::setEnd(std::pair<double,double> positionEnd) {
	this->positionEnd = positionEnd;
}

void mass::setSize(int size) {
	this->size = size;
}

void mass::setPosition(std::pair<double,double> position) {
	this->position = position;
}

std::pair<double,double> mass::getPosition(void) {
	return position;
}

void mass::setColor(NVGcolor massColor) {
	this->massColor = massColor;
}

void mass::draw(NVGcontext *vg) {
	nvgFillColor(vg, massColor);
	NVGcolor transColor = nvgLerpRGBA(nvgRGB(0xFF, 0xFF, 0xFF), massColor, 0.25);
	nvgBeginPath(vg);
	nvgCircle(vg, position.first, position.second, size);
	nvgFillPaint(vg,nvgRadialGradient(vg,position.first, position.second, 1, size, transColor ,massColor));
	nvgFill(vg);
	nvgClosePath(vg);
}	


void line::draw(NVGcontext *vg) {
	NVGcolor lineColor = nvgRGBA(0xFF, 0x7F, 0x00, 0xA0);
	nvgFillColor(vg, lineColor);
	nvgStrokeColor(vg, lineColor);
	nvgStrokeWidth(vg, 2);
	nvgBeginPath(vg);
	nvgMoveTo(vg,positionBegin.first,positionBegin.second);
	nvgLineTo(vg,positionEnd.first,positionEnd.second);
	nvgStroke(vg);
	nvgClosePath(vg);
}

void pendulum::setNrOfNodes(int nrOfNodes) {
	this->nrOfNodes = nrOfNodes;
}

void pendulum::setPosition(std::pair<double,double> position) {
	this->position = position;
}

std::pair<double,double> pendulum::getPosition(void) {
	return position;
}

void pendulum::draw(NVGcontext *vg) {

	for (int i=0;i<nrOfNodes;i++) { 
		nodes[i].draw(vg);
		if (i>0) {
			lines[i-1].setBegin(nodes[i-1].newMass.getPosition());
			lines[i-1].setEnd(nodes[i].newMass.getPosition());
			lines[i-1].draw(vg);
		}
	}
}

void GenieDisplay::drawLayer(const DrawArgs &args,int layer) {

	if (module == NULL) return;
	if (layer == 1) {
		// The clipbox is whole module, so we fix so it is only display screen
		Rect clipBox = args.clipBox;
		clipBox.pos.x = args.clipBox.pos.x + 75;
		nvgScissor(args.vg, RECT_ARGS(clipBox));

		for (int n=0;n<module->nrOfPendulums;n++) {
			module->pendulums[n].draw(args.vg);
		}
	}
	TransparentWidget::drawLayer(args,layer);
}


GenieExpanderModuleWidget::GenieExpanderModuleWidget(GenieExpander* module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GenieExpander.svg")));

	addChild(createWidget<ScrewSilver>(Vec(0, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15,0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(0, 365)));

	// Fill the display for the module browser
	display = new GenieDisplay();
	display->box.pos = Vec(15, 30);
	display->box.size = Vec(WIDTH, HEIGHT);
	display->module = module;
	addChild(display);	

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