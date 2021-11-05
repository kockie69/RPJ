#include "ZVA.hpp"
#include "AudioDetector.hpp"

const double kMaxFilterFrequency = 20480.0; // 10 octaves above 20 Hz

/**
\struct EnvelopeFollowerParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the EnvelopeFollower object.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct EnvelopeFollowerParameters
{
	EnvelopeFollowerParameters();
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	//EnvelopeFollowerParameters& operator=(const EnvelopeFollowerParameters& params);

	// --- individual parameters
	double fc = 0.0;				///< filter fc
	double Q = 0.707;				///< filter Q
	double attackTime_mSec = 10.0;	///< detector attack time
	double releaseTime_mSec = 10.0;	///< detector release time
	double threshold_dB = 0.0;		///< detector threshold in dB
	double sensitivity = 1.0;		///< detector sensitivity
};

/**
\class EnvelopeFollower
\ingroup FX-Objects
\brief
The EnvelopeFollower object implements a traditional envelope follower effect modulating a LPR fc value
using the strength of the detected input.

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use EnvelopeFollowerParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct EnvelopeFollower
{
public:
	EnvelopeFollower();	/* C-TOR */
	~EnvelopeFollower();		/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double );

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return EnvelopeFollowerParameters custom data structure
	*/
	EnvelopeFollowerParameters getParameters();

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param EnvelopeFollowerParameters custom data structure
	*/
	void setParameters(const EnvelopeFollowerParameters& );

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame();

	/** process input x(n) through the envelope follower to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual rack::simd::float_4 processAudioSample(rack::simd::float_4 );


protected:
	EnvelopeFollowerParameters parameters; ///< object parameters

	// --- 1 filter and 1 detector
	ZVAFilter filter;		///< filter to modulate
	AudioDetector<rack::simd::float_4> detector; ///< detector to track input signal
};
