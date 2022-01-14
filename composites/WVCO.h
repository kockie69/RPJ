
#pragma once

/**
 * 7/2 same number. if m=160 (instead of 16) : 80.8
 *  if n=40 (instead of 4) : 67
 * conclusion: no point in optimizing m path. could get a little from n path,
 * but not much.
 * 
 * optimize for no sync or fm: 84.7
 * add missing CV, clean up: 86.1
 * do shape calcs at reduced rate 87.4
 * 6/13 start to refactor 86.8% (gates at full rate). 
 * 6/4 use new sine approximation, down to 124%
 * 5/31: feature complete:
 *  8channels 148%
 
 * 
 * -finline-limit=500000  down to 144.7
 * down to 66 if step is gutted. so let's make an optimized step.
 * (removing adsr inner only took down to 64). SO, asside from inner loop, there must be a lot of bs to get rid of
 * instead of gutting step, just gutted oversampled part. that gives 97. so optimize at step level seems sensible.
 * however, minimal inner loop is still 136. same when I add flatten.
 * 
 * removing the optimzied sine takes back to 67, so the loop is gnarly
 * it's 110 with the filter removed, 82 with sine and filter removed
 * so - inner loops = 70 (40 filter, 30 sine)
 * 
 * 
 * 
 * 
 * 5/25 : 8 channels = 135%
 * 2,005,168 bytes in reduced plugin
 * 
 * down to 96% with re-written asserts
 * first try update envelopes audio rate: 136
 * 
 * 5/17 stock : 123.5
 * -march=native: 100 
 * -finline-limit=n: 103.5
 * -finline-limit=500000 -finline-functions-called-once: 103.6
 *  flatten on step() -finline-limit=500000 -finline-functions-called-once: 102
 *  -flto and all linlines
 * all the above: 77
 * 
 * gcc options to try:
 * 
 * -flto
 * (nc with ?) -fwhole-program
 * -finline-funciton
 * -march=native or skylake

 * static int max(int x, int y) __attribute__((always_inline));
static int max(int x, int y)
{

-finline-functions-called-once

--forceinline

__forceinline static int max(int x, int y)

-finline-limit=n

__attribute__ ((flatten))

conclusions, round 1:
march native is significant, but why? It's not generating avx, is it? (more to do)
inline options make a difference. unclear which to use, although inline-limit is easy
float -> float_4 isn't free.
 */

#include <assert.h>

#include <memory>
#include <vector>

//#ifndef _MSC_VER
#if 1
#include "ADSR16.h"
#include "../sqsrc/util/Divider.h"
#include "../composites/IComposite.h"
#include "../dsp/utils/LookupTable.h"
#include "../dsp/utils/ObjectCache.h"
#include "../dsp/generators/WVCODsp.h"

using float_4 = rack::simd::float_4;

#ifndef _CLAMP
#define _CLAMP
namespace std {
inline float clamp(float v, float lo, float hi) {
    assert(lo < hi);
    return std::min(hi, std::max(v, lo));
}
}  // namespace std
#endif

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack

using Module = ::rack::engine::Module;

template <class TBase>
class WVCODescription : public IComposite {
public:
    Config getParamValue(int i) override;
    int getNumParams() override;
};

class TriFormula {
public:
    static void getLeftA(float_4& outA, float_4 k) {
        simd_assertLT(k, float_4(1));
        simd_assertGT(k, float_4(0));
        outA = 1 / k;
    }
    static void getRightAandB(float_4& outA, float_4& outB, float_4 k) {
        outA = 1 / (k - 1);
        outB = -outA;
    }
};

template <class TBase>
class WVCO : public TBase {
public:
    int wfFromUI = 0;
    int steppingFromUI = 0;
    std::vector<std::vector<float>> R = {
    {},
    {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},
    {0.125,0.25, 0.5, 1.00,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},
    {0.125, 0.25, 0.5, 1.00, 2.00, 4.00, 8.00, 16.00, 32.00},
    {0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00, 2.25, 2.50, 2.75, 3.00, 3.25, 3.50, 3.75, 4.00, 4.25, 4.50, 4.75, 5.00, 5.50, 6.00, 6.50, 7.00, 7.50, 8.00, 8.50, 9.00, 9.50, 10.00, 11.00, 12.00, 13.00, 14.00, 15.00, 16.00},
    {0.50, 0.71, 0.78, 0.87, 1.00, 1.41, 1.57, 1.73, 2.00, 2.82, 3.00, 3.14, 3.46, 4.00, 4.24, 4.71, 5.00, 5.19, 5.65, 6.00, 6.28, 6.92, 7.00, 7.07, 7.85, 8.00, 8.48, 8.65, 9.00, 9.42, 9.89, 10.00, 10.38, 10.99, 11.00, 11.30, 12.00, 12.11, 12.56, 12.72, 13.00, 13.84, 14.00, 14.10, 14.13, 15.00, 15.55, 15.57, 15.70, 16.96, 17.27, 17.30, 18.37, 18.84, 19.03, 19.78, 20.41, 20.76, 21.20, 21.98, 22.49, 23.55, 24.22, 25.95}, 
    {0.50,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32}
    };
    std::vector<std::vector<float>> Av = {
    {},
    {0,0.55556,1.11111,1.66667,2.22222,2.77778,3.33333,3.88889,4.44444,5.00000,5.55556,6.11111,6.66667,7.22222,7.77778,8.33333,8.88889,9.44444,10.00000}
    };
    /*std::vector<std::vector<float>> Av2 = {
    {},
    {0.27778,0.83333,1.38889,1.94444,2.50000,3.05556,3.61111,4.16667,4.72222,5.27778,5.83333,6.38889,6.94444,7.50000,8.05556,8.61111,9.16667,9.72222}
    };*/
    WVCO(Module* module) : TBase(module) {
    }
    WVCO() : TBase() {
    }
    
    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds {
        VCA_PARAM,
        FREQUENCY_MULTIPLIER_PARAM,
        FINE_TUNE_PARAM,
        FM_DEPTH_PARAM,
        LINEAR_FM_DEPTH_PARAM,
        LINEAR_FM_PARAM,
        WAVESHAPE_GAIN_PARAM,
        WAVE_SHAPE_PARAM,
        FEEDBACK_PARAM,
        OUTPUT_LEVEL_PARAM,  // 8
        LINEXP_PARAM,
        POSINV_PARAM,
        PATCH_VERSION_PARAM,  // just for backwards compatibility with patch loading 19
        PARAM_STEPPING_DOWN,
        PARAM_STEPPING_UP,
        PARAM_WAVEFORM_DOWN,
        PARAM_WAVEFORM_UP,
        NUM_PARAMS
    };

    enum InputIds {
        VOCT_INPUT,
        FM_INPUT,
        LINEAR_FM_INPUT,
        GATE_INPUT,
        SYNC_INPUT,
        SHAPE_INPUT,
        LINEAR_FM_DEPTH_INPUT,
        FEEDBACK_INPUT,
        RATIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<WVCODescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
#ifndef _MSC_VER
    __attribute__((flatten))
#endif
    void
    step() override;

    static std::vector<std::string> getWaveformNames() {
        return {"Sine", "Wave folder", "Triangle<>Saw"};
    }

    static std::vector<std::string> getRatioNames(std::vector<float> R) {
        std::vector<std::string> v; 
        for (std::vector<float>::iterator it = R.begin() ; it != R.end(); ++it)
            v.push_back(std::to_string(*it));
        return v;
    }

    float convertOldShapeGain(float old) const;
    float getScaledRatio(std::vector<float>,float,float);
    float getRatio(int, float,float);

private:
    Divider divn;
    Divider divm;
    WVCODsp dsp[4];
    ADSR16 adsr;

    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaperLookupParams = ObjectCache<float>::getAudioTaper();

    // variables to stash processed knobs and other input
    // variables with _m suffix updated every m period
    float_4 basePitch_m = 0;  // all the knobs, no cv. units are volts
    int numChannels_m = 1;    // 1..16
    int numBanks_m = 1;       // 1..4
    float_4 depth_m;          // exp mode depth from knob
    float_4 baseFmDepth_m = 0;
    bool fmDepthConnected_m = false;
    bool feedbackConnected_m = false;
    bool fmInputConnected_m = false;
    bool syncInputConnected_m = false;
    bool ratioInputConnected_m = false;

    float_4 freqMultiplier_m = 1;
    float baseShapeGain = 0;  // 0..1 -> re-do this!
    float_4 baseFeedback_m = 0;
    float_4 baseOutputLevel_m = 1;  // 0..x
    float_4 baseOffset_m = 0;

    bool enableAdsrFeedback = false;
    bool enableAdsrFM = false;
    bool enableAdsrShape = false;
    dsp::SchmittTrigger waveFormUpTrigger,waveFormDownTrigger,steppingUpTrigger,steppingDownTrigger;
    float_4 getOscFreq(int bank);

    /**
     * This was originally a regular /4 stepn, but
     * I started doing it at X 1.
     * Now need to refactor
     */
    void stepn_fullRate();

    void stepn_lowerRate();

    /**
     * This is called "very infrequently"
     * Currently every 16 samples, but could go lower. for knobs and such
     */
    void stepm();

    /**
     * combine CV with knobs (basePitch_m),
     * send to DSPs
     */
    void updateFreq_n();
    void updateShapes_n();
};

template <class TBase>
inline void WVCO<TBase>::init() {
    adsr.setNumChannels(1);  // just to prime the pump, will write true value later
    divn.setup(4, [this]() {
        stepn_lowerRate();
    });
    divm.setup(16, [this]() {
        stepm();
    });
}

template <class TBase>
inline float WVCO<TBase>::getScaledRatio(std::vector<float> R,float ratio,float ratioCV) {

    float rDiv  = 10.f / (R.size()-1);
    float AV = rack::math::clamp(ratio + ratioCV, 0.f, 10.f);
    int ratioIndex = round(AV / rDiv);
    return R[ratioIndex];
}

template <class TBase>
inline float WVCO<TBase>::getRatio(int steppingType, float ratio, float ratioCV) {
    float ret;
    switch (steppingType) {
        case 0: //none
            ret = rack::math::clamp(ratio + ratioCV,0.f,10.f) * 3.2f;
            break;
        case 1: { //legacy
            ret = getScaledRatio(R[1], ratio, ratioCV);
            break;
        }
        case 2: // legacy+suboctaves
        {
            ret = getScaledRatio(R[2], ratio, ratioCV);
            break;
        }
        case 3: //octaves
        {
            ret = getScaledRatio(R[3], ratio, ratioCV);
            break;
        }
        case 4: //Digitone opperator
        {
            ret = getScaledRatio(R[4], ratio, ratioCV);
            break;
        }
        case 5: //Yamaha TX81Z
        {
            ret = getScaledRatio(R[5], ratio, ratioCV);
            break;
        }
        case 6: //Yamaha DX7:
        {
            ret = getScaledRatio(R[6], ratio, ratioCV);
            break;
        }
    }
    return ret;
}

template <class TBase>
inline float WVCO<TBase>::convertOldShapeGain(float old) const {
    std::function<double(double)> fi = AudioMath::makeFunc_InverseAudioTaper(-18);

    // run the current value through the inverse function to get the new value.
    // The new value, when turn through the audio taper, should yield the same value
    // as would be generated in 1.0 version.

    const float recoverdOldGain = old / 100;
    const float newParamValue = fi(recoverdOldGain) * 100;

    return newParamValue;
}

template <class TBase>
inline void WVCO<TBase>::stepm() {
    numChannels_m = std::max<int>(TBase::inputs[VOCT_INPUT].getChannels(),TBase::inputs[GATE_INPUT].getChannels());
    numChannels_m = std::max<int>(1,numChannels_m);
    WVCO<TBase>::outputs[WVCO<TBase>::MAIN_OUTPUT].setChannels(numChannels_m);
    
    numBanks_m = numChannels_m / 4;
    if (numChannels_m > numBanks_m * 4) {
        numBanks_m++;
    }

    // float basePitch = -4.0f + roundf(TBase::params[OCTAVE_PARAM].value) +
    //                  TBase::params[FINE_TUNE_PARAM].value / 12.0f;

    float basePitch = -4.0f + TBase::params[FINE_TUNE_PARAM].value / 12.0f;
    const float q = float(log2(261.626));  // move up to pitch range of EvenVCO
    basePitch += q;
    basePitch_m = float_4(basePitch);

    depth_m = .3f * LookupTable<float>::lookup(
                        *audioTaperLookupParams,
                        TBase::params[FM_DEPTH_PARAM].value * .01f);

    //freqMultiplier_m = float_4(std::round(TBase::params[FREQUENCY_MULTIPLIER_PARAM].value));
    Port& ratioInputPort = TBase::inputs[RATIO_INPUT];
    freqMultiplier_m = 2.f * getRatio(steppingFromUI, TBase::params[FREQUENCY_MULTIPLIER_PARAM].getValue(),ratioInputPort.getVoltage());
    //TBase::params[FREQUENCY_MULTIPLIER_PARAM].setDisplayValueString(std::to_string(freqMultiplier_m[0]));
    baseFmDepth_m = float_4(WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .003f);
    {
        Port& depthCVPort = WVCO<TBase>::inputs[LINEAR_FM_DEPTH_INPUT];
        fmDepthConnected_m = depthCVPort.isConnected();
    }
    {
        Port& feedbackPort = WVCO<TBase>::inputs[FEEDBACK_INPUT];
        feedbackConnected_m = feedbackPort.isConnected();
    }
    {
        Port& fmInputPort = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
        fmInputConnected_m = fmInputPort.isConnected();
    }
    {
        Port& syncInputPort = WVCO<TBase>::inputs[SYNC_INPUT];
        syncInputConnected_m = syncInputPort.isConnected();
    }
    {
        Port& ratioInputPort = WVCO<TBase>::inputs[RATIO_INPUT];
        ratioInputConnected_m = ratioInputPort.isConnected();
    }

    //int wfFromUI = (int)std::round(TBase::params[WAVE_SHAPE_PARAM].value);
    WVCODsp::WaveForm wf = WVCODsp::WaveForm(wfFromUI);
    baseShapeGain = TBase::params[WAVESHAPE_GAIN_PARAM].value / 100;

    // we want an nice taper for the wave folding depth.
    // regular linear taper is good for the tri<>saw morph.
    if (wf == WVCODsp::WaveForm::Fold) {
        baseShapeGain = LookupTable<float>::lookup(*audioTaperLookupParams, baseShapeGain, false);
    }

    const bool sync = TBase::inputs[SYNC_INPUT].isConnected();

    for (int bank = 0; bank < numBanks_m; ++bank) {
        dsp[bank].waveform = wf;
        dsp[bank].setSyncEnable(sync);
        dsp[bank].waveformOffset = baseOffset_m;
    }

    // these numbers here are just values found by experimenting - no math.
    baseFeedback_m = float_4(3.f * TBase::params[FEEDBACK_PARAM].value * 2.f / 1000.f);
    baseOutputLevel_m = float_4(TBase::params[OUTPUT_LEVEL_PARAM].value / 100.f);

    // Sine, Fold, SawTri
    // find the correct offset and gains to apply the waveformat
    // get thet them nomalized
    switch (wf) {
        case WVCODsp::WaveForm::Sine:
            baseOffset_m = 0;
            baseOutputLevel_m *= 5;
            break;
        case WVCODsp::WaveForm::Fold:
            baseOffset_m = 0;
            baseOutputLevel_m *= (5.f * 5.f / 5.6f);
            break;
        case WVCODsp::WaveForm::SawTri:
            baseOffset_m = -.5f;
            baseOutputLevel_m *= 10;
            break;
        default:
            assert(0);
    }
}

template <class TBase>
inline void WVCO<TBase>::updateFreq_n() {
    for (int bank = 0; bank < numBanks_m; ++bank) {
        float_4 freq = 0;

        const int baseChannel = 4 * bank;

        float_4 pitch = basePitch_m;
        // use SIMD here?
        Port& v8Port = TBase::inputs[VOCT_INPUT];
        pitch += v8Port.getPolyVoltageSimd<float_4>(baseChannel);

        Port& fmInputPort = TBase::inputs[FM_INPUT];
        pitch += fmInputPort.getPolyVoltage(baseChannel) * depth_m;
        for (int i = 0; i < 4; ++i) {
            freq[i] = expLookup(pitch[i]);
        }
        //RPJ: pitch * 16?
        freq *= freqMultiplier_m;
        float_4 time = rack::simd::clamp(freq * TBase::engineGetSampleTime(), -.5f, 0.5f);
        freq = time*16;

        dsp[bank].normalizedFreq = freq / WVCODsp::oversampleRate;
    }
}

template <class TBase>
inline void WVCO<TBase>::updateShapes_n() {
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;

        float_4 envMult = (enableAdsrShape) ? adsr.get(bank) : 1;
        simd_assertLE(envMult, float_4(2));
        simd_assertGE(envMult, float_4(0));

        float_4 baseGain = baseShapeGain;
        Port& port = TBase::inputs[SHAPE_INPUT];
        if (port.isConnected()) {
            baseGain *= port.getPolyVoltageSimd<float_4>(baseChannel);
            baseGain *= float_4(.1f);
            baseGain = rack::simd::clamp(baseGain, 0, 1);
        }
        float_4 correctedWaveShapeMultiplier = baseGain * envMult;
        switch (dsp[bank].waveform) {
            case WVCODsp::WaveForm::Sine:
                break;
            case WVCODsp::WaveForm::Fold:
                correctedWaveShapeMultiplier += float_4(.095f);
                correctedWaveShapeMultiplier *= 10;
                break;
            case WVCODsp::WaveForm::SawTri:
                correctedWaveShapeMultiplier = .5 + correctedWaveShapeMultiplier / 2;
                break;
        }
        dsp[bank].correctedWaveShapeMultiplier = correctedWaveShapeMultiplier;

        assertLE(baseShapeGain, 1);
        assertGE(baseShapeGain, 0);

        // we could spit this triangle calc it we aren't triangle,
        // but it doesn't take very long.

        // could to this at 'n' rate, and only it triangle
        // now let's compute triangle params
        if (dsp[bank].waveform == WVCODsp::WaveForm::SawTri) {
            const float_4 shapeGain = rack::simd::clamp(baseGain * envMult, .01f, .99f);
            simd_assertLT(shapeGain, float_4(2));
            simd_assertGT(shapeGain, float_4(0));

            float_4 k = .5 + shapeGain / 2;
            float_4 a, b;
            TriFormula::getLeftA(a, k);
            dsp[bank].aLeft = a;
            TriFormula::getRightAandB(a, b, k);

            dsp[bank].aRight = a;
            dsp[bank].bRight = b;
        }
    }
}

template <class TBase>
inline void WVCO<TBase>::stepn_lowerRate() {
    updateFreq_n();
    updateShapes_n();
}

#ifndef _MSC_VER
template <class TBase>
inline void __attribute__((flatten)) WVCO<TBase>::stepn_fullRate()
#else
template <class TBase>
inline void WVCO<TBase>::stepn_fullRate()
#endif
{
    assert(numBanks_m > 0);

    //--------------------------------------------------------------------
    // round up all the gates and run the ADSR;
    {
        // can do gates is lower rate (but it's no better)
        float_4 gates[4];
        for (int i = 0; i < 4; ++i) {
            Port& p = TBase::inputs[GATE_INPUT];
            float_4 g = p.getVoltageSimd<float_4>(i * 4);
            float_4 gate = (g > float_4(1));
            simd_assertMask(gate);
            gates[i] = gate;
        }

        adsr.step(gates, TBase::engineGetSampleTime());
    }

    // ----------------------------------------------------------------------------
    // now update all the DSP params.
    // This is the new, cleaner version.
    for (int bank = 0; bank < numBanks_m; ++bank) {
        float_4 feedbackAmount = baseFeedback_m;
        ;
        if (enableAdsrFeedback) {
           //feedbackAmount *= adsr.get(bank);
           Port& p = TBase::inputs[GATE_INPUT];
           feedbackAmount *= p.getVoltageSimd<float_4>(bank);
        }
        if (feedbackConnected_m) {
            Port& feedbackPort = WVCO<TBase>::inputs[FEEDBACK_INPUT];
            feedbackAmount *= feedbackPort.getPolyVoltageSimd<float_4>(bank * 4) * float_4(.1f);
            feedbackAmount = rack::simd::clamp(feedbackAmount, float_4(0), float_4(1));
        }
        dsp[bank].feedback = feedbackAmount;

        // TODO: add CV (use getNormalPolyVoltage)
        dsp[bank].outputLevel = baseOutputLevel_m;
        Port& p = TBase::inputs[GATE_INPUT];

        Param& pa_vca = TBase::params[VCA_PARAM];
        Param& pa_pi = TBase::params[POSINV_PARAM];
        
        if (pa_pi.getValue())
            dsp[bank].outputLevel = (pa_vca.getValue() * 10.f) + ((1 - pa_vca.getValue()) * p.getVoltageSimd<float_4>(bank));
        else
            dsp[bank].outputLevel = (pa_vca.getValue() * 10.f) - (pa_vca.getValue() * p.getVoltageSimd<float_4>(bank));
    }
}

template <class TBase>
#ifndef _MSC_VER
inline void __attribute__((flatten)) WVCO<TBase>::step()
#else
inline void WVCO<TBase>::step()
#endif
{
    // clock the sub-sample rate tasks
    divn.step();
    divm.step();
    
    stepn_fullRate();
    assert(numBanks_m > 0);

	if (waveFormUpTrigger.process(rescale(WVCO<TBase>::params[PARAM_WAVEFORM_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		if (wfFromUI+1 < 3)
			wfFromUI = wfFromUI + 1;

	if (waveFormDownTrigger.process(rescale(WVCO<TBase>::params[PARAM_WAVEFORM_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		if (wfFromUI - 1 >= 0)
			wfFromUI = wfFromUI - 1;
    
    if (steppingUpTrigger.process(rescale(WVCO<TBase>::params[PARAM_STEPPING_UP].getValue(), 1.f, 0.1f, 0.f, 1.f))) 
		if (steppingFromUI+1 < 7)
			steppingFromUI = steppingFromUI + 1;
    
	if (steppingDownTrigger.process(rescale(WVCO<TBase>::params[PARAM_STEPPING_DOWN].getValue(), 1.f, 0.1f, 0.f, 1.f)))
		if (steppingFromUI - 1 >= 0)
			steppingFromUI = steppingFromUI - 1;
    
    // this could even be moves out of the "every sample" loop
    if (!syncInputConnected_m && !fmInputConnected_m) {
        for (int bank = 0; bank < numBanks_m; ++bank) {
            const int baseChannel = 4 * bank;
            dsp[bank].fmInput = 0;
            float_4 v = dsp[bank].step(float_4::zero());
            WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, baseChannel);
        }
    } else {
        // TODO: don't do this if fm input port not connected. sync also
        for (int bank = 0; bank < numBanks_m; ++bank) {
            const int baseChannel = 4 * bank;
            Port& fmInputPort = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
            float_4 fmInput = fmInputPort.getPolyVoltageSimd<float_4>(baseChannel);

            // TODO:much of this could be done in stepn
            // TODO: add depth CV
            float_4 fmInputScaling = baseFmDepth_m;
            if (enableAdsrFM) {
                fmInputScaling *= adsr.get(bank);
            }
            if (fmDepthConnected_m) {
                Port& depthPort = WVCO<TBase>::inputs[LINEAR_FM_DEPTH_INPUT];
                fmInputScaling *= depthPort.getPolyVoltageSimd<float_4>(baseChannel) * float_4(.1f);
                fmInputScaling = rack::simd::clamp(fmInputScaling, float_4(0), float_4(1));
            }
            dsp[bank].fmInput = fmInput * fmInputScaling;

            Port& syncPort = WVCO<TBase>::inputs[SYNC_INPUT];
            const float_4 syncInput = syncPort.getPolyVoltageSimd<float_4>(baseChannel);
            float_4 v = dsp[bank].step(syncInput);
            WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, baseChannel);
        }
    }
}

template <class TBase>
inline int WVCODescription<TBase>::getNumParams() {
    return WVCO<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config WVCODescription<TBase>::getParamValue(int i) {
    Config ret(0, 1, 0, "");
    switch (i) {
        
        case WVCO<TBase>::PARAM_WAVEFORM_UP:
            ret = {.0f, 1.0f, 0.f, "Waveform Up"};
            break;
        case WVCO<TBase>::PARAM_WAVEFORM_DOWN:
            ret = {.0f, 1.0f, 0.f, "Waveform Down"};
            break;
        case WVCO<TBase>::PARAM_STEPPING_UP:
            ret = {.0f, 1.0f, 0.f, "Stepping Up"};
            break;
        case WVCO<TBase>::PARAM_STEPPING_DOWN:
            ret = {.0f, 1.0f, 0.f, "Stepping Down"};
            break;
        case WVCO<TBase>::VCA_PARAM:
            ret = {.0f, 1.0f, 0.5f, "VCA"};
            break;
        case WVCO<TBase>::FREQUENCY_MULTIPLIER_PARAM:
            ret = {0.f, 10.f, 0, "Frequency Ratio"};
            break;
        case WVCO<TBase>::FINE_TUNE_PARAM:
            ret = {-12.0f, 12.0f, 0, "Fine tune"};
            break;
        case WVCO<TBase>::FM_DEPTH_PARAM:
            ret = {.0f, 100.0f, 0, "Frequency modulation"};
            break;
        case WVCO<TBase>::LINEAR_FM_DEPTH_PARAM:
            ret = {0, 100, 0, "Through-zero FM Depth"};
            break;
        case WVCO<TBase>::LINEAR_FM_PARAM:
            ret = {0, 100, 0, "Linear FM Modulation"};
            break;
        case WVCO<TBase>::WAVESHAPE_GAIN_PARAM:
            ret = {0, 100, 0, "Shape modulation"};
            break;
        case WVCO<TBase>::WAVE_SHAPE_PARAM:
            ret = {0, 2, 0, "Wave shape"};
            break;
        case WVCO<TBase>::FEEDBACK_PARAM:
            ret = {0, 100, 0, "FM Feedback depth"};
            break;
        case WVCO<TBase>::LINEXP_PARAM:
            ret = {0, 1, 0, "Linear or Logarythmic"};
            break;
        case WVCO<TBase>::POSINV_PARAM:
            ret = {0, 1, 0, "Positive or Inverted"};
            break;  
        case WVCO<TBase>::OUTPUT_LEVEL_PARAM:
            ret = {0, 100, 100, "output Level"};
            break;
        case WVCO<TBase>::PATCH_VERSION_PARAM:
            ret = {0, 10, 0, "patch version"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif
