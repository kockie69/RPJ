# RPJ VCV plugins

## [Grendel](https://www.songfacts.com/facts/marillion/grendel)

![image](https://user-images.githubusercontent.com/2545165/127217988-c96fbf5f-4ab9-4919-becb-cefad05b025b.png)

Grendel is highly inspired by the [Batumi from XAOC](http://xaocdevices.com/main/batumi/). It is a fully voltage controlled, quadruple digital LFO module with a couple of nice features. Each oscillator can be used independently or in one of the 3 other modes: quad, phase and divide. For each oscillator, there are sine, square, saw, ramp and triangle waveform outputs available. Voltage control is possible for each oscillator’s frequency, phase and reset.

The four modes of Grendel can be selected by using the selector on the top. The colors of the 4 flashing leds will also indicate the selected mode, being red for FREE, green for QUAD, blue for PHASE and yellow for DIVIDE.

### FREE LFO MODE

When the FREE mode is selected, all the four LFO's will operate independently. By rotating the knob for each of the LFO's, you can control the frequency. 

### QUADRATURE LFO MODE

In this mode, the first LFO is the master controlling the frequency of the 3 other remaining LFO's. Each LFO generates a wave that is shifted 90 degrees in relation to the previous one. The knobs and the CV inputs control the amplitude of the signal for each of the 3 lfo's. The knob and the cv of the first LFO control its the frequency. 

### PHASE LFO MODE

The phase mode is similar to the quad mode except that you can control the amount of phase shift can be controlled using the knob for each LFO. Also the CV input can be used to control the phase shift.

### DIVIDE LFO MODE

In this mode the LFO's 2-4 are synced to LFO 1 with their frequencies occuring at divisions of the master. The knobs of the LFO's 2-4 define the ratio. This way frequencies 32 times slower than the master can be achieved. Again the CV input can be used to control and change the ratio.

### RESET INPUT SIGNAL

Each of the LFO's has a RESET input signal that can receive an incoming trigger impulse that resets the individual LFO to zero state (hard sync). 

## [Easter](https://www.songfacts.com/facts/marillion/easter)

![image](https://user-images.githubusercontent.com/2545165/127218106-64fa59c8-dddb-4db1-857e-a5c44ad5f3d9.png)

Easter is a resonance filter module that delivers 2 different resonators. One is a standard type and the other is a Smith-Angell resonator. You can choose the filters by clicking the + and - small buttons.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Lavender](https://en.wikipedia.org/wiki/Lavender_(Marillion_song))

![image](https://user-images.githubusercontent.com/2545165/127217771-0e96e853-8075-444d-a466-317f0895b2ef.png)

Lavender is a module that implements both a 2nd order LPF and HPF. It has a cv for controlling the Cutoff and Frequency and the Resonance. 

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [DryLand](https://www.marillion.com/music/lyric.htm?id=72)

![image](https://user-images.githubusercontent.com/2545165/127218199-ee2dbd8e-22ca-439a-b362-dd48daf559fa.png)

DryLand is a module that implements both a 1st order LPF and HPF. It has a cv for controlling the Cutoff Frequency.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [The Web](https://www.marillion.com/music/lyric.htm?id=12)

![image](https://user-images.githubusercontent.com/2545165/127219028-4e5a946b-6702-4a2c-8f6d-2d062c3052e8.png)

This module is a 2nd order Butterworth LPF, HPF, BPF and BSF.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Gazpacho](https://www.songfacts.com/facts/marillion/gazpacho)

![image](https://user-images.githubusercontent.com/2545165/127219112-12b4aaba-407f-4f19-8824-6dc62ebbc091.png)

Gazpacho is a 2nd order Linkwitz-Riley LPF and HPF. 

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Estonia](https://www.marillion.com/music/lyric.htm?id=110)

![image](https://user-images.githubusercontent.com/2545165/127219244-33ab71a0-a44e-4af0-9fde-6e1eee2b5c9f.png)

Estonia is a 1st order High and Low shelving filter.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Essence](https://www.marillion.com/music/lyric.htm?id=173)

![image](https://user-images.githubusercontent.com/2545165/127219358-aff97efc-2d02-429f-a674-fd4c83a067fe.png)

Essence, a new 2nd Order Parametric EQ Filter with a constant Q

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Brave](https://www.marillion.com/music/lyric.htm?id=91)

![image](https://user-images.githubusercontent.com/2545165/127219486-2280d8ea-07b0-4a5b-aef9-be3c22c9fe62.png)

Brave, a new 2nd Order Parametric EQ Filter with a Non constant Q

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Lady Nina](https://en.wikipedia.org/wiki/Lady_Nina)

![image](https://user-images.githubusercontent.com/2545165/127219589-584c9bae-919d-4f91-8bc9-f00cdbf51e9d.png)

This module is a 2nd Order State Variable VA Filter with some additional configuration options that can be accessed throught the menu.

### [Menu Options:]

#### [Enable Gain Compensation]
For every dB increase in peak gain due to increased Q, we lower the filter  response by half that value in dB.

#### [Enable NLP]
Option to enable Non Linear Processing.

#### [Enable Self Oscilation]


#### [Match Analog Nyquist LPF]
The LPF suffers from the zero-gain-at-Nyquist problem. This issue is addressed with a tweak to provide the correct gain at Nyquist: in this case, the gain at Nyquist exactly matches that of the analog equivalent, however, there is a small error in the peak magnitude at a combination of extremely high fc and Q values. Nevertheless the filters are stable.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Sugar Mice](https://en.wikipedia.org/wiki/Sugar_Mice)

![image](https://user-images.githubusercontent.com/2545165/127218347-a59eaf0e-fb35-40e4-8e80-f284c821b8eb.png)

This module is a 3rd order Butterworth low-pass ladder filter. 

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Montreal](https://www.marillion.com/music/lyric.htm?id=822)

![image](https://user-images.githubusercontent.com/2545165/127218426-1a3e075c-1cd7-4257-94a3-ce7779d6563f.png)

Montreal offers Ideal 2nd Order LPF, HPF, BPF and BSF RLC Filters.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [BlindCurve](https://www.marillion.com/music/lyric.htm?id=35)
Blindcurve is a simple Envelope follower. Yoe can change the detection to Peak, Mean Square or Root Mean Square.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [Gaza](https://www.marillion.com/music/lyric.htm?id=824)
Gaza is a more advanced Envelope Follower.

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.

## [CircularRide](https://www.marillion.com/music/lyric.htm?id=169)
CircularRide, a Stereo Delay with at the moment 2 delay algorithms (more to come)

The module is an implementation of the formula's and theory from the book [Designing Audio Effect Plugins in C++](https://www.amazon.co.uk/Designing-Software-Synthesizer-Plugins-Audio/dp/0367510464) from Will C. Pirkle.
