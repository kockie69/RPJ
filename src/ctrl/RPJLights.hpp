#include "rack.hpp"

template <typename TBase = ModuleLightWidget>
struct TRedGreenBlueWhiteLight : TBase {
	TRedGreenBlueWhiteLight() {
    	this->addBaseColor(SCHEME_RED);
    	this->addBaseColor(SCHEME_GREEN);
    	this->addBaseColor(SCHEME_BLUE);
		this->addBaseColor(SCHEME_WHITE);
     }
 };

typedef TRedGreenBlueWhiteLight<> RedGreenBlueWhiteLight;

template <typename TBase = ModuleLightWidget>
struct TRedGreenWhiteLight : TBase {
	TRedGreenWhiteLight() {
    	this->addBaseColor(SCHEME_RED);
    	this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_WHITE);
	}
};

typedef TRedGreenWhiteLight<> RedGreenWhiteLight;
