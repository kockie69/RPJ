#include "rack.hpp"

struct IComponentAdaptor
{
public:
	/** initialize with source resistor R1 */
	virtual void initialize(double _R1) {}

	/** initialize all downstream adaptors in the chain */
	virtual void initializeAdaptorChain() {}

	/** set input value into component port  */
	virtual void setInput(double _in) {}

	/** get output value from component port  */
	virtual double getOutput() { return 0.0; }

	// --- for adaptors
	/** ADAPTOR: set input port 1  */
	virtual void setInput1(double _in1) = 0;

	/** ADAPTOR: set input port 2  */
	virtual void setInput2(double _in2) = 0;

	/** ADAPTOR: set input port 3 */
	virtual void setInput3(double _in3) = 0;

	/** ADAPTOR: get output port 1 value */
	virtual double getOutput1() = 0;

	/** ADAPTOR: get output port 2 value */
	virtual double getOutput2() = 0;

	/** ADAPTOR: get output port 3 value */
	virtual double getOutput3() = 0;

	/** reset the object with new sample rate */
	virtual void reset(double _sampleRate) {}

	/** get the commponent resistance from the attached object at Port3 */
	virtual double getComponentResistance() { return 0.0; }

	/** get the commponent conductance from the attached object at Port3 */
	virtual double getComponentConductance() { return 0.0; }

	/** update the commponent resistance at Port3 */
	virtual void updateComponentResistance() {}

	/** set an individual component value (may be R, L, or C */
	virtual void setComponentValue(double _componentValue) { }

	/** set LC combined values */
	virtual void setComponentValue_LC(double componentValue_L, double componentValue_C) { }

	/** set RL combined values */
	virtual void setComponentValue_RL(double componentValue_R, double componentValue_L) { }

	/** set RC combined values */
	virtual void setComponentValue_RC(double componentValue_R, double componentValue_C) { }

	/** get a component value */
	virtual double getComponentValue() { return 0.0; }
};

struct WdfResistor : public IComponentAdaptor
{
public:
	WdfResistor(double _componentValue) { componentValue = _componentValue; }
	WdfResistor() { }
	virtual ~WdfResistor() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** get the component value */
	virtual double getComponentValue() override { return componentValue; }

	/** set the component value */
	virtual void setComponentValue(double _componentValue) override
	{
		componentValue = _componentValue;
		updateComponentResistance();
	}

	/** change the resistance of component */
	virtual void updateComponentResistance() override { componentResistance = componentValue; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate);  zRegister = 0.0; }

	/** set input value into component; NOTE: resistor is dead-end energy sink so this function does nothing */
	virtual void setInput(double in) override {}

	/** get output value; NOTE: a WDF resistor produces no reflected output */
	virtual double getOutput() override { return 0.0; }

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister = 0.0;			///< storage register (not used with resistor)
	double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
	double componentResistance = 0.0;///< simulated resistance
	double sampleRate = 0.0;		///< sample rate
};

class WdfCapacitor : public IComponentAdaptor
{
public:
	WdfCapacitor(double _componentValue) { componentValue = _componentValue; }
	WdfCapacitor() { }
	virtual ~WdfCapacitor() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** get the component value */
	virtual double getComponentValue() override { return componentValue; }

	/** set the component value */
	virtual void setComponentValue(double _componentValue) override
	{
		componentValue = _componentValue;
		updateComponentResistance();
	}

	/** change the resistance of component */
	virtual void updateComponentResistance() override
	{
		componentResistance = 1.0 / (2.0*componentValue*sampleRate);
	}

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister = 0.0; }

	/** set input value into component; NOTE: capacitor sets value into register*/
	virtual void setInput(double in) override { zRegister = in; }

	/** get output value; NOTE: capacitor produces reflected output */
	virtual double getOutput() override { return zRegister; }	// z^-1

	/** get output1 value; only one capacitor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one capacitor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one capacitor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister = 0.0;			///< storage register (not used with resistor)
	double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
	double componentResistance = 0.0;///< simulated resistance
	double sampleRate = 0.0;		///< sample rate
};

struct WdfInductor : public IComponentAdaptor
{
public:
	WdfInductor(double _componentValue) { componentValue = _componentValue; }
	WdfInductor() { }
	virtual ~WdfInductor() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** get the component value */
	virtual double getComponentValue() override { return componentValue; }

	/** set the component value */
	virtual void setComponentValue(double _componentValue) override
	{
		componentValue = _componentValue;
		updateComponentResistance();
	}

	/** change the resistance of component R(L) = 2Lfs */
	virtual void updateComponentResistance() override { componentResistance = 2.0*componentValue*sampleRate;}

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister = 0.0; }

	/** set input value into component; NOTE: inductor sets value into storage register */
	virtual void setInput(double in) override { zRegister = in; }

	/** get output value; NOTE: a WDF inductor produces reflected output that is inverted */
	virtual double getOutput() override { return -zRegister; } // -z^-1

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister = 0.0;			///< storage register (not used with resistor)
	double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
	double componentResistance = 0.0;///< simulated resistance
	double sampleRate = 0.0;		///< sample rate
};

class WdfSeriesLC : public IComponentAdaptor
{
public:
	WdfSeriesLC() {}
	WdfSeriesLC(double _componentValue_L, double _componentValue_C)
	{
		componentValue_L = _componentValue_L;
		componentValue_C = _componentValue_C;
	}
	virtual ~WdfSeriesLC() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override
	{
		RL = 2.0*componentValue_L*sampleRate;
		RC = 1.0 / (2.0*componentValue_C*sampleRate);
		componentResistance = RL + (1.0 / RC);
	}

	/** set both LC components at once */
	virtual void setComponentValue_LC(double _componentValue_L, double _componentValue_C) override
	{
		componentValue_L = _componentValue_L;
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** set L component */
	virtual void setComponentValue_L(double _componentValue_L)
	{
		componentValue_L = _componentValue_L;
		updateComponentResistance();
	}

	/** set C component */
	virtual void setComponentValue_C(double _componentValue_C)
	{
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** get L component value */
	virtual double getComponentValue_L() { return componentValue_L; }

	/** get C component value */
	virtual double getComponentValue_C() { return componentValue_C; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

	/** set input value into component; NOTE: K is calculated here */
	virtual void setInput(double in) override
	{
		double YC = 1.0 / RC;
		double K = (1.0 - RL*YC) / (1.0 + RL*YC);
		double N1 = K*(in - zRegister_L);
		zRegister_L = N1 + zRegister_C;
		zRegister_C = in;
	}

	/** get output value; NOTE: utput is located in zReg_L */
	virtual double getOutput() override { return zRegister_L; }

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister_L = 0.0; ///< storage register for L
	double zRegister_C = 0.0; ///< storage register for C

	double componentValue_L = 0.0; ///< component value L
	double componentValue_C = 0.0; ///< component value C

	double RL = 0.0; ///< RL value
	double RC = 0.0; ///< RC value
	double componentResistance = 0.0; ///< equivalent resistance of pair of components
	double sampleRate = 0.0; ///< sample rate
};

class WdfParallelLC : public IComponentAdaptor
{
public:
	WdfParallelLC() {}
	WdfParallelLC(double _componentValue_L, double _componentValue_C)
	{
		componentValue_L = _componentValue_L;
		componentValue_C = _componentValue_C;
	}
	virtual ~WdfParallelLC() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override
	{
		RL = 2.0*componentValue_L*sampleRate;
		RC = 1.0 / (2.0*componentValue_C*sampleRate);
		componentResistance = (RC + 1.0 / RL);
	}

	/** set both LC components at once */
	virtual void setComponentValue_LC(double _componentValue_L, double _componentValue_C) override
	{
		componentValue_L = _componentValue_L;
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** set L component */
	virtual void setComponentValue_L(double _componentValue_L)
	{
		componentValue_L = _componentValue_L;
		updateComponentResistance();
	}

	/** set C component */
	virtual void setComponentValue_C(double _componentValue_C)
	{
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** get L component value */
	virtual double getComponentValue_L() { return componentValue_L; }

	/** get C component value */
	virtual double getComponentValue_C() { return componentValue_C; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

	/** set input value into component; NOTE: K is calculated here */
	virtual void setInput(double in) override
	{
		double YL = 1.0 / RL;
		double K = (YL*RC - 1.0) / (YL*RC + 1.0);
		double N1 = K*(in - zRegister_L);
		zRegister_L = N1 + zRegister_C;
		zRegister_C = in;
	}

	/** get output value; NOTE: output is located in -zReg_L */
	virtual double getOutput() override { return -zRegister_L; }

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister_L = 0.0; ///< storage register for L
	double zRegister_C = 0.0; ///< storage register for C

	double componentValue_L = 0.0; ///< component value L
	double componentValue_C = 0.0; ///< component value C

	double RL = 0.0; ///< RL value
	double RC = 0.0; ///< RC value
	double componentResistance = 0.0; ///< equivalent resistance of pair of components
	double sampleRate = 0.0; ///< sample rate
};

class WdfSeriesRL : public IComponentAdaptor
{
public:
	WdfSeriesRL() {}
	WdfSeriesRL(double _componentValue_R, double _componentValue_L)
	{
		componentValue_L = _componentValue_L;
		componentValue_R = _componentValue_R;
	}
	virtual ~WdfSeriesRL() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override
	{
		RR = componentValue_R;
		RL = 2.0*componentValue_L*sampleRate;
		componentResistance = RR + RL;
		K = RR / componentResistance;
	}

	/** set both RL components at once */
	virtual void setComponentValue_RL(double _componentValue_R, double _componentValue_L) override
	{
		componentValue_L = _componentValue_L;
		componentValue_R = _componentValue_R;
		updateComponentResistance();
	}

	/** set L component */
	virtual void setComponentValue_L(double _componentValue_L)
    {
		componentValue_L = _componentValue_L;
		updateComponentResistance();
	}

	/** set R component */
	virtual void setComponentValue_R(double _componentValue_R)
	{
		componentValue_R = _componentValue_R;
		updateComponentResistance();
	}

	/** get L component value */
	virtual double getComponentValue_L() { return componentValue_L; }

	/** get R component value */
	virtual double getComponentValue_R() { return componentValue_R; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

	/** set input value into component */
	virtual void setInput(double in) override { zRegister_L = in; }

	/** get output value; NOTE: see FX book for details */
	virtual double getOutput() override
	{
		double NL = -zRegister_L;
		double out = NL*(1.0 - K) - K*zRegister_C;
		zRegister_C = out;

		return out;
	}

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister_L = 0.0; ///< storage register for L
	double zRegister_C = 0.0;///< storage register for C (not used)
	double K = 0.0;

	double componentValue_L = 0.0;///< component value L
	double componentValue_R = 0.0;///< component value R

	double RL = 0.0; ///< RL value
	double RC = 0.0; ///< RC value
	double RR = 0.0; ///< RR value

	double componentResistance = 0.0; ///< equivalent resistance of pair of componen
	double sampleRate = 0.0; ///< sample rate
};

class WdfParallelRL : public IComponentAdaptor
{
public:
	WdfParallelRL() {}
	WdfParallelRL(double _componentValue_R, double _componentValue_L)
	{
		componentValue_L = _componentValue_L;
		componentValue_R = _componentValue_R;
	}
	virtual ~WdfParallelRL() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override
	{
		RR = componentValue_R;
		RL = 2.0*componentValue_L*sampleRate;
		componentResistance = 1.0 / ((1.0 / RR) + (1.0 / RL));
		K = componentResistance / RR;
	}


	/** set both RL components at once */
	virtual void setComponentValue_RL(double _componentValue_R, double _componentValue_L) override
	{
		componentValue_L = _componentValue_L;
		componentValue_R = _componentValue_R;
		updateComponentResistance();
	}

	/** set L component */
	virtual void setComponentValue_L(double _componentValue_L)
	{
		componentValue_L = _componentValue_L;
		updateComponentResistance();
	}

	/** set R component */
	virtual void setComponentValue_R(double _componentValue_R)
	{
		componentValue_R = _componentValue_R;
		updateComponentResistance();
	}

	/** get L component value */
	virtual double getComponentValue_L() { return componentValue_L; }

	/** get R component value */
	virtual double getComponentValue_R() { return componentValue_R; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

	/** set input value into component */
	virtual void setInput(double in) override { zRegister_L = in; }

	/** get output value; NOTE: see FX book for details */
	virtual double getOutput() override
	{
		double NL = -zRegister_L;
		double out = NL*(1.0 - K) + K*zRegister_C;
		zRegister_C = out;
		return out;
	}

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister_L = 0.0;	///< storage register for L
	double zRegister_C = 0.0;	///< storage register for L
	double K = 0.0;				///< K value

	double componentValue_L = 0.0;	///< component value L
	double componentValue_R = 0.0;	///< component value R

	double RL = 0.0;	///< RL value
	double RC = 0.0;	///< RC value
	double RR = 0.0;	///< RR value

	double componentResistance = 0.0; ///< equivalent resistance of pair of components
	double sampleRate = 0.0; ///< sample rate
};

class WdfSeriesRC : public IComponentAdaptor
{
public:
	WdfSeriesRC() {}
	WdfSeriesRC(double _componentValue_R, double _componentValue_C)
	{
		componentValue_C = _componentValue_C;
		componentValue_R = _componentValue_R;
	}
	virtual ~WdfSeriesRC() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override
	{
		RR = componentValue_R;
		RC = 1.0 / (2.0*componentValue_C*sampleRate);
		componentResistance = RR + RC;
		K = RR / componentResistance;
	}

	/** set both RC components at once */
	virtual void setComponentValue_RC(double _componentValue_R, double _componentValue_C) override
	{
		componentValue_R = _componentValue_R;
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** set R component */
	virtual void setComponentValue_R(double _componentValue_R)
	{
		componentValue_R = _componentValue_R;
		updateComponentResistance();
	}

	/** set C component */
	virtual void setComponentValue_C(double _componentValue_C)
	{
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** get R component value */
	virtual double getComponentValue_R() { return componentValue_R; } 

	/** get C component value */
	virtual double getComponentValue_C() { return componentValue_C; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

	/** set input value into component */
	virtual void setInput(double in) override { zRegister_L = in; }

	/** get output value; NOTE: see FX book for details */
	virtual double getOutput() override
	{
		double NL = zRegister_L;
		double out = NL*(1.0 - K) + K*zRegister_C;
		zRegister_C = out;
		return out;
	}

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister_L = 0.0; ///< storage register for L
	double zRegister_C = 0.0; ///< storage register for C
	double K = 0.0;

	double componentValue_R = 0.0;///< component value R
	double componentValue_C = 0.0;///< component value C

	double RL = 0.0;	///< RL value
	double RC = 0.0;	///< RC value
	double RR = 0.0;	///< RR value

	double componentResistance = 0.0; ///< equivalent resistance of pair of components
	double sampleRate = 0.0; ///< sample rate
};

class WdfParallelRC : public IComponentAdaptor
{
public:
	WdfParallelRC() {}
	WdfParallelRC(double _componentValue_R, double _componentValue_C)
	{
		componentValue_C = _componentValue_C;
		componentValue_R = _componentValue_R;
	}
	virtual ~WdfParallelRC() {}

	/** set sample rate and update component */
	void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		updateComponentResistance();
	}

	/** get component's value as a resistance */
	virtual double getComponentResistance() override { return componentResistance; }

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override { return 1.0 / componentResistance; }

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override
	{
		RR = componentValue_R;
		RC = 1.0 / (2.0*componentValue_C*sampleRate);
		componentResistance = 1.0 / ((1.0 / RR) + (1.0 / RC));
		K = componentResistance / RR;
	}

	/** set both RC components at once */
	virtual void setComponentValue_RC(double _componentValue_R, double _componentValue_C) override
	{
		componentValue_R = _componentValue_R;
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** set R component */
	virtual void setComponentValue_R(double _componentValue_R)
	{
		componentValue_R = _componentValue_R;
		updateComponentResistance();
	}

	/** set C component */
	virtual void setComponentValue_C(double _componentValue_C)
	{
		componentValue_C = _componentValue_C;
		updateComponentResistance();
	}

	/** get R component value */
	virtual double getComponentValue_R() { return componentValue_R; }

	/** get C component value */
	virtual double getComponentValue_C() { return componentValue_C; }

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }

	/** set input value into component; */
	virtual void setInput(double in) override { zRegister_L = in; }

	/** get output value; NOTE: output is located in zRegister_C */
	virtual double getOutput() override
	{
		double NL = zRegister_L;
		double out = NL*(1.0 - K) - K*zRegister_C;
		zRegister_C = out;
		return out;
	}

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override { return  getOutput(); }

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override { return  getOutput(); }

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override { return  getOutput(); }

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override {}

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override {}

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override {}

protected:
	double zRegister_L = 0.0; ///< storage register for L
	double zRegister_C = 0.0; ///< storage register for C
	double K = 0.0;

	double componentValue_C = 0.0;	///< component value C
	double componentValue_R = 0.0;	///< component value R

	double RL = 0.0; ///< RL value
	double RC = 0.0; ///< RC value
	double RR = 0.0; ///< RR value

	double componentResistance = 0.0; ///< equivalent resistance of pair of components
	double sampleRate = 0.0; ///< sample rate
};

enum class wdfComponent { R, L, C, seriesLC, parallelLC, seriesRL, parallelRL, seriesRC, parallelRC };

struct WdfComponentInfo
{
	WdfComponentInfo() { }

	WdfComponentInfo(wdfComponent _componentType, double value1 = 0.0, double value2 = 0.0)
	{
		componentType = _componentType;
		if (componentType == wdfComponent::R)
			R = value1;
		else if (componentType == wdfComponent::L)
			L = value1;
		else if (componentType == wdfComponent::C)
			C = value1;
		else if (componentType == wdfComponent::seriesLC || componentType == wdfComponent::parallelLC)
		{
			L = value1;
			C = value2;
		}
		else if (componentType == wdfComponent::seriesRL || componentType == wdfComponent::parallelRL)
		{
			R = value1;
			L = value2;
		}
		else if (componentType == wdfComponent::seriesRC || componentType == wdfComponent::parallelRC)
		{
			R = value1;
			C = value2;
		}
	}

	double R = 0.0; ///< value of R component
	double L = 0.0;	///< value of L component
	double C = 0.0;	///< value of C component
	wdfComponent componentType = wdfComponent::R; ///< selected component type
};

class WdfAdaptorBase : public IComponentAdaptor
{
public:
	WdfAdaptorBase() {}
	virtual ~WdfAdaptorBase() {}

	/** set the termainal (load) resistance for terminating adaptors */
	void setTerminalResistance(double _terminalResistance) { terminalResistance = _terminalResistance; }

	/** set the termainal (load) resistance as open circuit for terminating adaptors */
	void setOpenTerminalResistance(bool _openTerminalResistance = true)
	{
		// --- flag overrides value
		openTerminalResistance = _openTerminalResistance;
		terminalResistance = 1.0e+34; // avoid /0.0
	}

	/** set the input (source) resistance for an input adaptor */
	void setSourceResistance(double _sourceResistance) { sourceResistance = _sourceResistance; }

	/** set the component or connected adaptor at port 1; functions is generic and allows extending the functionality of the WDF Library */
	void setPort1_CompAdaptor(IComponentAdaptor* _port1CompAdaptor) { port1CompAdaptor = _port1CompAdaptor; }

	/** set the component or connected adaptor at port 2; functions is generic and allows extending the functionality of the WDF Library */
	void setPort2_CompAdaptor(IComponentAdaptor* _port2CompAdaptor) { port2CompAdaptor = _port2CompAdaptor; }

	/** set the component or connected adaptor at port 3; functions is generic and allows extending the functionality of the WDF Library */
	void setPort3_CompAdaptor(IComponentAdaptor* _port3CompAdaptor) { port3CompAdaptor = _port3CompAdaptor; }

	/** reset the connected component */
	virtual void reset(double _sampleRate) override
	{
		if (wdfComponent)
			wdfComponent->reset(_sampleRate);
	}

	/** creates a new WDF component and connects it to Port 3 */
	void setComponent(wdfComponent componentType, double value1 = 0.0, double value2 = 0.0)
	{
		// --- decode and set
		if (componentType == wdfComponent::R)
		{
			wdfComponent = new WdfResistor;
			wdfComponent->setComponentValue(value1);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::L)
		{
			wdfComponent = new WdfInductor;
			wdfComponent->setComponentValue(value1);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::C)
		{
			wdfComponent = new WdfCapacitor;
			wdfComponent->setComponentValue(value1);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::seriesLC)
		{
			wdfComponent = new WdfSeriesLC;
			wdfComponent->setComponentValue_LC(value1, value2);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::parallelLC)
		{
			wdfComponent = new WdfParallelLC;
			wdfComponent->setComponentValue_LC(value1, value2);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::seriesRL)
		{
			wdfComponent = new WdfSeriesRL;
			wdfComponent->setComponentValue_RL(value1, value2);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::parallelRL)
		{
			wdfComponent = new WdfParallelRL;
			wdfComponent->setComponentValue_RL(value1, value2);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::seriesRC)
		{
			wdfComponent = new WdfSeriesRC;
			wdfComponent->setComponentValue_RC(value1, value2);
			port3CompAdaptor = wdfComponent;
		}
		else if (componentType == wdfComponent::parallelRC)
		{
			wdfComponent = new WdfParallelRC;
			wdfComponent->setComponentValue_RC(value1, value2);
			port3CompAdaptor = wdfComponent;
		}
	}

	/** connect two adapters together upstreamAdaptor --> downstreamAdaptor */
	static void connectAdaptors(WdfAdaptorBase* upstreamAdaptor, WdfAdaptorBase* downstreamAdaptor)
	{
		upstreamAdaptor->setPort2_CompAdaptor(downstreamAdaptor);
		downstreamAdaptor->setPort1_CompAdaptor(upstreamAdaptor);
	}

	/** initialize the chain of adaptors from upstreamAdaptor --> downstreamAdaptor */
	virtual void initializeAdaptorChain() override
	{
		initialize(sourceResistance);
	}

	/** set value of single-component adaptor */
	virtual void setComponentValue(double _componentValue) override
	{
		if (wdfComponent)
			wdfComponent->setComponentValue(_componentValue);
	}

	/** set LC value of mjulti-component adaptor */
	virtual void setComponentValue_LC(double componentValue_L, double componentValue_C) override
	{
		if (wdfComponent)
			wdfComponent->setComponentValue_LC(componentValue_L, componentValue_C);
	}

	/** set RL value of mjulti-component adaptor */
	virtual void setComponentValue_RL(double componentValue_R, double componentValue_L) override
	{
		if (wdfComponent)
			wdfComponent->setComponentValue_RL(componentValue_R, componentValue_L);
	}

	/** set RC value of mjulti-component adaptor */
	virtual void setComponentValue_RC(double componentValue_R, double componentValue_C) override
	{
		if (wdfComponent)
			wdfComponent->setComponentValue_RC(componentValue_R, componentValue_C);
	}

	/** get adaptor connected at port 1: for extended functionality; not used in WDF ladder filter library */
	IComponentAdaptor* getPort1_CompAdaptor() { return port1CompAdaptor; }

	/** get adaptor connected at port 2: for extended functionality; not used in WDF ladder filter library */
	IComponentAdaptor* getPort2_CompAdaptor() { return port2CompAdaptor; }

	/** get adaptor connected at port 3: for extended functionality; not used in WDF ladder filter library */
	IComponentAdaptor* getPort3_CompAdaptor() { return port3CompAdaptor; }

protected:
	// --- can in theory connect any port to a component OR adaptor;
	//     though this library is setup with a convention R3 = component
	IComponentAdaptor* port1CompAdaptor = nullptr;	///< componant or adaptor connected to port 1
	IComponentAdaptor* port2CompAdaptor = nullptr;	///< componant or adaptor connected to port 2
	IComponentAdaptor* port3CompAdaptor = nullptr;	///< componant or adaptor connected to port 3
	IComponentAdaptor* wdfComponent = nullptr;		///< WDF componant connected to port 3 (default operation)

	// --- These hold the input (R1), component (R3) and output (R2) resistances
	double R1 = 0.0; ///< input port resistance
	double R2 = 0.0; ///< output port resistance
	double R3 = 0.0; ///< component resistance

	// --- these are input variables that are stored;
	//     not used in this implementation but may be required for extended versions
	double in1 = 0.0;	///< stored port 1 input;  not used in this implementation but may be required for extended versions
	double in2 = 0.0;	///< stored port 2 input;  not used in this implementation but may be required for extended versions
	double in3 = 0.0;	///< stored port 3 input;  not used in this implementation but may be required for extended versions

	// --- these are output variables that are stored;
	//     currently out2 is the only one used as it is y(n) for this library
	//     out1 and out2 are stored; not used in this implementation but may be required for extended versions
	double out1 = 0.0;	///< stored port 1 output; not used in this implementation but may be required for extended versions
	double out2 = 0.0;	///< stored port 2 output; it is y(n) for this library
	double out3 = 0.0;	///< stored port 3 output; not used in this implementation but may be required for extended versions

	// --- terminal impedance
	double terminalResistance = 600.0; ///< value of terminal (load) resistance
	bool openTerminalResistance = false; ///< flag for open circuit load

	// --- source impedance, OK for this to be set to 0.0 for Rs = 0
	double sourceResistance = 600.0; ///< source impedance; OK for this to be set to 0.0 for Rs = 0
};

class WdfSeriesAdaptor : public WdfAdaptorBase
{
public:
	WdfSeriesAdaptor() {}
	virtual ~WdfSeriesAdaptor() {}

	/** get the resistance at port 2; R2 = R1 + component (series)*/
	virtual double getR2()
	{
		double componentResistance = 0.0;
		if (getPort3_CompAdaptor())
			componentResistance = getPort3_CompAdaptor()->getComponentResistance();

		R2 = R1 + componentResistance;
		return R2;
	}

	/** initialize adaptor with input resistance */
	virtual void initialize(double _R1) override
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
	virtual void setInput1(double _in1) override
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
	virtual void setInput2(double _in2) override
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
	virtual void setInput3(double _in3) override { }

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override { return out1; }

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override { return out2; }

	/** get OUT3 always connects to component */
	virtual double getOutput3() override { return out3; }

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double B = 0.0;		///< B coefficient value
};

class WdfSeriesTerminatedAdaptor : public WdfAdaptorBase
{
public:
	WdfSeriesTerminatedAdaptor() {}
	virtual ~WdfSeriesTerminatedAdaptor() {}

	/** get the resistance at port 2; R2 = R1 + component (series)*/
	virtual double getR2()
	{
		double componentResistance = 0.0;
		if (getPort3_CompAdaptor())
			componentResistance = getPort3_CompAdaptor()->getComponentResistance();

		R2 = R1 + componentResistance;
		return R2;
	}

	/** initialize adaptor with input resistance */
	virtual void initialize(double _R1) override
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
	virtual void setInput1(double _in1) override
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
	virtual void setInput2(double _in2) override { in2 = _in2;}

	/** set input 3 always connects to component */
	virtual void setInput3(double _in3) override { in3 = _in3;}

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override { return out1; }

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override { return out2; }

	/** get OUT3 always connects to component */
	virtual double getOutput3() override { return out3; }

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double B1 = 0.0;	///< B1 coefficient value
	double B3 = 0.0;	///< B3 coefficient value
};

class WdfParallelAdaptor : public WdfAdaptorBase
{
public:
	WdfParallelAdaptor() {}
	virtual ~WdfParallelAdaptor() {}

	/** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
	virtual double getR2()
	{
		double componentConductance = 0.0;
		if (getPort3_CompAdaptor())
			componentConductance = getPort3_CompAdaptor()->getComponentConductance();

		// --- 1 / (sum of admittances)
		R2 = 1.0 / ((1.0 / R1) + componentConductance);
		return R2;
	}

	/** initialize adaptor with input resistance */
	virtual void initialize(double _R1) override
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
	virtual void setInput1(double _in1) override
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
	virtual void setInput2(double _in2) override
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
	virtual void setInput3(double _in3) override { }

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override { return out1; }

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override { return out2; }

	/** get OUT3 always connects to component */
	virtual double getOutput3() override { return out3; }

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double A = 0.0;		///< A coefficient value
};

class WdfParallelTerminatedAdaptor : public WdfAdaptorBase
{
public:
	WdfParallelTerminatedAdaptor() {}
	virtual ~WdfParallelTerminatedAdaptor() {}

	/** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
	virtual double getR2()
	{
		double componentConductance = 0.0;
		if (getPort3_CompAdaptor())
			componentConductance = getPort3_CompAdaptor()->getComponentConductance();

		// --- 1 / (sum of admittances)
		R2 = 1.0 / ((1.0 / R1) + componentConductance);
		return R2;
	}

	/** initialize adaptor with input resistance */
	virtual void initialize(double _R1) override
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
	virtual void setInput1(double _in1) override
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
	virtual void setInput2(double _in2) override { in2 = _in2;}

	/** set input 3 always connects to component */
	virtual void setInput3(double _in3) override { }

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override { return out1; }

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override { return out2; }

	/** get OUT3 always connects to component */
	virtual double getOutput3() override { return out3; }

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double A1 = 0.0;	///< A1 coefficient value
	double A3 = 0.0;	///< A3 coefficient value
};

struct WDFButterLPF3
{
public:
	WDFButterLPF3(void) { createWDF(); }	/* C-TOR */
	~WDFButterLPF3(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_L1.setInput1(xn);

		// --- output is at terminated L2's output2
		return seriesTerminatedAdaptor_L2.getOutput2();
	}

	/** create the WDF structure for this object - may be called more than once */
	void createWDF()
	{
		// --- actual component values fc = 1kHz
		double L1_value = 95.49e-3;		// 95.5 mH
		double C1_value = 0.5305e-6;	// 0.53 uF
		double L2_value = 95.49e-3;		// 95.5 mH

										// --- set adapter components
		seriesAdaptor_L1.setComponent(wdfComponent::L, L1_value);
		parallelAdaptor_C1.setComponent(wdfComponent::C, C1_value);
		seriesTerminatedAdaptor_L2.setComponent(wdfComponent::L, L2_value);

		// --- connect adapters
		WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1, &parallelAdaptor_C1);
		WdfAdaptorBase::connectAdaptors(&parallelAdaptor_C1, &seriesTerminatedAdaptor_L2);

		// --- set source resistance
		seriesAdaptor_L1.setSourceResistance(600.0); // --- Rs = 600

		// --- set terminal resistance
		seriesTerminatedAdaptor_L2.setTerminalResistance(600.0); // --- Rload = 600
	}

protected:
	// --- three adapters
	WdfSeriesAdaptor seriesAdaptor_L1;			///< adaptor for L1
	WdfParallelAdaptor parallelAdaptor_C1;		///< adaptor for C1
	WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_L2;	///< adaptor for L2
};

class WDFTunableButterLPF3
{
public:
	WDFTunableButterLPF3(void) { createWDF(); }	/* C-TOR */
	~WDFTunableButterLPF3(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_L1.setInput1(xn);

		// --- output is at terminated L2's output2
		return seriesTerminatedAdaptor_L2.getOutput2();
	}

	/** create the filter structure; may be called more than once */
	void createWDF()
	{
		// --- create components, init to noramlized values fc = 1Hz
		seriesAdaptor_L1.setComponent(wdfComponent::L, L1_norm);
		parallelAdaptor_C1.setComponent(wdfComponent::C, C1_norm);
		seriesTerminatedAdaptor_L2.setComponent(wdfComponent::L, L2_norm);

		// --- connect adapters
		WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1, &parallelAdaptor_C1);
		WdfAdaptorBase::connectAdaptors(&parallelAdaptor_C1, &seriesTerminatedAdaptor_L2);

		// --- set source resistance
		seriesAdaptor_L1.setSourceResistance(600.0); // --- Rs = 600

		// --- set terminal resistance
		seriesTerminatedAdaptor_L2.setTerminalResistance(600.0); // --- Rload = 600
	}

	/** parameter setter for warping */
	void setUsePostWarping(bool b) { useFrequencyWarping = b; }

	/** parameter setter for fc */
	void setFilterFc(double fc_Hz)
	{
		if (useFrequencyWarping)
		{
			double arg = (M_PI*fc_Hz) / sampleRate;
			fc_Hz = fc_Hz*(tan(arg) / arg);
		}

		seriesAdaptor_L1.setComponentValue(L1_norm / fc_Hz);
		parallelAdaptor_C1.setComponentValue(C1_norm / fc_Hz);
		seriesTerminatedAdaptor_L2.setComponentValue(L2_norm / fc_Hz);
	}

protected:
	// --- three adapters
	WdfSeriesAdaptor seriesAdaptor_L1;		///< adaptor for L1
	WdfParallelAdaptor parallelAdaptor_C1;	///< adaptor for C1
	WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_L2;	///< adaptor for L2

	double L1_norm = 95.493;		// 95.5 mH
	double C1_norm = 530.516e-6;	// 0.53 uF
	double L2_norm = 95.493;		// 95.5 mH

	bool useFrequencyWarping = false;	///< flag for freq warping
	double sampleRate = 1.0;			///< stored sample rate
};

class WDFBesselBSF3
{
public:
	WDFBesselBSF3(void) { createWDF(); }	/* C-TOR */
	~WDFBesselBSF3(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_L1C1.setInput1(xn);

		// --- output is at terminated L2's output2
		return seriesTerminatedAdaptor_L3C3.getOutput2();
	}

	/** create the WDF structure; may be called more than once*/
	void createWDF()
	{
		// --- set component values
		// --- fo = 5kHz
		//     BW = 2kHz or Q = 2.5
		seriesAdaptor_L1C1.setComponent(wdfComponent::parallelLC, 16.8327e-3, 0.060193e-6);	/* L, C */
		parallelAdaptor_L2C2.setComponent(wdfComponent::seriesLC, 49.1978e-3, 0.02059e-6);	/* L, C */
		seriesTerminatedAdaptor_L3C3.setComponent(wdfComponent::parallelLC, 2.57755e-3, 0.393092e-6);	/* L, C */

		// --- connect adapters
		WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1C1, &parallelAdaptor_L2C2);
		WdfAdaptorBase::connectAdaptors(&parallelAdaptor_L2C2, &seriesTerminatedAdaptor_L3C3);

		// --- set source resistance
		seriesAdaptor_L1C1.setSourceResistance(600.0); // Ro = 600

		// --- set terminal resistance
		seriesTerminatedAdaptor_L3C3.setTerminalResistance(600.0);
	}

protected:
	// --- three adapters
	WdfSeriesAdaptor seriesAdaptor_L1C1;		///< adaptor for L1 and C1
	WdfParallelAdaptor parallelAdaptor_L2C2;	///< adaptor for L2 and C2
	WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_L3C3;	///< adaptor for L3 and C3
};

class WDFConstKBPF6
{
public:
	WDFConstKBPF6(void) { createWDF(); }	/* C-TOR */
	~WDFConstKBPF6(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_L1C1.setInput1(xn);

		// --- output is at terminated L6C6 output2
		double output = parallelTerminatedAdaptor_L6C6.getOutput2();

		return output;
	}

	/** create the WDF structure */
	void createWDF()
	{
		// --- fo = 5kHz
		//     BW = 2kHz or Q = 2.5
		seriesAdaptor_L1C1.setComponent(wdfComponent::seriesLC, 47.7465e-3, 0.02122e-6);
		parallelAdaptor_L2C2.setComponent(wdfComponent::parallelLC, 3.81972e-3, 0.265258e-6);

		seriesAdaptor_L3C3.setComponent(wdfComponent::seriesLC, 95.493e-3, 0.01061e-6);
		parallelAdaptor_L4C4.setComponent(wdfComponent::parallelLC, 3.81972e-3, 0.265258e-6);

		seriesAdaptor_L5C5.setComponent(wdfComponent::seriesLC, 95.493e-3, 0.01061e-6);
		parallelTerminatedAdaptor_L6C6.setComponent(wdfComponent::parallelLC, 7.63944e-3, 0.132629e-6);

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

protected:
	// --- six adapters
	WdfSeriesAdaptor seriesAdaptor_L1C1;		///< adaptor for L1 and C1
	WdfParallelAdaptor parallelAdaptor_L2C2;	///< adaptor for L2 and C2

	WdfSeriesAdaptor seriesAdaptor_L3C3;		///< adaptor for L3 and C3
	WdfParallelAdaptor parallelAdaptor_L4C4;	///< adaptor for L4 and C4

	WdfSeriesAdaptor seriesAdaptor_L5C5;		///< adaptor for L5 and C5
	WdfParallelTerminatedAdaptor parallelTerminatedAdaptor_L6C6;///< adaptor for L6 and C6
};

struct WDFParameters
{
	WDFParameters() {}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	WDFParameters& operator=(const WDFParameters& params)
	{
		if (this == &params)
			return *this;

		fc = params.fc;
		Q = params.Q;
		boostCut_dB = params.boostCut_dB;
		frequencyWarping = params.frequencyWarping;
		return *this;
	}

	// --- individual parameters
	double fc = 100.0;				///< filter fc
	double Q = 0.707;				///< filter Q
	double boostCut_dB = 0.0;		///< filter boost or cut in dB
	bool frequencyWarping = true;	///< enable frequency warping
};

class WDFIdealRLCLPF
{
public:
	WDFIdealRLCLPF(void) { createWDF(); }	/* C-TOR */
	~WDFIdealRLCLPF(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_RL.setInput1(xn);

		// --- output is at terminated L2's output2
		//     note compensation scaling by -6dB = 0.5
		//     because of WDF assumption about Rs and Rload
		return 0.5*parallelTerminatedAdaptor_C.getOutput2();
	}

	/** create the WDF structure; may be called more than once */
	void createWDF()
	{
		// --- create components, init to noramlized values fc =
		//	   initial values for fc = 1kHz Q = 0.707
		//     Holding C Constant at 1e-6
		//			   L = 2.533e-2
		//			   R = 2.251131 e2
		seriesAdaptor_RL.setComponent(wdfComponent::seriesRL, 2.251131e2, 2.533e-2);
		parallelTerminatedAdaptor_C.setComponent(wdfComponent::C, 1.0e-6);

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
	void setParameters(const WDFParameters& _wdfParameters)
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

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_RL;				///< adaptor for series RL
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_C;	///< adaptopr for parallel C

	double sampleRate = 1.0;

};

class WDFIdealRLCHPF
{
public:
	WDFIdealRLCHPF(void) { createWDF(); }	/* C-TOR */
	~WDFIdealRLCHPF(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_RC.setInput1(xn);

		// --- output is at terminated L2's output2
		//     note compensation scaling by -6dB = 0.5
		//     because of WDF assumption about Rs and Rload
		return 0.5*parallelTerminatedAdaptor_L.getOutput2();
	}

	/** create WDF structure; may be called more than once */
	void createWDF()
	{
		// --- create components, init to noramlized values fc =
		//	   initial values for fc = 1kHz Q = 0.707
		//     Holding C Constant at 1e-6
		//			   L = 2.533e-2
		//			   R = 2.251131 e2
		seriesAdaptor_RC.setComponent(wdfComponent::seriesRC, 2.251131e2, 1.0e-6);
		parallelTerminatedAdaptor_L.setComponent(wdfComponent::L, 2.533e-2);

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
	void setParameters(const WDFParameters& _wdfParameters)
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


protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- three
	WdfSeriesAdaptor				seriesAdaptor_RC;				///< adaptor for RC
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_L;	///< adaptor for L

	double sampleRate = 1.0;	///< sample rate storage
};

class WDFIdealRLCBPF
{
public:
	WDFIdealRLCBPF(void) { createWDF(); }	/* C-TOR */
	~WDFIdealRLCBPF(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_LC.setInput1(xn);

		// --- output is at terminated L2's output2
		//     note compensation scaling by -6dB = 0.5
		//     because of WDF assumption about Rs and Rload
		return 0.5*parallelTerminatedAdaptor_R.getOutput2();
	}

	/** create the WDF structure*/
	void createWDF()
	{
		// --- create components, init to noramlized values fc =
		//	   initial values for fc = 1kHz Q = 0.707
		//     Holding C Constant at 1e-6
		//			   L = 2.533e-2
		//			   R = 2.251131 e2
		seriesAdaptor_LC.setComponent(wdfComponent::seriesLC, 2.533e-2, 1.0e-6);
		parallelTerminatedAdaptor_R.setComponent(wdfComponent::R, 2.251131e2);

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
	void setParameters(const WDFParameters& _wdfParameters)
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

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_LC; ///< adaptor for LC
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_R; ///< adaptor for R

	double sampleRate = 1.0;
};

class WDFIdealRLCBSF
{
public:
	WDFIdealRLCBSF(void) { createWDF(); }	/* C-TOR */
	~WDFIdealRLCBSF(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
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
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_R.setInput1(xn);

		// --- output is at terminated L2's output2
		//     note compensation scaling by -6dB = 0.5
		//     because of WDF assumption about Rs and Rload
		return 0.5*parallelTerminatedAdaptor_LC.getOutput2();
	}

	/** create WDF structure */
	void createWDF()
	{
		// --- create components, init to noramlized values fc =
		//	   initial values for fc = 1kHz Q = 0.707
		//     Holding C Constant at 1e-6
		//			   L = 2.533e-2
		//			   R = 2.251131 e2
		seriesAdaptor_R.setComponent(wdfComponent::R, 2.533e-2);
		parallelTerminatedAdaptor_LC.setComponent(wdfComponent::seriesLC, 2.533e-2, 1.0e-6);

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
	void setParameters(const WDFParameters& _wdfParameters)
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

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_R; ///< adaptor for series R
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_LC; ///< adaptor for parallel LC

	double sampleRate = 1.0; ///< sample rate storage
};
