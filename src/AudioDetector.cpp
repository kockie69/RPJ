#include "AudioDetector.hpp"

/**
\struct AudioDetectorParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the AudioDetector object. NOTE: this object uses constant defintions:

- const unsigned int TLD_AUDIO_DETECT_MODE_PEAK = 0;
- const unsigned int TLD_AUDIO_DETECT_MODE_MS = 1;
- const unsigned int TLD_AUDIO_DETECT_MODE_RMS = 2;
- const double TLD_AUDIO_ENVELOPE_ANALOG_TC = -0.99967234081320612357829304641019; // ln(36.7%)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

AudioDetectorParameters::AudioDetectorParameters() {}













/** set our internal atack time coefficients based on times and sample rate */
//void setAttackTime(double attack_in_ms, bool forceCalc = false);

/** set our internal release time coefficients based on times and sample rate */
//void setReleaseTime(double release_in_ms, bool forceCalc = false);


