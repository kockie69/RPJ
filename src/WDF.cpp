#include "WDF.hpp"

/** initialize with source resistor R1 */
void IComponentAdaptor::initialize(double _R1) {}

/** initialize all downstream adaptors in the chain */
void IComponentAdaptor::initializeAdaptorChain() {}

/** set input value into component port  */
void IComponentAdaptor::setInput(double _in) {}

/** get output value from component port  */
double IComponentAdaptor::getOutput() { return 0.0; }

// --- for adaptors
/** ADAPTOR: set input port 1  */
//virtual void IComponentAdaptor::setInput1(double _in1);

/** ADAPTOR: set input port 2  */
//void IComponentAdaptor::setInput2(double _in2);

/** ADAPTOR: set input port 3 */
//|void IComponentAdaptor::setInput3(double _in3);

/** ADAPTOR: get output port 1 value */
//double IComponentAdaptor::getOutput1();

/** ADAPTOR: get output port 2 value */
//double IComponentAdaptor::getOutput2();

/** ADAPTOR: get output port 3 value */
//double IComponentAdaptor::getOutput3();

/** reset the object with new sample rate */
void IComponentAdaptor::reset(double _sampleRate) {}

/** get the commponent resistance from the attached object at Port3 */
double IComponentAdaptor::getComponentResistance() { return 0.0; }

/** get the commponent conductance from the attached object at Port3 */
double IComponentAdaptor::getComponentConductance() { return 0.0; }

/** update the commponent resistance at Port3 */
void IComponentAdaptor::updateComponentResistance() {}

/** set an individual component value (may be R, L, or C */
void IComponentAdaptor::setComponentValue(double _componentValue) { }

/** set LC combined values */
void IComponentAdaptor::setComponentValue_LC(double componentValue_L, double componentValue_C) { }

/** set RL combined values */
void IComponentAdaptor::setComponentValue_RL(double componentValue_R, double componentValue_L) { }

/** set RC combined values */
void IComponentAdaptor::setComponentValue_RC(double componentValue_R, double componentValue_C) { }

/** get a component value */
double IComponentAdaptor::getComponentValue() { return 0.0; }

WdfResistor::WdfResistor(double _componentValue) { componentValue = _componentValue; }
WdfResistor::WdfResistor() { }
WdfResistor::~WdfResistor() {}

/** set sample rate and update component */
void WdfResistor::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfResistor::getComponentResistance() { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfResistor::getComponentConductance() { return 1.0 / componentResistance; }

/** get the component value */
double WdfResistor::getComponentValue() { return componentValue; }

/** set the component value */
void WdfResistor::setComponentValue(double _componentValue)
{
	componentValue = _componentValue;
	updateComponentResistance();
}

/** change the resistance of component */
void WdfResistor::updateComponentResistance()  { componentResistance = componentValue; }

/** reset the component; clear registers */
void WdfResistor::reset(double _sampleRate)  { setSampleRate(_sampleRate);  zRegister = 0.0; }

/** set input value into component; NOTE: resistor is dead-end energy sink so this function does nothing */
void WdfResistor::setInput(double in)  {}

/** get output value; NOTE: a WDF resistor produces no reflected output */
double WdfResistor::getOutput()  { return 0.0; }

/** get output1 value; only one resistor output (not used) */
double WdfResistor::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfResistor::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfResistor::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfResistor::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfResistor::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfResistor::setInput3(double _in3)  {}

WdfCapacitor::WdfCapacitor(double _componentValue) { componentValue = _componentValue; }
WdfCapacitor::WdfCapacitor() { }
WdfCapacitor::~WdfCapacitor() {}

/** set sample rate and update component */
void WdfCapacitor::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfCapacitor::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfCapacitor::getComponentConductance()  { return 1.0 / componentResistance; }

/** get the component value */
double WdfCapacitor::getComponentValue()  { return componentValue; }

/** set the component value */
void WdfCapacitor::setComponentValue(double _componentValue) 
{
	componentValue = _componentValue;
	updateComponentResistance();
}

/** change the resistance of component */
void WdfCapacitor::updateComponentResistance() 
{
	componentResistance = 1.0 / (2.0*componentValue*sampleRate);
}

/** reset the component; clear registers */
void WdfCapacitor::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister = 0.0; }

/** set input value into component; NOTE: capacitor sets value into register*/
void WdfCapacitor::setInput(double in)  { zRegister = in; }

/** get output value; NOTE: capacitor produces reflected output */
double WdfCapacitor::getOutput()  { return zRegister; }	// z^-1

/** get output1 value; only one capacitor output (not used) */
double WdfCapacitor::getOutput1()  { return  getOutput(); }

/** get output2 value; only one capacitor output (not used) */
double WdfCapacitor::getOutput2()  { return  getOutput(); }

/** get output3 value; only one capacitor output (not used) */
double WdfCapacitor::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfCapacitor::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfCapacitor::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfCapacitor::setInput3(double _in3)  {}

WdfInductor::WdfInductor(double _componentValue) { componentValue = _componentValue; }
WdfInductor::WdfInductor() { }
WdfInductor::~WdfInductor() {}

/** set sample rate and update component */
void WdfInductor::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfInductor::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfInductor::getComponentConductance()  { return 1.0 / componentResistance; }

/** get the component value */
double WdfInductor::getComponentValue()  { return componentValue; }

/** set the component value */
void WdfInductor::setComponentValue(double _componentValue) 
{
	componentValue = _componentValue;
	updateComponentResistance();
}

/** change the resistance of component R(L) = 2Lfs */
void WdfInductor::updateComponentResistance()  { componentResistance = 2.0*componentValue*sampleRate;}

/** reset the component; clear registers */
void WdfInductor::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister = 0.0; }

/** set input value into component; NOTE: inductor sets value into storage register */
void WdfInductor::setInput(double in)  { zRegister = in; }

/** get output value; NOTE: a WDF inductor produces reflected output that is inverted */
double WdfInductor::getOutput()  { return -zRegister; } // -z^-1

/** get output1 value; only one resistor output (not used) */
double WdfInductor::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfInductor::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfInductor::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfInductor::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfInductor::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfInductor::setInput3(double _in3)  {}

WdfSeriesLC::WdfSeriesLC() {}
WdfSeriesLC::WdfSeriesLC(double _componentValue_L, double _componentValue_C) {
	componentValue_L = _componentValue_L;
	componentValue_C = _componentValue_C;
}

WdfSeriesLC::~WdfSeriesLC() {}

/** set sample rate and update component */
void WdfSeriesLC::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfSeriesLC::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfSeriesLC::getComponentConductance()  { return 1.0 / componentResistance; }

/** change the resistance of component; see FX book for details */
void WdfSeriesLC::updateComponentResistance() 
{
	RL = 2.0*componentValue_L*sampleRate;
	RC = 1.0 / (2.0*componentValue_C*sampleRate);
	componentResistance = RL + (1.0 / RC);
}

/** set both LC components at once */
void WdfSeriesLC::setComponentValue_LC(double _componentValue_L, double _componentValue_C) 
{
	componentValue_L = _componentValue_L;
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** set L component */
void WdfSeriesLC::setComponentValue_L(double _componentValue_L)
{
	componentValue_L = _componentValue_L;
	updateComponentResistance();
}

/** set C component */
void WdfSeriesLC::setComponentValue_C(double _componentValue_C)
{
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** get L component value */
double WdfSeriesLC::getComponentValue_L() { return componentValue_L; }

/** get C component value */
double WdfSeriesLC::getComponentValue_C() { return componentValue_C; }

/** reset the component; clear registers */
void WdfSeriesLC::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

/** set input value into component; NOTE: K is calculated here */
void WdfSeriesLC::setInput(double in) 
{
	double YC = 1.0 / RC;
	double K = (1.0 - RL*YC) / (1.0 + RL*YC);
	double N1 = K*(in - zRegister_L);
	zRegister_L = N1 + zRegister_C;
	zRegister_C = in;
}

/** get output value; NOTE: utput is located in zReg_L */
double WdfSeriesLC::getOutput()  { return zRegister_L; }

/** get output1 value; only one resistor output (not used) */
double WdfSeriesLC::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfSeriesLC::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfSeriesLC::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfSeriesLC::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfSeriesLC::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfSeriesLC::setInput3(double _in3)  {}

WdfParallelLC::WdfParallelLC() {}
WdfParallelLC::WdfParallelLC(double _componentValue_L, double _componentValue_C)
{
	componentValue_L = _componentValue_L;
	componentValue_C = _componentValue_C;
}
WdfParallelLC::~WdfParallelLC() {}

/** set sample rate and update component */
void WdfParallelLC::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfParallelLC::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfParallelLC::getComponentConductance()  { return 1.0 / componentResistance; }

/** change the resistance of component; see FX book for details */
void WdfParallelLC::updateComponentResistance() 
{
	RL = 2.0*componentValue_L*sampleRate;
	RC = 1.0 / (2.0*componentValue_C*sampleRate);
	componentResistance = (RC + 1.0 / RL);
}

/** set both LC components at once */
void WdfParallelLC::setComponentValue_LC(double _componentValue_L, double _componentValue_C) 
{
	componentValue_L = _componentValue_L;
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** set L component */
void WdfParallelLC::setComponentValue_L(double _componentValue_L)
{
	componentValue_L = _componentValue_L;
	updateComponentResistance();
}

/** set C component */
void WdfParallelLC::setComponentValue_C(double _componentValue_C)
{
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** get L component value */
double WdfParallelLC::getComponentValue_L() { return componentValue_L; }

/** get C component value */
double WdfParallelLC::getComponentValue_C() { return componentValue_C; }

/** reset the component; clear registers */
void WdfParallelLC::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

/** set input value into component; NOTE: K is calculated here */
void WdfParallelLC::setInput(double in) 
{
	double YL = 1.0 / RL;
	double K = (YL*RC - 1.0) / (YL*RC + 1.0);
	double N1 = K*(in - zRegister_L);
	zRegister_L = N1 + zRegister_C;
	zRegister_C = in;
}

/** get output value; NOTE: output is located in -zReg_L */
double WdfParallelLC::getOutput()  { return -zRegister_L; }

/** get output1 value; only one resistor output (not used) */
double WdfParallelLC::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfParallelLC::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfParallelLC::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfParallelLC::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfParallelLC::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfParallelLC::setInput3(double _in3) {}

WdfSeriesRL::WdfSeriesRL() {}
WdfSeriesRL::WdfSeriesRL(double _componentValue_R, double _componentValue_L) {
	componentValue_L = _componentValue_L;
	componentValue_R = _componentValue_R;
}
WdfSeriesRL::~WdfSeriesRL() {}

/** set sample rate and update component */
void WdfSeriesRL::setSampleRate(double _sampleRate) {
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfSeriesRL::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfSeriesRL::getComponentConductance()  { return 1.0 / componentResistance; }

/** change the resistance of component; see FX book for details */
void WdfSeriesRL::updateComponentResistance() {
		RR = componentValue_R;
		RL = 2.0*componentValue_L*sampleRate;
		componentResistance = RR + RL;
		K = RR / componentResistance;
	}

/** set both RL components at once */
void WdfSeriesRL::setComponentValue_RL(double _componentValue_R, double _componentValue_L) {
	componentValue_L = _componentValue_L;
	componentValue_R = _componentValue_R;
	updateComponentResistance();
}

/** set L component */
void WdfSeriesRL::setComponentValue_L(double _componentValue_L) {
	componentValue_L = _componentValue_L;
	updateComponentResistance();
}

/** set R component */
void WdfSeriesRL::setComponentValue_R(double _componentValue_R) {
	componentValue_R = _componentValue_R;
	updateComponentResistance();
}

/** get L component value */
double WdfSeriesRL::getComponentValue_L() { return componentValue_L; }

/** get R component value */
double WdfSeriesRL::getComponentValue_R() { return componentValue_R; }

/** reset the component; clear registers */
void WdfSeriesRL::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

/** set input value into component */
void WdfSeriesRL::setInput(double in)  { zRegister_L = in; }

/** get output value; NOTE: see FX book for details */
double WdfSeriesRL::getOutput() {
	double NL = -zRegister_L;
	double out = NL*(1.0 - K) - K*zRegister_C;
	zRegister_C = out;

	return out;
}

/** get output1 value; only one resistor output (not used) */
double WdfSeriesRL::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfSeriesRL::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfSeriesRL::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfSeriesRL::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfSeriesRL::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfSeriesRL::setInput3(double _in3)  {}

WdfParallelRL::WdfParallelRL() {}
WdfParallelRL::WdfParallelRL(double _componentValue_R, double _componentValue_L)	{
	componentValue_L = _componentValue_L;
	componentValue_R = _componentValue_R;
}
WdfParallelRL::~WdfParallelRL() {}

/** set sample rate and update component */
void WdfParallelRL::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfParallelRL::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfParallelRL::getComponentConductance()  { return 1.0 / componentResistance; }

/** change the resistance of component; see FX book for details */
void WdfParallelRL::updateComponentResistance() 
{
	RR = componentValue_R;
	RL = 2.0*componentValue_L*sampleRate;
	componentResistance = 1.0 / ((1.0 / RR) + (1.0 / RL));
	K = componentResistance / RR;
}


/** set both RL components at once */
void WdfParallelRL::setComponentValue_RL(double _componentValue_R, double _componentValue_L) 
{
	componentValue_L = _componentValue_L;
	componentValue_R = _componentValue_R;
	updateComponentResistance();
}

/** set L component */
void WdfParallelRL::setComponentValue_L(double _componentValue_L)
{
	componentValue_L = _componentValue_L;
	updateComponentResistance();
}

/** set R component */
void WdfParallelRL::setComponentValue_R(double _componentValue_R)
{
	componentValue_R = _componentValue_R;
	updateComponentResistance();
}

/** get L component value */
double WdfParallelRL::getComponentValue_L() { return componentValue_L; }

/** get R component value */
double WdfParallelRL::getComponentValue_R() { return componentValue_R; }

/** reset the component; clear registers */
void WdfParallelRL::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

/** set input value into component */
void WdfParallelRL::setInput(double in)  { zRegister_L = in; }

/** get output value; NOTE: see FX book for details */
double WdfParallelRL::getOutput() 
{
	double NL = -zRegister_L;
	double out = NL*(1.0 - K) + K*zRegister_C;
	zRegister_C = out;
	return out;
}

/** get output1 value; only one resistor output (not used) */
double WdfParallelRL::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfParallelRL::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfParallelRL::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfParallelRL::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfParallelRL::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfParallelRL::setInput3(double _in3)  {}

WdfSeriesRC::WdfSeriesRC() {}
WdfSeriesRC::WdfSeriesRC(double _componentValue_R, double _componentValue_C)
{
	componentValue_C = _componentValue_C;
	componentValue_R = _componentValue_R;
}
WdfSeriesRC::~WdfSeriesRC() {}

/** set sample rate and update component */
void WdfSeriesRC::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfSeriesRC::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfSeriesRC::getComponentConductance()  { return 1.0 / componentResistance; }

/** change the resistance of component; see FX book for details */
void WdfSeriesRC::updateComponentResistance() 
{
	RR = componentValue_R;
	RC = 1.0 / (2.0*componentValue_C*sampleRate);
	componentResistance = RR + RC;
	K = RR / componentResistance;
}

/** set both RC components at once */
void WdfSeriesRC::setComponentValue_RC(double _componentValue_R, double _componentValue_C) 
{
	componentValue_R = _componentValue_R;
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** set R component */
void WdfSeriesRC::setComponentValue_R(double _componentValue_R)
{
	componentValue_R = _componentValue_R;
	updateComponentResistance();
}

/** set C component */
void WdfSeriesRC::setComponentValue_C(double _componentValue_C)
{
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** get R component value */
double WdfSeriesRC::getComponentValue_R() { return componentValue_R; } 

/** get C component value */
double WdfSeriesRC::getComponentValue_C() { return componentValue_C; }

/** reset the component; clear registers */
void WdfSeriesRC::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

/** set input value into component */
void WdfSeriesRC::setInput(double in)  { zRegister_L = in; }

/** get output value; NOTE: see FX book for details */
double WdfSeriesRC::getOutput() 
{
	double NL = zRegister_L;
	double out = NL*(1.0 - K) + K*zRegister_C;
	zRegister_C = out;
	return out;
}

/** get output1 value; only one resistor output (not used) */
double WdfSeriesRC::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfSeriesRC::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfSeriesRC::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfSeriesRC::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfSeriesRC::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfSeriesRC::setInput3(double _in3)  {}

WdfParallelRC::WdfParallelRC() {}
WdfParallelRC::WdfParallelRC(double _componentValue_R, double _componentValue_C)
{
	componentValue_C = _componentValue_C;
	componentValue_R = _componentValue_R;
}
WdfParallelRC::~WdfParallelRC() {}

/** set sample rate and update component */
void WdfParallelRC::setSampleRate(double _sampleRate)
{
	sampleRate = _sampleRate;
	updateComponentResistance();
}

/** get component's value as a resistance */
double WdfParallelRC::getComponentResistance()  { return componentResistance; }

/** get component's value as a conducatance (or admittance) */
double WdfParallelRC::getComponentConductance()  { return 1.0 / componentResistance; }

/** change the resistance of component; see FX book for details */
void WdfParallelRC::updateComponentResistance() 
{
	RR = componentValue_R;
	RC = 1.0 / (2.0*componentValue_C*sampleRate);
	componentResistance = 1.0 / ((1.0 / RR) + (1.0 / RC));
	K = componentResistance / RR;
}

/** set both RC components at once */
void WdfParallelRC::setComponentValue_RC(double _componentValue_R, double _componentValue_C) 
{
	componentValue_R = _componentValue_R;
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** set R component */
void WdfParallelRC::setComponentValue_R(double _componentValue_R)
{
	componentValue_R = _componentValue_R;
	updateComponentResistance();
}

/** set C component */
void WdfParallelRC::setComponentValue_C(double _componentValue_C)
{
	componentValue_C = _componentValue_C;
	updateComponentResistance();
}

/** get R component value */
double WdfParallelRC::getComponentValue_R() { return componentValue_R; }

/** get C component value */
double WdfParallelRC::getComponentValue_C() { return componentValue_C; }

/** reset the component; clear registers */
void WdfParallelRC::reset(double _sampleRate)  { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

/** set input value into component; */
void WdfParallelRC::setInput(double in)  { zRegister_L = in; }

/** get output value; NOTE: output is located in zRegister_C */
double WdfParallelRC::getOutput() 
{
	double NL = zRegister_L;
	double out = NL*(1.0 - K) - K*zRegister_C;
	zRegister_C = out;
	return out;
}

/** get output1 value; only one resistor output (not used) */
double WdfParallelRC::getOutput1()  { return  getOutput(); }

/** get output2 value; only one resistor output (not used) */
double WdfParallelRC::getOutput2()  { return  getOutput(); }

/** get output3 value; only one resistor output (not used) */
double WdfParallelRC::getOutput3()  { return  getOutput(); }

/** set input1 value; not used for components */
void WdfParallelRC::setInput1(double _in1)  {}

/** set input2 value; not used for components */
void WdfParallelRC::setInput2(double _in2)  {}

/** set input3 value; not used for components */
void WdfParallelRC::setInput3(double _in3)  {}

WdfComponentInfo::WdfComponentInfo() { }

WdfComponentInfo::WdfComponentInfo(wdfComponentType _componentType, double value1 = 0.0, double value2 = 0.0)
{		
	componentType = _componentType;
	if (componentType == wdfComponentType::R)
		R = value1;
	else if (componentType == wdfComponentType::L)
		L = value1;
	else if (componentType == wdfComponentType::C)
		C = value1;
	else if (componentType == wdfComponentType::seriesLC || componentType == wdfComponentType::parallelLC)
	{
		L = value1;
		C = value2;
	}
	else if (componentType == wdfComponentType::seriesRL || componentType == wdfComponentType::parallelRL)
	{
		R = value1;
		L = value2;
	}
	else if (componentType == wdfComponentType::seriesRC || componentType == wdfComponentType::parallelRC)
	{
		R = value1;
		C = value2;
	}
}

WdfAdaptorBase::WdfAdaptorBase() {}
WdfAdaptorBase::~WdfAdaptorBase() {}

/** set the termainal (load) resistance for terminating adaptors */
void WdfAdaptorBase::setTerminalResistance(double _terminalResistance) { terminalResistance = _terminalResistance; }

/** set the termainal (load) resistance as open circuit for terminating adaptors */
void WdfAdaptorBase::setOpenTerminalResistance(bool _openTerminalResistance = true)
{
	// --- flag overrides value
	openTerminalResistance = _openTerminalResistance;
	terminalResistance = 1.0e+34; // avoid /0.0
}

/** set the input (source) resistance for an input adaptor */
void WdfAdaptorBase::setSourceResistance(double _sourceResistance) { sourceResistance = _sourceResistance; }

/** set the component or connected adaptor at port 1; functions is generic and allows extending the functionality of the WDF Library */
void WdfAdaptorBase::setPort1_CompAdaptor(IComponentAdaptor* _port1CompAdaptor) { port1CompAdaptor = _port1CompAdaptor; }

/** set the component or connected adaptor at port 2; functions is generic and allows extending the functionality of the WDF Library */
void WdfAdaptorBase::setPort2_CompAdaptor(IComponentAdaptor* _port2CompAdaptor) { port2CompAdaptor = _port2CompAdaptor; }

/** set the component or connected adaptor at port 3; functions is generic and allows extending the functionality of the WDF Library */
void WdfAdaptorBase::setPort3_CompAdaptor(IComponentAdaptor* _port3CompAdaptor) { port3CompAdaptor = _port3CompAdaptor; }

/** reset the connected component */
void WdfAdaptorBase::reset(double _sampleRate) {
	if (wdfComponent)
		wdfComponent->reset(_sampleRate);
}

/** creates a new WDF component and connects it to Port 3 */
void WdfAdaptorBase::setComponent(wdfComponentType componentType, double value1 = 0.0, double value2 = 0.0)
{
	// --- decode and set
	if (componentType == wdfComponentType::R)
	{
		wdfComponent = new WdfResistor;
		wdfComponent->setComponentValue(value1);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::L)
	{
		wdfComponent = new WdfInductor;
		wdfComponent->setComponentValue(value1);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::C)
	{
		wdfComponent = new WdfCapacitor;
		wdfComponent->setComponentValue(value1);			
		port3CompAdaptor = wdfComponent;		
	}		
	else if (componentType == wdfComponentType::seriesLC)
	{
		wdfComponent = new WdfSeriesLC;
		wdfComponent->setComponentValue_LC(value1, value2);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::parallelLC)
	{
		wdfComponent = new WdfParallelLC;
		wdfComponent->setComponentValue_LC(value1, value2);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::seriesRL)
	{
		wdfComponent = new WdfSeriesRL;
		wdfComponent->setComponentValue_RL(value1, value2);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::parallelRL)
	{
		wdfComponent = new WdfParallelRL;
		wdfComponent->setComponentValue_RL(value1, value2);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::seriesRC)
	{
		wdfComponent = new WdfSeriesRC;
		wdfComponent->setComponentValue_RC(value1, value2);
		port3CompAdaptor = wdfComponent;
	}
	else if (componentType == wdfComponentType::parallelRC)
	{
		wdfComponent = new WdfParallelRC;
		wdfComponent->setComponentValue_RC(value1, value2);
		port3CompAdaptor = wdfComponent;
	}
}

/** connect two adapters together upstreamAdaptor --> downstreamAdaptor */
void WdfAdaptorBase::connectAdaptors(WdfAdaptorBase* upstreamAdaptor, WdfAdaptorBase* downstreamAdaptor)
{
	upstreamAdaptor->setPort2_CompAdaptor(downstreamAdaptor);
	downstreamAdaptor->setPort1_CompAdaptor(upstreamAdaptor);
}

/** initialize the chain of adaptors from upstreamAdaptor --> downstreamAdaptor */
void WdfAdaptorBase::initializeAdaptorChain() 
{
	initialize(sourceResistance);
}

/** set value of single-component adaptor */
void WdfAdaptorBase::setComponentValue(double _componentValue) 
{
	if (wdfComponent)
		wdfComponent->setComponentValue(_componentValue);
}

/** set LC value of mjulti-component adaptor */
void WdfAdaptorBase::setComponentValue_LC(double componentValue_L, double componentValue_C) 
{
	if (wdfComponent)
		wdfComponent->setComponentValue_LC(componentValue_L, componentValue_C);
}

/** set RL value of mjulti-component adaptor */
void WdfAdaptorBase::setComponentValue_RL(double componentValue_R, double componentValue_L) 
{
	if (wdfComponent)
		wdfComponent->setComponentValue_RL(componentValue_R, componentValue_L);
}

/** set RC value of mjulti-component adaptor */
void WdfAdaptorBase::setComponentValue_RC(double componentValue_R, double componentValue_C) 
{
	if (wdfComponent)
		wdfComponent->setComponentValue_RC(componentValue_R, componentValue_C);
}

/** get adaptor connected at port 1: for extended functionality; not used in WDF ladder filter library */
IComponentAdaptor* WdfAdaptorBase::getPort1_CompAdaptor() { return port1CompAdaptor; }

/** get adaptor connected at port 2: for extended functionality; not used in WDF ladder filter library */
IComponentAdaptor* WdfAdaptorBase::getPort2_CompAdaptor() { return port2CompAdaptor; }

/** get adaptor connected at port 3: for extended functionality; not used in WDF ladder filter library */
IComponentAdaptor* WdfAdaptorBase::getPort3_CompAdaptor() { return port3CompAdaptor; }

WdfSeriesAdaptor::WdfSeriesAdaptor() {}
WdfSeriesAdaptor::~WdfSeriesAdaptor() {}

/** get the resistance at port 2; R2 = R1 + component (series)*/
double WdfSeriesAdaptor::getR2()
{
	double componentResistance = 0.0;
	if (getPort3_CompAdaptor())
		componentResistance = getPort3_CompAdaptor()->getComponentResistance();

	R2 = R1 + componentResistance;
	return R2;
}

/** initialize adaptor with input resistance */
void WdfSeriesAdaptor::initialize(double _R1)
{
	// --- R1 is source resistance for this adaptor
	R1 = _R1;

	double componentResistance = 0.0;
	if (getPort3_CompAdaptor())
		componentResistance = getPort3_CompAdaptor()->getComponentResistance();

	// --- calculate B coeff
	B = R1 / (R1 + componentResistance);

	// --- init downstream adaptor
	if (getPort2_CompAdaptor())
		getPort2_CompAdaptor()->initialize(getR2());

	// --- not used in this implementation but saving for extended use
	R3 = componentResistance;
}

/** push audio input sample into incident wave input*/
void WdfSeriesAdaptor::setInput1(double _in1) 
{
	// --- save
	in1 = _in1;

	// --- read component value
	N2 = 0.0;
	if (getPort3_CompAdaptor())
		N2 = getPort3_CompAdaptor()->getOutput();

	// --- form output
	out2 = -(in1 + N2);

	// --- deliver downstream
	if (getPort2_CompAdaptor())
		getPort2_CompAdaptor()->setInput1(out2);
}

/** push audio input sample into reflected wave input */
void WdfSeriesAdaptor::setInput2(double _in2) 	
{
	// --- save
	in2 = _in2;

	// --- calc N1
	N1 = -(in1 - B*(in1 + N2 + in2) + in2);

	// --- calc out1
	out1 = in1 - B*(N2 + in2);

	// --- deliver upstream
	if (getPort1_CompAdaptor())
		getPort1_CompAdaptor()->setInput2(out1);

	// --- set component state
	if (getPort3_CompAdaptor())
		getPort3_CompAdaptor()->setInput(N1);
}

/** set input 3 always connects to component */
void WdfSeriesAdaptor::setInput3(double _in3)  { }

/** get OUT1 = reflected output pin on Port 1 */
double WdfSeriesAdaptor::getOutput1()  { return out1; }

/** get OUT2 = incident (normal) output pin on Port 2 */
double WdfSeriesAdaptor::getOutput2()  { return out2; }

/** get OUT3 always connects to component */
double WdfSeriesAdaptor::getOutput3()  { return out3; }

WdfSeriesTerminatedAdaptor::WdfSeriesTerminatedAdaptor() {}
WdfSeriesTerminatedAdaptor::~WdfSeriesTerminatedAdaptor() {}

/** get the resistance at port 2; R2 = R1 + component (series)*/
double WdfSeriesTerminatedAdaptor::getR2()
{
		double componentResistance = 0.0;
if (getPort3_CompAdaptor())
		componentResistance = getPort3_CompAdaptor()->getComponentResistance();

	R2 = R1 + componentResistance;
	return R2;
}

/** initialize adaptor with input resistance */
void WdfSeriesTerminatedAdaptor::initialize(double _R1) 
{
	// --- source impedance
	R1 = _R1;

	double componentResistance = 0.0;
	if (getPort3_CompAdaptor())
		componentResistance = getPort3_CompAdaptor()->getComponentResistance();

	B1 = (2.0*R1) / (R1 + componentResistance + terminalResistance);
	B3 = (2.0*terminalResistance) / (R1 + componentResistance + terminalResistance);

	// --- init downstream
	if (getPort2_CompAdaptor())
		getPort2_CompAdaptor()->initialize(getR2());

	// --- not used in this implementation but saving for extended use
	R3 = componentResistance;
}

/** push audio input sample into incident wave input*/
void WdfSeriesTerminatedAdaptor::setInput1(double _in1) 
{
	// --- save
	in1 = _in1;

	N2 = 0.0;
	if (getPort3_CompAdaptor())
		N2 = getPort3_CompAdaptor()->getOutput();

	double N3 = in1 + N2;

	// --- calc out2 y(n)
	out2 = -B3*N3;

	// --- form output1
	out1 = in1 - B1*N3;

	// --- form N1
	N1 = -(out1 + out2 + N3);

	// --- deliver upstream to input2
	if (getPort1_CompAdaptor())
		getPort1_CompAdaptor()->setInput2(out1);

	// --- set component state
	if (getPort3_CompAdaptor())
	getPort3_CompAdaptor()->setInput(N1);
}

/** push audio input sample into reflected wave input
	for terminated adaptor, this is dead end, just store it */
void WdfSeriesTerminatedAdaptor::setInput2(double _in2)  { in2 = _in2;}

/** set input 3 always connects to component */
void WdfSeriesTerminatedAdaptor::setInput3(double _in3)  { in3 = _in3;}

/** get OUT1 = reflected output pin on Port 1 */
double WdfSeriesTerminatedAdaptor::getOutput1()  { return out1; }

/** get OUT2 = incident (normal) output pin on Port 2 */
double WdfSeriesTerminatedAdaptor::getOutput2()  { return out2; }

/** get OUT3 always connects to component */
double WdfSeriesTerminatedAdaptor::getOutput3()  { return out3; }

WdfParallelAdaptor::WdfParallelAdaptor() {}
WdfParallelAdaptor::~WdfParallelAdaptor() {}

/** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
double WdfParallelAdaptor::getR2()
{
	double componentConductance = 0.0;
	if (getPort3_CompAdaptor())
		componentConductance = getPort3_CompAdaptor()->getComponentConductance();

	// --- 1 / (sum of admittances)
	R2 = 1.0 / ((1.0 / R1) + componentConductance);
	return R2;
}

/** initialize adaptor with input resistance */
void WdfParallelAdaptor::initialize(double _R1) 
{
	// --- save R1
	R1 = _R1;

	double G1 = 1.0 / R1;
	double componentConductance = 0.0;
	if (getPort3_CompAdaptor())
		componentConductance = getPort3_CompAdaptor()->getComponentConductance();

	// --- calculate B coeff
	A = G1 / (G1 + componentConductance);

	// --- now, do we init our downstream??
	if (getPort2_CompAdaptor())
		getPort2_CompAdaptor()->initialize(getR2());

	// --- not used in this implementation but saving for extended use
	R3 = 1.0/ componentConductance;
}

/** push audio input sample into incident wave input*/
void WdfParallelAdaptor::setInput1(double _in1) 
{
	// --- save
	in1 = _in1;

	// --- read component
	N2 = 0.0;
	if (getPort3_CompAdaptor())
		N2 = getPort3_CompAdaptor()->getOutput();

	// --- form output
	out2 = N2 - A*(-in1 + N2);

	// --- deliver downstream
	if (getPort2_CompAdaptor())
		getPort2_CompAdaptor()->setInput1(out2);
}

/** push audio input sample into reflected wave input*/
void WdfParallelAdaptor::setInput2(double _in2) 
{
	// --- save
	in2 = _in2;

	// --- calc N1
	N1 = in2 - A*(-in1 + N2);

	// --- calc out1
	out1 = -in1 + N2 + N1;

	// --- deliver upstream
	if (getPort1_CompAdaptor())
		getPort1_CompAdaptor()->setInput2(out1);

	// --- set component state
	if (getPort3_CompAdaptor())
		getPort3_CompAdaptor()->setInput(N1);
}

/** set input 3 always connects to component */
void WdfParallelAdaptor::WdfParallelAdaptor::setInput3(double _in3)  { }

	/** get OUT1 = reflected output pin on Port 1 */
double WdfParallelAdaptor::getOutput1()  { return out1; }

/** get OUT2 = incident (normal) output pin on Port 2 */
double WdfParallelAdaptor::getOutput2()  { return out2; }

/** get OUT3 always connects to component */
double WdfParallelAdaptor::getOutput3()  { return out3; }

WdfParallelTerminatedAdaptor::WdfParallelTerminatedAdaptor() {}
WdfParallelTerminatedAdaptor::~WdfParallelTerminatedAdaptor() {}

/** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
double WdfParallelTerminatedAdaptor::getR2()
{
	double componentConductance = 0.0;
	if (getPort3_CompAdaptor())
		componentConductance = getPort3_CompAdaptor()->getComponentConductance();

	// --- 1 / (sum of admittances)
	R2 = 1.0 / ((1.0 / R1) + componentConductance);
	return R2;
}

/** initialize adaptor with input resistance */
void WdfParallelTerminatedAdaptor::initialize(double _R1) 
{
	// --- save R1
	R1 = _R1;

	double G1 = 1.0 / R1;
	if (terminalResistance <= 0.0)
		terminalResistance = 1e-15;

	double G2 = 1.0 / terminalResistance;
	double componentConductance = 0.0;
	if (getPort3_CompAdaptor())
		componentConductance = getPort3_CompAdaptor()->getComponentConductance();

	A1 = 2.0*G1 / (G1 + componentConductance + G2);
	A3 = openTerminalResistance ? 0.0 : 2.0*G2 / (G1 + componentConductance + G2);

	// --- init downstream
	if (getPort2_CompAdaptor())
		getPort2_CompAdaptor()->initialize(getR2());

	// --- not used in this implementation but saving for extended use
	R3 = 1.0 / componentConductance;
}

/** push audio input sample into incident wave input*/
void WdfParallelTerminatedAdaptor::setInput1(double _in1) 
{
	// --- save
	in1 = _in1;

	N2 = 0.0;
	if (getPort3_CompAdaptor())
		N2 = getPort3_CompAdaptor()->getOutput();

	// --- form N1
	N1 = -A1*(-in1 + N2) + N2 - A3*N2;

	// --- form output1
	out1 = -in1 + N2 + N1;

	// --- deliver upstream to input2
	if (getPort1_CompAdaptor())
		getPort1_CompAdaptor()->setInput2(out1);

	// --- calc out2 y(n)
	out2 = N2 + N1;

	// --- set component state
	if (getPort3_CompAdaptor())
		getPort3_CompAdaptor()->setInput(N1);
}

/** push audio input sample into reflected wave input; this is a dead end for terminated adaptorsthis is a dead end for terminated adaptors  */
void WdfParallelTerminatedAdaptor::setInput2(double _in2)  { in2 = _in2;}

/** set input 3 always connects to component */
void WdfParallelTerminatedAdaptor::setInput3(double _in3)  { }

/** get OUT1 = reflected output pin on Port 1 */
double WdfParallelTerminatedAdaptor::getOutput1()  { return out1; }

/** get OUT2 = incident (normal) output pin on Port 2 */
double WdfParallelTerminatedAdaptor::getOutput2()  { return out2; }

/** get OUT3 always connects to component */
double WdfParallelTerminatedAdaptor::getOutput3()  { return out3; }

WDFButterLPF3::WDFButterLPF3(void) { createWDF(); }	/* C-TOR */
WDFButterLPF3::~WDFButterLPF3(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFButterLPF3::reset(double _sampleRate)
{
	// --- rest WDF components (flush state registers)
	seriesAdaptor_L1.reset(_sampleRate);
	parallelAdaptor_C1.reset(_sampleRate);
	seriesTerminatedAdaptor_L2.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_L1.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFButterLPF3::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF ladder filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFButterLPF3::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_L1.setInput1(xn);

	// --- output is at terminated L2's output2
	return seriesTerminatedAdaptor_L2.getOutput2();
}

/** create the WDF structure for this object - may be called more than once */
void WDFButterLPF3::createWDF()
{
	// --- actual component values fc = 1kHz
	double L1_value = 95.49e-3;		// 95.5 mH
	double C1_value = 0.5305e-6;	// 0.53 uF
	double L2_value = 95.49e-3;		// 95.5 mH

	// --- set adapter components
	seriesAdaptor_L1.setComponent(wdfComponentType::L, L1_value);
	parallelAdaptor_C1.setComponent(wdfComponentType::C, C1_value);
	seriesTerminatedAdaptor_L2.setComponent(wdfComponentType::L, L2_value);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1, &parallelAdaptor_C1);
	WdfAdaptorBase::connectAdaptors(&parallelAdaptor_C1, &seriesTerminatedAdaptor_L2);

	// --- set source resistance
	seriesAdaptor_L1.setSourceResistance(600.0); // --- Rs = 600

	// --- set terminal resistance
	seriesTerminatedAdaptor_L2.setTerminalResistance(600.0); // --- Rload = 600
}

WDFTunableButterLPF3::WDFTunableButterLPF3(void) { createWDF(); }	/* C-TOR */
WDFTunableButterLPF3::~WDFTunableButterLPF3(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFTunableButterLPF3::reset(double _sampleRate)
{
	sampleRate = _sampleRate;
	// --- rest WDF components (flush state registers)
	seriesAdaptor_L1.reset(_sampleRate);
	parallelAdaptor_C1.reset(_sampleRate);
	seriesTerminatedAdaptor_L2.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_L1.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFTunableButterLPF3::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF ladder filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFTunableButterLPF3::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_L1.setInput1(xn);

	// --- output is at terminated L2's output2
	return seriesTerminatedAdaptor_L2.getOutput2();
}

/** create the filter structure; may be called more than once */
void WDFTunableButterLPF3::createWDF()
{
	// --- create components, init to noramlized values fc = 1Hz
	seriesAdaptor_L1.setComponent(wdfComponentType::L, L1_norm);
	parallelAdaptor_C1.setComponent(wdfComponentType::C, C1_norm);
	seriesTerminatedAdaptor_L2.setComponent(wdfComponentType::L, L2_norm);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1, &parallelAdaptor_C1);
	WdfAdaptorBase::connectAdaptors(&parallelAdaptor_C1, &seriesTerminatedAdaptor_L2);

	// --- set source resistance
	seriesAdaptor_L1.setSourceResistance(600.0); // --- Rs = 600

	// --- set terminal resistance
	seriesTerminatedAdaptor_L2.setTerminalResistance(600.0); // --- Rload = 600
}

/** parameter setter for warping */
void WDFTunableButterLPF3::setUsePostWarping(bool b) { useFrequencyWarping = b; }

bool WDFTunableButterLPF3::getUsePostWarping() { return useFrequencyWarping; }

/** parameter setter for fc */
void WDFTunableButterLPF3::setFilterFc(double fc_Hz)
{
	if (useFrequencyWarping)
	{
		double arg = (M_PI*fc_Hz) / sampleRate;
		fc_Hz = fc_Hz*(tan(arg) / arg);
	}

	seriesAdaptor_L1.setComponentValue(L1_norm / fc_Hz);
	parallelAdaptor_C1.setComponentValue(C1_norm / fc_Hz);
	seriesTerminatedAdaptor_L2.setComponentValue(L2_norm / fc_Hz);
	seriesAdaptor_L1.initializeAdaptorChain();
}

WDFBesselBSF3::WDFBesselBSF3(void) { createWDF(); }	/* C-TOR */
WDFBesselBSF3::~WDFBesselBSF3(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFBesselBSF3::reset(double _sampleRate)
{
	// --- rest WDF components (flush state registers)
	seriesAdaptor_L1C1.reset(_sampleRate);
	parallelAdaptor_L2C2.reset(_sampleRate);
	seriesTerminatedAdaptor_L3C3.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_L1C1.initializeAdaptorChain();

	return true;
}

/** return false: this object only processes samples */
bool WDFBesselBSF3::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF ladder filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFBesselBSF3::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_L1C1.setInput1(xn);

	// --- output is at terminated L2's output2
	return seriesTerminatedAdaptor_L3C3.getOutput2();
}

/** create the WDF structure; may be called more than once*/
void WDFBesselBSF3::createWDF()
{
	// --- set component values
	// --- fo = 5kHz
	//     BW = 2kHz or Q = 2.5
	seriesAdaptor_L1C1.setComponent(wdfComponentType::parallelLC, 16.8327e-3, 0.060193e-6);	/* L, C */
	parallelAdaptor_L2C2.setComponent(wdfComponentType::seriesLC, 49.1978e-3, 0.02059e-6);	/* L, C */
	seriesTerminatedAdaptor_L3C3.setComponent(wdfComponentType::parallelLC, 2.57755e-3, 0.393092e-6);	/* L, C */

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1C1, &parallelAdaptor_L2C2);
	WdfAdaptorBase::connectAdaptors(&parallelAdaptor_L2C2, &seriesTerminatedAdaptor_L3C3);

	// --- set source resistance
	seriesAdaptor_L1C1.setSourceResistance(600.0); // Ro = 600

	// --- set terminal resistance
	seriesTerminatedAdaptor_L3C3.setTerminalResistance(600.0);
}

WDFConstKBPF6::WDFConstKBPF6(void) { createWDF(); }	/* C-TOR */
WDFConstKBPF6::~WDFConstKBPF6(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFConstKBPF6::reset(double _sampleRate)
{
	// --- rest WDF components (flush state registers)
	seriesAdaptor_L1C1.reset(_sampleRate);
	parallelAdaptor_L2C2.reset(_sampleRate);

	seriesAdaptor_L3C3.reset(_sampleRate);
	parallelAdaptor_L4C4.reset(_sampleRate);

	seriesAdaptor_L5C5.reset(_sampleRate);
	parallelTerminatedAdaptor_L6C6.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_L1C1.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFConstKBPF6::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF ladder filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFConstKBPF6::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_L1C1.setInput1(xn);

	// --- output is at terminated L6C6 output2
	double output = parallelTerminatedAdaptor_L6C6.getOutput2();

	return output;
}

/** create the WDF structure */
void WDFConstKBPF6::createWDF()
{
	// --- fo = 5kHz
	//     BW = 2kHz or Q = 2.5
	seriesAdaptor_L1C1.setComponent(wdfComponentType::seriesLC, 47.7465e-3, 0.02122e-6);
	parallelAdaptor_L2C2.setComponent(wdfComponentType::parallelLC, 3.81972e-3, 0.265258e-6);

	seriesAdaptor_L3C3.setComponent(wdfComponentType::seriesLC, 95.493e-3, 0.01061e-6);
	parallelAdaptor_L4C4.setComponent(wdfComponentType::parallelLC, 3.81972e-3, 0.265258e-6);

	seriesAdaptor_L5C5.setComponent(wdfComponentType::seriesLC, 95.493e-3, 0.01061e-6);
	parallelTerminatedAdaptor_L6C6.setComponent(wdfComponentType::parallelLC, 7.63944e-3, 0.132629e-6);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1C1, &parallelAdaptor_L2C2);
	WdfAdaptorBase::connectAdaptors(&parallelAdaptor_L2C2, &seriesAdaptor_L3C3);
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L3C3, &parallelAdaptor_L4C4);
	WdfAdaptorBase::connectAdaptors(&parallelAdaptor_L4C4, &seriesAdaptor_L5C5);
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L5C5, &parallelTerminatedAdaptor_L6C6);

	// --- set source resistance
	seriesAdaptor_L1C1.setSourceResistance(600.0); // Ro = 600

	// --- set terminal resistance
	parallelTerminatedAdaptor_L6C6.setTerminalResistance(600.0);
}

WDFParameters::WDFParameters() {}
/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
/*WDFParameters::WDFParameters& operator=(const WDFParameters& params)
{
	if (this == &params)
		return *this;

	fc = params.fc;
	Q = params.Q;
	boostCut_dB = params.boostCut_dB;
	frequencyWarping = params.frequencyWarping;
	return *this;
}*/

WDFIdealRLCLPF::WDFIdealRLCLPF(void) { createWDF(); }	/* C-TOR */
WDFIdealRLCLPF::~WDFIdealRLCLPF(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFIdealRLCLPF::reset(double _sampleRate)
{
	sampleRate = _sampleRate;

	// --- rest WDF components (flush state registers)
	seriesAdaptor_RL.reset(_sampleRate);
	parallelTerminatedAdaptor_C.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_RL.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFIdealRLCLPF::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFIdealRLCLPF::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_RL.setInput1(xn);

	// --- output is at terminated L2's output2
	//     note compensation scaling by -6dB = 0.5
	//     because of WDF assumption about Rs and Rload
	return 0.5*parallelTerminatedAdaptor_C.getOutput2();
}

/** create the WDF structure; may be called more than once */
void WDFIdealRLCLPF::createWDF()
{
	// --- create components, init to noramlized values fc =
	//	   initial values for fc = 1kHz Q = 0.707
	//     Holding C Constant at 1e-6
	//			   L = 2.533e-2
	//			   R = 2.251131 e2
	seriesAdaptor_RL.setComponent(wdfComponentType::seriesRL, 2.251131e2, 2.533e-2);
	parallelTerminatedAdaptor_C.setComponent(wdfComponentType::C, 1.0e-6);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_RL, &parallelTerminatedAdaptor_C);

	// --- set source resistance
	seriesAdaptor_RL.setSourceResistance(0.0); // --- Rs = 600

	// --- set open ckt termination
	parallelTerminatedAdaptor_C.setOpenTerminalResistance(true);
}

/** get parameters: note use of custom structure for passing param data */
/**
\return WDFParameters custom data structure
*/
//WDFParameters getParameters() { return wdfParameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param WDFParameters custom data structure
*/
void WDFIdealRLCLPF::setParameters(const WDFParameters& _wdfParameters)
{
	if (_wdfParameters.fc != wdfParameters.fc ||
		_wdfParameters.Q != wdfParameters.Q ||
		_wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
		_wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
	{
		wdfParameters = _wdfParameters;
		double fc_Hz = wdfParameters.fc;

		if (wdfParameters.frequencyWarping)
		{
			double arg = (M_PI*fc_Hz) / sampleRate;
			fc_Hz = fc_Hz*(tan(arg) / arg);
		}

		double inductorValue = 1.0 / (1.0e-6 * pow((2.0*M_PI*fc_Hz), 2.0));
		double resistorValue = (1.0 / wdfParameters.Q)*(pow(inductorValue / 1.0e-6, 0.5));

		seriesAdaptor_RL.setComponentValue_RL(resistorValue, inductorValue);
		seriesAdaptor_RL.initializeAdaptorChain();
	}
}


WDFIdealRLCHPF::WDFIdealRLCHPF(void) { createWDF(); }	/* C-TOR */
WDFIdealRLCHPF::~WDFIdealRLCHPF(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFIdealRLCHPF::reset(double _sampleRate)
{
	sampleRate = _sampleRate;
	// --- rest WDF components (flush state registers)
	seriesAdaptor_RC.reset(_sampleRate);
	parallelTerminatedAdaptor_L.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_RC.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFIdealRLCHPF::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFIdealRLCHPF::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_RC.setInput1(xn);

	// --- output is at terminated L2's output2
	//     note compensation scaling by -6dB = 0.5
	//     because of WDF assumption about Rs and Rload
	return 0.5*parallelTerminatedAdaptor_L.getOutput2();
}

/** create WDF structure; may be called more than once */
void WDFIdealRLCHPF::createWDF()
{
	// --- create components, init to noramlized values fc =
	//	   initial values for fc = 1kHz Q = 0.707
	//     Holding C Constant at 1e-6
	//			   L = 2.533e-2
	//			   R = 2.251131 e2
	seriesAdaptor_RC.setComponent(wdfComponentType::seriesRC, 2.251131e2, 1.0e-6);
	parallelTerminatedAdaptor_L.setComponent(wdfComponentType::L, 2.533e-2);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_RC, &parallelTerminatedAdaptor_L);

	// --- set source resistance
	seriesAdaptor_RC.setSourceResistance(0.0); // --- Rs = 600

	// --- set open ckt termination
	parallelTerminatedAdaptor_L.setOpenTerminalResistance(true);
}

/** get parameters: note use of custom structure for passing param data */
/**
\return WDFParameters custom data structure
*/
//WDFParameters getParameters() { return wdfParameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param WDFParameters custom data structure
*/
void WDFIdealRLCHPF::setParameters(const WDFParameters& _wdfParameters)
{
	if (_wdfParameters.fc != wdfParameters.fc ||
		_wdfParameters.Q != wdfParameters.Q ||
		_wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
		_wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
	{
		wdfParameters = _wdfParameters;
		double fc_Hz = wdfParameters.fc;

		if (wdfParameters.frequencyWarping)
		{
			double arg = (M_PI*fc_Hz) / sampleRate;
			fc_Hz = fc_Hz*(tan(arg) / arg);
		}

		double inductorValue = 1.0 / (1.0e-6 * pow((2.0*M_PI*fc_Hz), 2.0));
		double resistorValue = (1.0 / wdfParameters.Q)*(pow(inductorValue / 1.0e-6, 0.5));

		seriesAdaptor_RC.setComponentValue_RC(resistorValue, 1.0e-6);
		parallelTerminatedAdaptor_L.setComponentValue(inductorValue);
		seriesAdaptor_RC.initializeAdaptorChain();
	}
}

WDFIdealRLCBPF::WDFIdealRLCBPF(void) { createWDF(); }	/* C-TOR */
WDFIdealRLCBPF::~WDFIdealRLCBPF(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFIdealRLCBPF::reset(double _sampleRate)
{
	sampleRate = _sampleRate;
	// --- rest WDF components (flush state registers)
	seriesAdaptor_LC.reset(_sampleRate);
	parallelTerminatedAdaptor_R.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_LC.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFIdealRLCBPF::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFIdealRLCBPF::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_LC.setInput1(xn);

	// --- output is at terminated L2's output2
	//     note compensation scaling by -6dB = 0.5
	//     because of WDF assumption about Rs and Rload
	return 0.5*parallelTerminatedAdaptor_R.getOutput2();
}

/** create the WDF structure*/
void WDFIdealRLCBPF::createWDF()
{
	// --- create components, init to noramlized values fc =
	//	   initial values for fc = 1kHz Q = 0.707
	//     Holding C Constant at 1e-6
	//			   L = 2.533e-2
	//			   R = 2.251131 e2
	seriesAdaptor_LC.setComponent(wdfComponentType::seriesLC, 2.533e-2, 1.0e-6);
	parallelTerminatedAdaptor_R.setComponent(wdfComponentType::R, 2.251131e2);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_LC, &parallelTerminatedAdaptor_R);

	// --- set source resistance
	seriesAdaptor_LC.setSourceResistance(0.0); // --- Rs = 600

	// --- set open ckt termination
	parallelTerminatedAdaptor_R.setOpenTerminalResistance(true);
}

/** get parameters: note use of custom structure for passing param data */
/**
\return WDFParameters custom data structure
*/
//WDFParameters getParameters() { return wdfParameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param WDFParameters custom data structure
*/
void WDFIdealRLCBPF::setParameters(const WDFParameters& _wdfParameters)
{
	if (_wdfParameters.fc != wdfParameters.fc ||
		_wdfParameters.Q != wdfParameters.Q ||
		_wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
		_wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
	{
		wdfParameters = _wdfParameters;
		double fc_Hz = wdfParameters.fc;

		if (wdfParameters.frequencyWarping)
		{
			double arg = (M_PI*fc_Hz) / sampleRate;
			fc_Hz = fc_Hz*(tan(arg) / arg);
		}

		double inductorValue = 1.0 / (1.0e-6 * pow((2.0*M_PI*fc_Hz), 2.0));
		double resistorValue = (1.0 / wdfParameters.Q)*(pow(inductorValue / 1.0e-6, 0.5));

		seriesAdaptor_LC.setComponentValue_LC(inductorValue, 1.0e-6);
		parallelTerminatedAdaptor_R.setComponentValue(resistorValue);
		seriesAdaptor_LC.initializeAdaptorChain();
	}
}

WDFIdealRLCBSF::WDFIdealRLCBSF(void) { createWDF(); }	/* C-TOR */
WDFIdealRLCBSF::~WDFIdealRLCBSF(void) {}	/* D-TOR */

/** reset members to initialized state */
bool WDFIdealRLCBSF::reset(double _sampleRate)
{
	sampleRate = _sampleRate;
	// --- rest WDF components (flush state registers)
	seriesAdaptor_R.reset(_sampleRate);
	parallelTerminatedAdaptor_LC.reset(_sampleRate);

	// --- intialize the chain of adapters
	seriesAdaptor_R.initializeAdaptorChain();
	return true;
}

/** return false: this object only processes samples */
bool WDFIdealRLCBSF::canProcessAudioFrame() { return false; }

/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
/**
\param xn input
\return the processed sample
*/
double WDFIdealRLCBSF::processAudioSample(double xn)
{
	// --- push audio sample into series L1
	seriesAdaptor_R.setInput1(xn);

	// --- output is at terminated L2's output2
	//     note compensation scaling by -6dB = 0.5
	//     because of WDF assumption about Rs and Rload
	return 0.5*parallelTerminatedAdaptor_LC.getOutput2();
}

/** create WDF structure */
void WDFIdealRLCBSF::createWDF()
{
	// --- create components, init to noramlized values fc =
	//	   initial values for fc = 1kHz Q = 0.707
	//     Holding C Constant at 1e-6
	//			   L = 2.533e-2
	//			   R = 2.251131 e2
	seriesAdaptor_R.setComponent(wdfComponentType::R, 2.533e-2);
	parallelTerminatedAdaptor_LC.setComponent(wdfComponentType::seriesLC, 2.533e-2, 1.0e-6);

	// --- connect adapters
	WdfAdaptorBase::connectAdaptors(&seriesAdaptor_R, &parallelTerminatedAdaptor_LC);

	// --- set source resistance
	seriesAdaptor_R.setSourceResistance(0.0); // --- Rs = 600

	// --- set open ckt termination
	parallelTerminatedAdaptor_LC.setOpenTerminalResistance(true);
}

/** get parameters: note use of custom structure for passing param data */
/**
\return WDFParameters custom data structure
*/
//WDFParameters getParameters() { return wdfParameters; }

/** set parameters: note use of custom structure for passing param data */
/**
\param WDFParameters custom data structure
*/
void WDFIdealRLCBSF::setParameters(const WDFParameters& _wdfParameters)
{
	if (_wdfParameters.fc != wdfParameters.fc ||
		_wdfParameters.Q != wdfParameters.Q ||
		_wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
		_wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
	{
		wdfParameters = _wdfParameters;
		double fc_Hz = wdfParameters.fc;

		if (wdfParameters.frequencyWarping)
		{
			double arg = (M_PI*fc_Hz) / sampleRate;
			fc_Hz = fc_Hz*(tan(arg) / arg);
		}

		double inductorValue = 1.0 / (1.0e-6 * pow((2.0*M_PI*fc_Hz), 2.0));
		double resistorValue = (1.0 / wdfParameters.Q)*(pow(inductorValue / 1.0e-6, 0.5));

		seriesAdaptor_R.setComponentValue(resistorValue);
		parallelTerminatedAdaptor_LC.setComponentValue_LC(inductorValue, 1.0e-6);
		seriesAdaptor_R.initializeAdaptorChain();
	}
}

