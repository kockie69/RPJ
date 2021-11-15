#include "RPJ.hpp"
#include "Bridge.hpp"
#include <regex>


BridgeIn::BridgeIn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	cid=0;
}

bool BridgeIn::idIsFree(int newCid) {
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
			if ( bridgeBus[i].channelId == newCid) {
				if (bridgeBus[i].publisher==0)
					return true;
				else
					return false;
				break;
			}
	}
	return true;
}

void BridgeIn::removePublisher(int64_t id, int cid) {
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
		if (bridgeBus[i].channelId == cid) {
			if (bridgeBus[i].publisher == id) {
				bridgeBus[i].publisher=0;
				bridgeBus[i].isConnected=false;
			}
			break;
		}
	}
}

void BridgeIn::onRemove(const RemoveEvent & e) {
	removePublisher(id, cid);
}

void BridgeIn::addPublisher(int cid) {
	bool found = false;
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
		if (bridgeBus[i].channelId == cid) {
			found = true;
			if (bridgeBus[i].publisher==0) {
				bridgeBus[i].publisher=id;
				if (bridgeBus[i].subscribers.size()>0)
					bridgeBus[i].isConnected=true;
			}
			break;
		}
	}
	if (!found) {
		//insert a record
		bridgeChannel b;
		b.channelId = cid;
		b.publisher = id;
		b.isConnected = false;
		b.subscribers = {};
		bridgeBus.push_back(b);
	}
}

bool BridgeIn::connected(int cid) {
	bool conn = false;
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
			if ( bridgeBus[i].channelId == cid) {
				conn = bridgeBus[i].isConnected;
				break;
			}
	}
	return conn;
}

void BridgeIn::updatePublisher(int newCid) {
	if (cid > 0)
		removePublisher(id, cid);
	addPublisher(newCid);
	cid = newCid;
}

void BridgeIn::process(const ProcessArgs &args) {

	if (cid!=0) {
		if (connected(cid)) {
			lights[RGB_LIGHT + 0].setBrightness(0.f);
			lights[RGB_LIGHT + 1].setBrightness(1.f);
			lights[RGB_LIGHT + 2].setBrightness(0.f);
		}
		else {
			lights[RGB_LIGHT + 0].setBrightness(1.f);
			lights[RGB_LIGHT + 1].setBrightness(0.f);
			lights[RGB_LIGHT + 2].setBrightness(0.f);
		}
	}
}

void nBridgeInIdMenuItem::onSelectKey(const SelectKeyEvent & e) {
if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		// Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::max(cursor - 1, 0);
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToPrevWord();
			}
			insertText("");
			e.consume(this);
		}
		// Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToNextWord();
			}
			insertText("");
			e.consume(this);
		}
		// Left
		if (e.key == GLFW_KEY_LEFT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToPrevWord();
			}
			else {
				cursor = std::max(cursor - 1, 0);
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Right
		if (e.key == GLFW_KEY_RIGHT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToNextWord();
			}
			else {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Up (placeholder)
		if (e.key == GLFW_KEY_UP) {
			e.consume(this);
		}
		// Down (placeholder)
		if (e.key == GLFW_KEY_DOWN) {
			e.consume(this);
		}
		// Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = 0;
			e.consume(this);
		}
		// Shift+Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = 0;
			e.consume(this);
		}
		// End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = text.size();
			e.consume(this);
		}
		// Shift+End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = text.size();
			e.consume(this);
		}
		// Ctrl+V
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			pasteClipboard();
			e.consume(this);
		}
		// Ctrl+X
		if (e.keyName == "x" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			cutClipboard();
			e.consume(this);
		}
		// Ctrl+C
		if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			copyClipboard();
			e.consume(this);
		}
		// Ctrl+A
		if (e.keyName == "a" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			selectAll();
			e.consume(this);
		}
		// Enter
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && (e.mods & RACK_MOD_MASK) == 0) {
			if (multiline) {
				insertText("\n");
			}
			else {
				ActionEvent eAction;
				onAction(eAction);
			}
			e.consume(this);
			if(regex_match(getText(), std::regex("[0-9]+"))) {
				if (stoi(getText()) >  0) {
					if (module->idIsFree(stoi(getText()))) {
						module->updatePublisher(stoi(getText()));
						getAncestorOfType<ui::MenuOverlay>()->requestDelete();
					}
				}
			}
		}
		// Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0) {
			if (nextField)
				APP->event->setSelectedWidget(nextField);
			e.consume(this);
		}
		// Shift-Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			if (prevField)
				APP->event->setSelectedWidget(prevField);
			e.consume(this);
		}
		// Consume all printable keys
		if ( e.keyName >= "0" && e.keyName <= "9") {
			e.consume(this);
		}

		assert(0 <= cursor);
		assert(cursor <= (int) text.size());
		assert(0 <= selection);
		assert(selection <= (int) text.size());
	}
}

struct BridgeInModuleWidget : ModuleWidget {
	BridgeInModuleWidget(BridgeIn* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BridgeIn.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			
		{
			BridgeDisplay<BridgeIn> * bid = new BridgeDisplay<BridgeIn>(Vec(21,315));
			bid->module = module;
			addChild(bid);
		}
		addInput(createInput<PJ301MPort>(Vec(10, 55), module, BridgeIn::POLYINPUT_A));
		addInput(createInput<PJ301MPort>(Vec(10, 90), module, BridgeIn::POLYINPUT_B));
		addInput(createInput<PJ301MPort>(Vec(10, 125), module, BridgeIn::POLYINPUT_C));
		addInput(createInput<PJ301MPort>(Vec(10, 160), module, BridgeIn::POLYINPUT_D));
		addInput(createInput<PJ301MPort>(Vec(10, 195), module, BridgeIn::POLYINPUT_E));
		addInput(createInput<PJ301MPort>(Vec(10, 230), module, BridgeIn::POLYINPUT_F));
		addInput(createInput<PJ301MPort>(Vec(10, 265), module, BridgeIn::POLYINPUT_G));
		addInput(createInput<PJ301MPort>(Vec(10, 300), module, BridgeIn::POLYINPUT_H));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(Vec(17, 30), module, BridgeIn::RGB_LIGHT));
	}

		void appendContextMenu(Menu *menu) override {
		BridgeIn *module = dynamic_cast<BridgeIn*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Bridge ID"));
		nBridgeInIdMenuItem *nBridgeIdItem = new nBridgeInIdMenuItem();
		nBridgeIdItem->box.size = Vec(120, 20);
		nBridgeIdItem->multiline = false;
		nBridgeIdItem->text = std::to_string(module->cid);
		nBridgeIdItem->module = module;
		nBridgeIdItem->menu = menu;
		menu->addChild(nBridgeIdItem);
	}	
};



Model * modelBridgeIn = createModel<BridgeIn, BridgeInModuleWidget>("BridgeIn");

BridgeOut::BridgeOut() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	cid = 0;
	modwid = NULL;
}

void BridgeOut::removeSubscriber(int64_t id, int cid) {
	if (cid > 0) {
		for (unsigned i=0; i<bridgeBus.size(); ++i) {
			if (bridgeBus[i].channelId == cid) {
				for (unsigned j=0; j<bridgeBus[i].subscribers.size(); ++j) {
					if (bridgeBus[i].subscribers[j] == id) {
						bridgeBus[i].subscribers.erase(bridgeBus[i].subscribers.begin()+j);
						if (bridgeBus[i].subscribers.size()==0)
							bridgeBus[i].isConnected=false;
						break;
					}
				}
				break;
			}
		}
	}
}

void BridgeOut::onRemove(const RemoveEvent & e) {
	removeSubscriber(id, cid);
}

void BridgeOut::addSubscriber(int cid) {
	bool found = false;
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
		if (bridgeBus[i].channelId == cid) {
			found = true;
			bridgeBus[i].subscribers.push_back(id);
			if (bridgeBus[i].publisher!=0)
				bridgeBus[i].isConnected=true;
			break;
		}
	}
	if (!found) {
		bridgeChannel b;
		b.channelId = cid;
		b.publisher = 0;
		b.isConnected = false;
		b.subscribers.push_back(id);
		bridgeBus.push_back(b);
	}
}

void BridgeOut::updateSubscriber(int newCid) {
	if (cid > 0)
		removeSubscriber(id, cid);
	addSubscriber(newCid);
	cid = newCid;
}

bool BridgeOut::connected(int cid) {
	bool conn = false;
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
			if ( bridgeBus[i].channelId == cid) {
				conn =  bridgeBus[i].isConnected;
				break;
			}
	}
	return conn;
}

uint64_t BridgeOut::getPublisherId(int cid) {
	uint64_t publisherId = 0;
	for (unsigned i=0; i<bridgeBus.size(); ++i) {
			if ( bridgeBus[i].channelId == cid) {
				publisherId = bridgeBus[i].publisher;
				break;
			}
	}
	return publisherId;
}

void BridgeOut::process(const ProcessArgs &args) {
	simd::float_4 output;
	int channels;
	Module *publisherModule;

	if (cid != 0) {
		uint64_t publisherId = getPublisherId(cid);
		if (publisherId) {
			publisherModule = APP->engine->getModule(publisherId);
			for (int i=0;i<8;i++) {
				if (publisherModule->inputs[i].isConnected() && outputs[i].isConnected()) {
					channels = publisherModule->inputs[i].getChannels();
					outputs[i].setChannels(channels);
					for (int c=0;c<channels;c+=4) {
						output = simd::float_4::load(publisherModule->inputs[i].getVoltages(c));
						output.store(outputs[i].getVoltages(c));
					}
				}
			}
			lights[RGB_LIGHT + 0].setBrightness(0.f);
			lights[RGB_LIGHT + 1].setBrightness(1.f);
			lights[RGB_LIGHT + 2].setBrightness(0.f);
		}
		else {
			lights[RGB_LIGHT + 0].setBrightness(1.f);
			lights[RGB_LIGHT + 1].setBrightness(0.f);
			lights[RGB_LIGHT + 2].setBrightness(0.f);
		}
	}
}

void nBridgeOutIdMenuItem::onSelectKey(const SelectKeyEvent & e) {
if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		// Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::max(cursor - 1, 0);
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToPrevWord();
			}
			insertText("");
			e.consume(this);
		}
		// Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToNextWord();
			}
			insertText("");
			e.consume(this);
		}
		// Left
		if (e.key == GLFW_KEY_LEFT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToPrevWord();
			}
			else {
				cursor = std::max(cursor - 1, 0);
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Right
		if (e.key == GLFW_KEY_RIGHT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToNextWord();
			}
			else {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Up (placeholder)
		if (e.key == GLFW_KEY_UP) {
			e.consume(this);
		}
		// Down (placeholder)
		if (e.key == GLFW_KEY_DOWN) {
			e.consume(this);
		}
		// Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = 0;
			e.consume(this);
		}
		// Shift+Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = 0;
			e.consume(this);
		}
		// End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = text.size();
			e.consume(this);
		}
		// Shift+End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = text.size();
			e.consume(this);
		}
		// Ctrl+V
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			pasteClipboard();
			e.consume(this);
		}
		// Ctrl+X
		if (e.keyName == "x" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			cutClipboard();
			e.consume(this);
		}
		// Ctrl+C
		if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			copyClipboard();
			e.consume(this);
		}
		// Ctrl+A
		if (e.keyName == "a" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			selectAll();
			e.consume(this);
		}
		// Enter
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && (e.mods & RACK_MOD_MASK) == 0) {
			if (multiline) {
				insertText("\n");
			}
			else {
				ActionEvent eAction;
				onAction(eAction);
			}
			e.consume(this);
			if(regex_match(getText(), std::regex("[0-9]+"))) {
				if (stoi(getText()) >  0) {
					module->updateSubscriber(stoi(getText()));
					getAncestorOfType<ui::MenuOverlay>()->requestDelete();
				}
			}
		}
		// Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0) {
			if (nextField)
				APP->event->setSelectedWidget(nextField);
			e.consume(this);
		}
		// Shift-Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			if (prevField)
				APP->event->setSelectedWidget(prevField);
			e.consume(this);
		}
		// Consume all printable keys
		if ( e.keyName >= "0" && e.keyName <= "9") {
			e.consume(this);
		}

		assert(0 <= cursor);
		assert(cursor <= (int) text.size());
		assert(0 <= selection);
		assert(selection <= (int) text.size());
	}
}

struct BridgeOutModuleWidget : ModuleWidget {
	BridgeOutModuleWidget(BridgeOut* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BridgeOut.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			
		{
			BridgeDisplay<BridgeOut> * bod = new BridgeDisplay<BridgeOut>(Vec(21,315));
			bod->module = module;
			addChild(bod);
		}
		addOutput(createOutput<PJ301MPort>(Vec(10, 55), module, BridgeOut::POLYOUTPUT_A));
		addOutput(createOutput<PJ301MPort>(Vec(10, 90), module, BridgeOut::POLYOUTPUT_B));
		addOutput(createOutput<PJ301MPort>(Vec(10, 125), module, BridgeOut::POLYOUTPUT_C));
		addOutput(createOutput<PJ301MPort>(Vec(10, 160), module, BridgeOut::POLYOUTPUT_D));
		addOutput(createOutput<PJ301MPort>(Vec(10, 195), module, BridgeOut::POLYOUTPUT_E));
		addOutput(createOutput<PJ301MPort>(Vec(10, 230), module, BridgeOut::POLYOUTPUT_F));
		addOutput(createOutput<PJ301MPort>(Vec(10, 265), module, BridgeOut::POLYOUTPUT_G));
		addOutput(createOutput<PJ301MPort>(Vec(10, 300), module, BridgeOut::POLYOUTPUT_H));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(Vec(17, 30), module, BridgeOut::RGB_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		BridgeOut *module = dynamic_cast<BridgeOut*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Bridge ID"));
		nBridgeOutIdMenuItem *nBridgeIdItem = new nBridgeOutIdMenuItem();
		nBridgeIdItem->box.size = Vec(120, 20);
		nBridgeIdItem->multiline = false;
		nBridgeIdItem->text = std::to_string(module->cid);
		nBridgeIdItem->module = module;
		nBridgeIdItem->menu = menu;
		menu->addChild(nBridgeIdItem);
	}
};

Model * modelBridgeOut = createModel<BridgeOut, BridgeOutModuleWidget>("BridgeOut");