# Pigeon Plink / VCO <a name="pigeonplink"></a>
![pigeonplink image](./pigeonplink.png)

Pigeon Plink is an evolution of one of the smartest VCOs in VCV Rack: Kitchen Sink from Squinky Labs. It has inherited its sound sculpting capabilities while giving you a more modern approach to modulation.

## So, what's new?

The main feature of Pigeon Plink is the new Ratio Functionality.
While Kitchen Sink has fixed Ratios of 1 - 16 in whole steps in addition to an "Octave Selector", Pigeon Plink combines both into one "Ratio" with selectable "Steppings" or no stepping at all - and offers modulation of the Ratio too, polyphonic of course.
We had a look at a few vintage and modern Synths to offer Steppings that are usable and musical. You can select between the following Steppings:

### "None":
No Stepping at all, the Ratio can be set from 0 to 32.

### "Legacy":
This is the Stepping you will find on Kitchen Sink, 1 - 16 in whole steps.

### "Legacy+Sub":
Here we added the Ratios 0.125, 0.25, 0.5 to the Legacy Stepping for 3 more octaves down below.

### "Octaves":
Only Ratios that represent a full octave are available: 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0

### "Digitone":
The Elektron Digitone(TM) is one of the most popular modern FM-Synths and we recreated the steppings that are available on its Operator:
0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00, 2.25, 2.50, 2.75, 3.00, 3.25, 3.50, 3.75, 4.00, 4.25, 4.50, 4.75, 5.00, 5.50, 6.00, 6.50, 7.00, 7.50, 8.00, 8.50, 9.00, 9.50, 10.00, 11.00, 12.00, 13.00, 14.00, 15.00, 16.00
This is for the gnarly Stuff!

### "DX11":
This may be a Stepping that needs validation, because there is no list of available Ratios in the manuals of the famous Yamaha-Synths from yesteryears, but from a forum post somewhere on the interwebs we got this stepping selection for the more esoteric "squareroot of two"-FMSounds:
0.50, 0.71, 0.78, 0.87, 1.00, 1.41, 1.57, 1.73, 2.00, 2.82, 3.00, 3.14, 3.46, 4.00, 4.24, 4.71, 5.00, 5.19, 5.65, 6.00, 6.28, 6.92, 7.00, 7.07, 7.85, 8.00, 8.48, 8.65, 9.00, 9.42, 9.89, 10.00, 10.38, 10.99, 11.00, 11.30, 12.00, 12.11, 12.56, 12.72, 13.00, 13.84, 14.00, 14.10, 14.13, 15.00, 15.55, 15.57, 15.70, 16.96, 17.27, 17.30, 18.37, 18.84, 19.03, 19.78, 20.41, 20.76, 21.20, 21.98, 22.49, 23.55, 24.22, 25.95

### "DX7":
Well...

In addition to this Ratio Madness you can modulate it!
Think of changing timbres of an E-Piano sound, where a short attack "noise" is modulated somewhere between a Ratio of 10 and 20. Or a fat monophonic 4-Operator Lead Sound with ever changing overtones and harmonics - the Digitone Stepping is perfect for this. Or a Pad Sound, where the FM-Modulation shimmers in Octaves up.
The modulation input accepts bipolar modulation sources. The modulation signal is added to the knob position. You can set the Ratio Knob to Zero and use a unipolar Modulation from 0-10V to modulate through all Steppings, or set the Knob to 50% and use a bipolar source (-5V to +5V) to achieve the same effect. With careful attenuation of your modulation signal you can achieve very subtle changes. Use a Sample&Hold to get a different timbre each time a note is triggered or use an envelope to scan through the steppings.
This input is polyphonic, so you can set a different Ratio for each note in a chord for example. If you attach a monophonic modulation source while playing polyphonic the modulation will affect all channels. (This is true for most of the other modulation inputs too)
This leads to a very simple but beautiful aspect: You can build a monophonic lead sound with multiple operators with just one Pigeon Plink. Just duplicate your V/Oct to as many polyphonic channels as you want and then use channel 4 to FM channel 3, 3 to FM 2, 2 into 1, and 1 into the mixer. It doesn't get more CPU-friendly than that.

Oh, yes, "Envelope" you ask. There is none. We removed it, it was too convenient.
"Just stick two operators together and activate a few Envelope buttons, good to go".

There are so many envelopes, function generators and LFOs out there, which can give you more options than the good old ADSR, that we thought a VCA would make much more sense. And this is a special VCA, it is of course polyphonic, has a setting for linear or exponential response and the polarity of the incoming signal can be flipped. The VCA-Knob acts like a Bias, which opens the VCA from zero to full. The incoming modulation will add to that bias. And with the polarity switch set to "neg" and a fully open VCA you can achieve ducking sounds on a modulator for key-off sounds on a classic E-Piano-Patch.

## The Controls:

Ratio Knob (see above for detailed explanation):
A Ratio of 1.00 is always C4 (with 0V at V/Oct).
Modulation Input is Bipolar and adds to the Value of the Knob, which represents a 0-10V Range. That is why the same position on different Steppings results in different ratios. Input is polyphonic, but a monophonic modulation can be used, which is then affecting all channels.

### Wave Selection
Use the + and - Buttons to toggle through the different settings:
Let me quote the Kitchen Sink Manual on this one:
"In addition to the sine waveform, the wave-shaper in triangle mode can can morph from triangle to sawtooth, and the wave-folder setting generates classic wave-folder timbres reminiscent of the Buchla modules."

### Stepping Selection
Use the + and - Buttons to toggle through the different Steppings mentioned above.

VCA Knob (see above for detailed explanation):
Range 0-10V where 10V results in an audio output with VCV-Standard-Levels of +/-5V Maximums.
Lin/Exp Switch changes the response curve of the VCA from linear to exponential.
Pos/Neg inverts the polarity of an incoming modulation Signal.
Modulation Input is Bipolar and adds to the Value of the Knob. Input is polyphonic, but a monophonic modulation can be used, which is then affecting all channels.

### Fine Knob
Adusts the overall pitch of the VCO from -12 to +12 semitones.

### Depth Knob
Adjust the Linear-Through-Zero-FM Amount of a Modulator which is connected to the LFM-Input in the bottom row.
The Modulation Input is unipolar (0-10V) and modulates between zero and the set knob value. Input is polyphonic, but a monophonic modulation can be used, which is then affecting all channels.

### FM Knob
The FM Knob is an attenuator for the Modulation Input below.
The Modulation Input is bipolar and can be used to get a simple vibrato or to attach Pitch Envelopes when designing drum sounds. Input is polyphonic, but a monophonic modulation can be used, which is then affecting all channels.

### Fdbk Knob
The Feedback Knob sets the amount of the output signal which is feed back into its LFM-Circuit. This can give you a subtle change of timbre up to completely destroying the sound resulting in pure noise.
The Modulation Input is unipolar (0-10V) and modulates between zero and the set knob value. Input is polyphonic, but a monophonic modulation can be used, which is then affecting all channels.

### Shape Knob
Affects the base Waveform the VCO is generating.
This Knob has no effect when Wave is set to "Sine".
It controls the waveshaper amount when Wave is set to "Fold".
It smoothly blends from a triangle wave to a saw wave when Wave is set to "T/S"
The Modulation Input is unipolar (0-10V) and modulates between zero and the set knob value. Input is polyphonic, but a monophonic modulation can be used, which is then affecting all channels.

### LFM Input
Polyphonic Input for a Modulator, which is then used for Linear-Through-Zero-FM.

### V/Oct
Polyphonic Input for a Standard Pitch Signal

### Sync
Since Pigeon Plink is a very CPU-friendly VCO, you don't have to use it solely as an FM-Operator. With Sync you can get those classic synced Lead Sounds. Just attach another VCO to it, and Pigeon Plink will be synced in pitch to that.

### Output
Connect to a mixer or the LFM-Input of another Pigeon Plink or wherever you like - because it's VCV.

For a more indepth look at some of the controls, please also have a look at the [Kitchen Sink Manual](https://github.com/kockie69/SquinkyVCV-main/blob/master/docs/kitchen-sink.md)

For a discussion about Pigeon Plink and Patch Ideas head over to the official community forum

# Credits
**Squinky Labs:** Without him/them we never would have created this module. We used [Kitchen Sink](https://github.com/kockie69/SquinkyVCV-main/blob/master/docs/kitchen-sink.md) as the base for our ideas and also made heavily use of the original code.

**[Tana Barbier](https://community.vcvrack.com/u/tanabarbier/summary):** Thanks for your input on the specs and testing.

**[Markus](https://community.vcvrack.com/u/mosphaere/summary):** Thanks for raising the initial request, providing me the specs and ideas and your testing time.