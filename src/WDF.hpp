#include "rack.hpp"

struct IComponentAdaptor
{
public:
	/** initialize with source resistor R1 */
	virtual void initialize(double);

	/** initialize all downstream adaptors in the chain */
	virtual void initializeAdaptorChain();

	/** set input value into component port  */
	virtual void setInput(double);

	/** get output value from component port  */
	virtual double getOutput();

	// --- for adaptors
	/** ADAPTOR: set input port 1  */
	virtual void setInput1(double) = 0;

	/** ADAPTOR: set input port 2  */
	virtual void setInput2(double) = 0;

	/** ADAPTOR: set input port 3 */
	virtual void setInput3(double) = 0;

	/** ADAPTOR: get output port 1 value */
	virtual double getOutput1() = 0;

	/** ADAPTOR: get output port 2 value */
	virtual double getOutput2() = 0;

	/** ADAPTOR: get output port 3 value */
	virtual double getOutput3() = 0;

	/** reset the object with new sample rate */
	virtual void reset(double);

	/** get the commponent resistance from the attached object at Port3 */
	virtual double getComponentResistance();

	/** get the commponent conductance from the attached object at Port3 */
	virtual double getComponentConductance();

	/** update the commponent resistance at Port3 */
	virtual void updateComponentResistance();

	/** set an individual component value (may be R, L, or C */
	virtual void setComponentValue(double);

	/** set LC combined values */
	virtual void setComponentValue_LC(double, double);

	/** set RL combined values */
	virtual void setComponentValue_RL(double, double);

	/** set RC combined values */
	virtual void setComponentValue_RC(double, double);

	/** get a component value */
	virtual double getComponentValue();
};

struct WdfResistor : public IComponentAdaptor
{
public:
	WdfResistor(double);
	WdfResistor();
	virtual ~WdfResistor();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** get the component value */
	virtual double getComponentValue() override;

	/** set the component value */
	virtual void setComponentValue(double) override;

	/** change the resistance of component */
	virtual void updateComponentResistance() override;

	/** reset the component; clear registers */
	virtual void reset(double) override;

	/** set input value into component; NOTE: resistor is dead-end energy sink so this function does nothing */
	virtual void setInput(double) override;

	/** get output value; NOTE: a WDF resistor produces no reflected output */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double) override;;

	/** set input2 value; not used for components */
	virtual void setInput2(double) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double) override;

protected:
	double zRegister = 0.0;			///< storage register (not used with resistor)
	double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
	double componentResistance = 0.0;///< simulated resistance
	double sampleRate = 0.0;		///< sample rate
};

struct WdfCapacitor : public IComponentAdaptor
{
public:
	WdfCapacitor(double);
	WdfCapacitor();
	virtual ~WdfCapacitor();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** get the component value */
	virtual double getComponentValue() override;

	/** set the component value */
	virtual void setComponentValue(double) override;

	/** change the resistance of component */
	virtual void updateComponentResistance() override;

	/** reset the component; clear registers */
	virtual void reset(double) override;

	/** set input value into component; NOTE: capacitor sets value into register*/
	virtual void setInput(double in) override;

	/** get output value; NOTE: capacitor produces reflected output */
	virtual double getOutput() override;	// z^-1

	/** get output1 value; only one capacitor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one capacitor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one capacitor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double _in1) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double _in2) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double _in3) override;

protected:
	double zRegister = 0.0;			///< storage register (not used with resistor)
	double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
	double componentResistance = 0.0;///< simulated resistance
	double sampleRate = 0.0;		///< sample rate
};

struct WdfInductor : public IComponentAdaptor
{
public:
	WdfInductor(double);
	WdfInductor();
	virtual ~WdfInductor();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** get the component value */
	virtual double getComponentValue() override;

	/** set the component value */
	virtual void setComponentValue(double) override;

	/** change the resistance of component R(L) = 2Lfs */
	virtual void updateComponentResistance() override;

	/** reset the component; clear registers */
	virtual void reset(double) override;;

	/** set input value into component; NOTE: inductor sets value into storage register */
	virtual void setInput(double) override;

	/** get output value; NOTE: a WDF inductor produces reflected output that is inverted */
	virtual double getOutput() override; // -z^-1

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double) override;

protected:
	double zRegister = 0.0;			///< storage register (not used with resistor)
	double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
	double componentResistance = 0.0;///< simulated resistance
	double sampleRate = 0.0;		///< sample rate
};

struct WdfSeriesLC : public IComponentAdaptor
{
public:
	WdfSeriesLC();
	WdfSeriesLC(double, double);
	virtual ~WdfSeriesLC();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override;

	/** set both LC components at once */
	virtual void setComponentValue_LC(double, double) override;


	/** set L component */
	virtual void setComponentValue_L(double);

	/** set C component */
	virtual void setComponentValue_C(double);

	/** get L component value */
	virtual double getComponentValue_L();

	/** get C component value */
	virtual double getComponentValue_C();

	/** reset the component; clear registers */
	virtual void reset(double) override;

	/** set input value into component; NOTE: K is calculated here */
	virtual void setInput(double) override;

	/** get output value; NOTE: utput is located in zReg_L */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double) override;

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

struct WdfParallelLC : public IComponentAdaptor
{
public:
	WdfParallelLC();
	WdfParallelLC(double, double);
	virtual ~WdfParallelLC();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override;

	/** set both LC components at once */
	virtual void setComponentValue_LC(double, double) override;

	/** set L component */
	virtual void setComponentValue_L(double);

	/** set C component */
	virtual void setComponentValue_C(double);

	/** get L component value */
	virtual double getComponentValue_L();

	/** get C component value */
	virtual double getComponentValue_C();

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override;
	
    /** set input value into component; NOTE: K is calculated here */
	virtual void setInput(double) override;

	/** get output value; NOTE: output is located in -zReg_L */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double) override;

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

struct WdfSeriesRL : public IComponentAdaptor
{
public:
	WdfSeriesRL();
	WdfSeriesRL(double, double);

	virtual ~WdfSeriesRL();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override;

	/** set both RL components at once */
	virtual void setComponentValue_RL(double , double ) override;

	/** set L component */
	virtual void setComponentValue_L(double);

	/** set R component */
	virtual void setComponentValue_R(double);

	/** get L component value */
	virtual double getComponentValue_L();

	/** get R component value */
	virtual double getComponentValue_R();

	/** reset the component; clear registers */
	virtual void reset(double) override;

	/** set input value into component */
	virtual void setInput(double) override;

	/** get output value; NOTE: see FX book for details */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double) override;

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

struct WdfParallelRL : public IComponentAdaptor
{
public:
	WdfParallelRL();
	WdfParallelRL(double, double);
	virtual ~WdfParallelRL();

	/** set sample rate and update component */
	void setSampleRate(double);

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override;

	/** set both RL components at once */
	virtual void setComponentValue_RL(double , double ) override;

	/** set L component */
	virtual void setComponentValue_L(double );

	/** set R component */
	virtual void setComponentValue_R(double );

	/** get L component value */
	virtual double getComponentValue_L();

	/** get R component value */
	virtual double getComponentValue_R();

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override;

	/** set input value into component */
	virtual void setInput(double) override;

	/** get output value; NOTE: see FX book for details */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double ) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double ) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double ) override;

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

struct WdfSeriesRC : public IComponentAdaptor
{
public:
	WdfSeriesRC();
	WdfSeriesRC(double , double );
	virtual ~WdfSeriesRC();

	/** set sample rate and update component */
	void setSampleRate(double );

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override;

	/** set both RC components at once */
	virtual void setComponentValue_RC(double , double ) override;

	/** set R component */
	virtual void setComponentValue_R(double );

	/** set C component */
	virtual void setComponentValue_C(double );


	/** get R component value */
	virtual double getComponentValue_R();

	/** get C component value */
	virtual double getComponentValue_C();

	/** reset the component; clear registers */
	virtual void reset(double _sampleRate) override;

	/** set input value into component */
	virtual void setInput(double ) override;

	/** get output value; NOTE: see FX book for details */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double ) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double ) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double ) override;

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

struct WdfParallelRC : public IComponentAdaptor
{
public:
	WdfParallelRC();
	WdfParallelRC(double , double );
	virtual ~WdfParallelRC();

	/** set sample rate and update component */
	void setSampleRate(double );

	/** get component's value as a resistance */
	virtual double getComponentResistance() override;

	/** get component's value as a conducatance (or admittance) */
	virtual double getComponentConductance() override;

	/** change the resistance of component; see FX book for details */
	virtual void updateComponentResistance() override;

	/** set both RC components at once */
	virtual void setComponentValue_RC(double , double ) override;

	/** set R component */
	virtual void setComponentValue_R(double );

	/** set C component */
	virtual void setComponentValue_C(double );

	/** get R component value */
	virtual double getComponentValue_R();

	/** get C component value */
	virtual double getComponentValue_C();

	/** reset the component; clear registers */
	virtual void reset(double ) override;

	/** set input value into component; */
	virtual void setInput(double ) override;

	/** get output value; NOTE: output is located in zRegister_C */
	virtual double getOutput() override;

	/** get output1 value; only one resistor output (not used) */
	virtual double getOutput1() override;

	/** get output2 value; only one resistor output (not used) */
	virtual double getOutput2() override;

	/** get output3 value; only one resistor output (not used) */
	virtual double getOutput3() override;

	/** set input1 value; not used for components */
	virtual void setInput1(double ) override;

	/** set input2 value; not used for components */
	virtual void setInput2(double ) override;

	/** set input3 value; not used for components */
	virtual void setInput3(double ) override;

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

enum struct wdfComponentType { R, L, C, seriesLC, parallelLC, seriesRL, parallelRL, seriesRC, parallelRC };

struct WdfComponentInfo
{
	WdfComponentInfo();

	WdfComponentInfo(wdfComponentType , double , double);

	double R = 0.0; ///< value of R component
	double L = 0.0;	///< value of L component
	double C = 0.0;	///< value of C component
	wdfComponentType componentType = wdfComponentType::R; ///< selected component type
};

struct WdfAdaptorBase : public IComponentAdaptor
{
public:
	WdfAdaptorBase();
	virtual ~WdfAdaptorBase();

	/** set the termainal (load) resistance for terminating adaptors */
	void setTerminalResistance(double );

	/** set the termainal (load) resistance as open circuit for terminating adaptors */
	void setOpenTerminalResistance(bool);

	/** set the input (source) resistance for an input adaptor */
	void setSourceResistance(double );

	/** set the component or connected adaptor at port 1; functions is generic and allows extending the functionality of the WDF Library */
	void setPort1_CompAdaptor(IComponentAdaptor* );

	/** set the component or connected adaptor at port 2; functions is generic and allows extending the functionality of the WDF Library */
	void setPort2_CompAdaptor(IComponentAdaptor* );

	/** set the component or connected adaptor at port 3; functions is generic and allows extending the functionality of the WDF Library */
	void setPort3_CompAdaptor(IComponentAdaptor* );

	/** reset the connected component */
	virtual void reset(double ) override;

	/** creates a new WDF component and connects it to Port 3 */
	void setComponent(wdfComponentType, double, double);

	/** connect two adapters together upstreamAdaptor --> downstreamAdaptor */
	static void connectAdaptors(WdfAdaptorBase* , WdfAdaptorBase* );

	/** initialize the chain of adaptors from upstreamAdaptor --> downstreamAdaptor */
	virtual void initializeAdaptorChain() override;

	/** set value of single-component adaptor */
	virtual void setComponentValue(double ) override;

	/** set LC value of mjulti-component adaptor */
	virtual void setComponentValue_LC(double , double ) override;

	/** set RL value of mjulti-component adaptor */
	virtual void setComponentValue_RL(double , double ) override;

	/** set RC value of mjulti-component adaptor */
	virtual void setComponentValue_RC(double , double ) override;

	/** get adaptor connected at port 1: for extended functionality; not used in WDF ladder filter library */
	IComponentAdaptor* getPort1_CompAdaptor();

	/** get adaptor connected at port 2: for extended functionality; not used in WDF ladder filter library */
	IComponentAdaptor* getPort2_CompAdaptor();

	/** get adaptor connected at port 3: for extended functionality; not used in WDF ladder filter library */
	IComponentAdaptor* getPort3_CompAdaptor();

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

struct WdfSeriesAdaptor : public WdfAdaptorBase
{
public:
	WdfSeriesAdaptor();
	virtual ~WdfSeriesAdaptor();

	/** get the resistance at port 2; R2 = R1 + component (series)*/
	virtual double getR2();

	/** initialize adaptor with input resistance */
	virtual void initialize(double ) override;

	/** push audio input sample into incident wave input*/
	virtual void setInput1(double ) override;

	/** push audio input sample into reflected wave input */
	virtual void setInput2(double ) override;

	/** set input 3 always connects to component */
	virtual void setInput3(double ) override;

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override;

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override;

	/** get OUT3 always connects to component */
	virtual double getOutput3() override;

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double B = 0.0;		///< B coefficient value
};

struct WdfSeriesTerminatedAdaptor : public WdfAdaptorBase
{
public:
	WdfSeriesTerminatedAdaptor();
	virtual ~WdfSeriesTerminatedAdaptor();

	/** get the resistance at port 2; R2 = R1 + component (series)*/
	virtual double getR2();

	/** initialize adaptor with input resistance */
	virtual void initialize(double ) override;

	/** push audio input sample into reflected wave input
	for terminated adaptor, this is dead end, just store it */
	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	virtual void setInput1(double ) override;

	/** push audio input sample into reflected wave input
	    for terminated adaptor, this is dead end, just store it */
	virtual void setInput2(double ) override;

	/** set input 3 always connects to component */
	virtual void setInput3(double ) override;

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override;

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override;

	/** get OUT3 always connects to component */
	virtual double getOutput3() override;

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double B1 = 0.0;	///< B1 coefficient value
	double B3 = 0.0;	///< B3 coefficient value
};

struct WdfParallelAdaptor : public WdfAdaptorBase
{
public:
	WdfParallelAdaptor();
	virtual ~WdfParallelAdaptor();

	/** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
	virtual double getR2();

	/** initialize adaptor with input resistance */
	virtual void initialize(double ) override;

	/** push audio input sample into incident wave input*/
	virtual void setInput1(double ) override;

	/** push audio input sample into reflected wave input*/
	virtual void setInput2(double ) override;

	/** set input 3 always connects to component */
	virtual void setInput3(double ) override;

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override;

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override;

	/** get OUT3 always connects to component */
	virtual double getOutput3() override;

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double A = 0.0;		///< A coefficient value
};

struct WdfParallelTerminatedAdaptor : public WdfAdaptorBase
{
public:
	WdfParallelTerminatedAdaptor();
	virtual ~WdfParallelTerminatedAdaptor();

	/** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
	virtual double getR2();

	/** initialize adaptor with input resistance */
	virtual void initialize(double ) override;

	/** push audio input sample into incident wave input*/
	virtual void setInput1(double ) override;

	/** push audio input sample into reflected wave input; this is a dead end for terminated adaptorsthis is a dead end for terminated adaptors  */
	virtual void setInput2(double ) override;

	/** set input 3 always connects to component */
	virtual void setInput3(double ) override;

	/** get OUT1 = reflected output pin on Port 1 */
	virtual double getOutput1() override;

	/** get OUT2 = incident (normal) output pin on Port 2 */
	virtual double getOutput2() override;

	/** get OUT3 always connects to component */
	virtual double getOutput3() override;

private:
	double N1 = 0.0;	///< node 1 value, internal use only
	double N2 = 0.0;	///< node 2 value, internal use only
	double A1 = 0.0;	///< A1 coefficient value
	double A3 = 0.0;	///< A3 coefficient value
};

struct WDFButterLPF3
{
public:
	WDFButterLPF3(void);	/* C-TOR */
	~WDFButterLPF3(void);	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create the WDF structure for this object - may be called more than once */
	void createWDF();

protected:
	// --- three adapters
	WdfSeriesAdaptor seriesAdaptor_L1;			///< adaptor for L1
	WdfParallelAdaptor parallelAdaptor_C1;		///< adaptor for C1
	WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_L2;	///< adaptor for L2
};

struct WDFTunableButterLPF3
{
public:
	WDFTunableButterLPF3(void);	/* C-TOR */
	~WDFTunableButterLPF3(void);/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create the filter structure; may be called more than once */
	void createWDF();

	/** parameter setter for warping */
	void setUsePostWarping(bool );

	/** parameter setter for warping */
	bool getUsePostWarping();

	/** parameter setter for fc */
	void setFilterFc(double );

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

struct WDFBesselBSF3
{
public:
	WDFBesselBSF3(void);	/* C-TOR */
	~WDFBesselBSF3(void);	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create the WDF structure; may be called more than once*/
	void createWDF();

protected:
	// --- three adapters
	WdfSeriesAdaptor seriesAdaptor_L1C1;		///< adaptor for L1 and C1
	WdfParallelAdaptor parallelAdaptor_L2C2;	///< adaptor for L2 and C2
	WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_L3C3;	///< adaptor for L3 and C3
};


struct WDFConstKBPF6
{
public:
	WDFConstKBPF6(void);	/* C-TOR */
	~WDFConstKBPF6(void);/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create the WDF structure */
	void createWDF();

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
	WDFParameters();
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	//WDFParameters& operator=(const WDFParameters& );

	// --- individual parameters
	double fc = 100.0;				///< filter fc
	double Q = 0.707;				///< filter Q
	double boostCut_dB = 0.0;		///< filter boost or cut in dB
	bool frequencyWarping = true;	///< enable frequency warping
};

struct WDFIdealRLCLPF
{
public:
	WDFIdealRLCLPF(void);	/* C-TOR */
	~WDFIdealRLCLPF(void);	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create the WDF structure; may be called more than once */
	void createWDF();

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return WDFParameters custom data structure
	*/
	//WDFParameters getParameters() { return wdfParameters; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param WDFParameters custom data structure
	*/
	void setParameters(const WDFParameters& );

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_RL;				///< adaptor for series RL
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_C;	///< adaptopr for parallel C

	double sampleRate = 1.0;

};

struct WDFIdealRLCHPF
{
public:
	WDFIdealRLCHPF(void); /* C-TOR */
	~WDFIdealRLCHPF(void);	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create WDF structure; may be called more than once */
	void createWDF();

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return WDFParameters custom data structure
	*/
	//WDFParameters getParameters() { return wdfParameters; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param WDFParameters custom data structure
	*/
	void setParameters(const WDFParameters& );


protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- three
	WdfSeriesAdaptor				seriesAdaptor_RC;				///< adaptor for RC
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_L;	///< adaptor for L

	double sampleRate = 1.0;	///< sample rate storage
};

struct WDFIdealRLCBPF
{
public:
	WDFIdealRLCBPF(void);	/* C-TOR */
	~WDFIdealRLCBPF(void);	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create the WDF structure*/
	void createWDF();

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return WDFParameters custom data structure
	*/
	//WDFParameters getParameters() { return wdfParameters; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param WDFParameters custom data structure
	*/
	void setParameters(const WDFParameters& );

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_LC; ///< adaptor for LC
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_R; ///< adaptor for R

	double sampleRate = 1.0;
};

struct WDFIdealRLCBSF
{
public:
	WDFIdealRLCBSF(void);	/* C-TOR */
	~WDFIdealRLCBSF(void);/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double );

	/** create WDF structure */
	void createWDF();

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return WDFParameters custom data structure
	*/
	//WDFParameters getParameters() { return wdfParameters; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param WDFParameters custom data structure
	*/
	void setParameters(const WDFParameters& );

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_R; ///< adaptor for series R
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_LC; ///< adaptor for parallel LC

	double sampleRate = 1.0; ///< sample rate storage
};

