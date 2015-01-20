/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/
#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__

#include "../JuceLibraryCode/JuceHeader.h"
#include "Clipper.h"
#include "OnePoleFilter.h"
#include "DspFilters\Common.h"
#include "DspFilters\Filter.h"
#include "DspFilters\RBJ.h"
#include "DspFilters\Bessel.h"
#include "DspFilters\Butterworth.h"
#include "DspFilters\SmoothedFilter.h"

//==============================================================================
/**
    As the name suggest, this class does the actual audio processing.
*/

class JuceDemoPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    JuceDemoPluginAudioProcessor();
    ~JuceDemoPluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;
    void reset() override;

    //==============================================================================
    bool hasEditor() const override                  { return true; }
    AudioProcessorEditor* createEditor() override;

    //==============================================================================
    const String getName() const override            { return JucePlugin_Name; }

    int getNumParameters() override;
    float getParameter (int index) override;
    float getParameterDefaultValue (int index) override;
    void setParameter (int index, float newValue) override;
    const String getParameterName (int index) override;
    const String getParameterText (int index) override;

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override                                               { return 1; }
    int getCurrentProgram() override                                            { return 0; }
    void setCurrentProgram (int /*index*/) override                             {}
    const String getProgramName (int /*index*/) override                        { return "Default"; }
    void changeProgramName (int /*index*/, const String& /*newName*/) override  {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
	float Saturate(float input, float threshold);
	float sigmoid(float input);
	float distortion(float input, float threashold);
    //==============================================================================
    // These properties are public so that our editor component can access them
    // A bit of a hacky way to do it, but it's only a demo! Obviously in your own
    // code you'll do this much more neatly..

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth, lastUIHeight;

    //==============================================================================
    enum Parameters
    {
        wetOnParam = 0,
		dryOnParam,
        gainParam,
        delayParam,
		delayTimeParam,
		panParam,
        midSideParam,
		saturationAmountParam,
		hpfParam,
		reverbSizeParam,
        totalNumParams
	};
	
    float wetOn, dryOn, wetGain, delayAmount, pan,delayTime,midSideAmount,saturationAmount,hpfFrequency,roomSize;
	IIRFilter m_filterL,m_filterR;
	IIRCoefficients m_ic;
	juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
	Clipper clipper;
	OnePoleFilter opf;	
	
	 Dsp::Filter* dspFilter;
	 Dsp::Params dspFilterParams;
private:
    //==============================================================================
    AudioSampleBuffer delayBuffer;
	AudioSampleBuffer inputBuffer;
    int delayPosition;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
