/*
==============================================================================

This file was auto-generated by the Jucer!

It contains the basic startup code for a Juce application.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter();

const float defaultWet = 1.0f;
const float defaultDry = 1.0f;
const float kfDefaultGain = 1.0f;
const float kfDefaultDelay = 0.0f;
const float kfDefaultDelayTime = 0.01f;
const float kfDefaultPan = 0.5f;
const float kfDefaultMidSide = 0.5f;
const int knMaxDelayBufferLength = 1200;


//==============================================================================
JuceDemoPluginAudioProcessor::JuceDemoPluginAudioProcessor()
    : delayBuffer(2, knMaxDelayBufferLength)
{
    // Set up some default values..
    wetGain = kfDefaultGain;
    delayAmount = kfDefaultDelay;
    delayTime = kfDefaultDelayTime;
    pan = kfDefaultPan;
    wetOn = defaultWet;
    dryOn = defaultDry;
    midSideAmount = kfDefaultMidSide;
    saturationAmount = 1.0f;
    roomSize = 0.5f;
    lastUIWidth = 400;
    lastUIHeight = 200;

    lastPosInfo.resetToDefault();
    delayPosition = 0;
    hpfFrequency = 0;
    /*m_ic = IIRCoefficients::makeHighPass (getSampleRate(), m_dFreq);
    m_filterL.setCoefficients( m_ic );
    m_filterR.setCoefficients( m_ic );*/

    // "1024" is the number of samples over which to fade parameter changes
    dspFilter = new Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::HighPass, 2>(1024);
    dspFilterParams[0] = 44100; // sample rate
    dspFilterParams[1] = 20; // cutoff frequency
    dspFilterParams[2] = 1.25; // Q
    dspFilter->setParams(dspFilterParams);

    reverbParams = reverb.getParameters();
    reverbParams.dryLevel = 0.0f;
}

JuceDemoPluginAudioProcessor::~JuceDemoPluginAudioProcessor()
{}

//==============================================================================
int JuceDemoPluginAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float JuceDemoPluginAudioProcessor::getParameter(int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case wetOnParam:
            return wetOn;
        case dryOnParam:
            return dryOn;
        case gainParam:
            return wetGain;
        case delayParam:
            return delayAmount;
        case delayTimeParam:
            return delayTime;
        case panParam:
            return pan;
        case midSideParam:
            return midSideAmount;
        case saturationAmountParam:
            return saturationAmount;
        case hpfParam:
            return hpfFrequency;
        case reverbSizeParam:
            return roomSize;
        default:
            return 0.0f;
    }
}

void JuceDemoPluginAudioProcessor::setParameter(int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case wetOnParam:
            wetOn = newValue;
            break;
        case dryOnParam:
            dryOn = newValue;
            break;
        case gainParam:
            wetGain = newValue;
            break;
        case delayParam:
            delayAmount = newValue;
            break;
        case delayTimeParam:
            delayTime = newValue;
            break;
        case panParam:
            pan = newValue;
            break;
        case midSideParam:
            midSideAmount = newValue;
            break;
        case saturationAmountParam:
            saturationAmount = newValue;
            break;
        case hpfParam:
            hpfFrequency = newValue;
            break;
        case reverbSizeParam:
            roomSize = newValue;
            break;
        default:
            break;
    }
}

float JuceDemoPluginAudioProcessor::getParameterDefaultValue(int index)
{
    switch (index)
    {
        case wetOnParam: return defaultWet;
        case dryOnParam: return defaultDry;
        case gainParam:     return kfDefaultGain;
        case delayParam:    return kfDefaultDelay;
        case delayTimeParam:    return kfDefaultDelayTime;
        case panParam:    return kfDefaultPan;
        case midSideParam:    return kfDefaultMidSide;
        case saturationAmountParam: return 1.0f;
        case hpfParam: return 0.0f;
        case reverbSizeParam: return 0.0f;
        default:            break;
    }

    return 0.0f;
}

const String JuceDemoPluginAudioProcessor::getParameterName(int index)
{
    switch (index)
    {
        case wetOnParam:		return "bypass";
        case dryOnParam:		return "dry on";
        case gainParam:			return "gain";
        case delayParam:		return "delay";
        case delayTimeParam:    return "delay time";
        case panParam:			return "pan";
        case midSideParam:		return "mid/side";
        case saturationAmountParam:	return "threshold";
        case hpfParam:			return "hpf";
        case reverbSizeParam: return "reverb size";
        default:				break;
    }

    return String::empty;
}

const String JuceDemoPluginAudioProcessor::getParameterText(int index)
{
    return String(getParameter(index), 2);
}

//==============================================================================
void JuceDemoPluginAudioProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    delayBuffer.clear();
}

void JuceDemoPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    //keyboardState.reset();
}

void JuceDemoPluginAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
    delayBuffer.clear();
    //m_Reverb.reset();
}

float JuceDemoPluginAudioProcessor::sigmoid(float x)
{
    if (fabs(x) < 1)
        return x*(1.5f - 0.5f*x*x);
    else
        return x > 0.f ? 1.f : -1.f;
}

float JuceDemoPluginAudioProcessor::distortion(float in, float threshold)
{
    if (in > threshold || in < -threshold)
    {
        in = (in + fabs(fabs(fmod(in - threshold, threshold * 4)) - threshold * 2) - threshold) / 2.0f;
    }
    return in;
}

float JuceDemoPluginAudioProcessor::Saturate(float x, float t)
{
    if (abs(x) < t)
        return x;
    else
    {
        if (x > 0.0f)
            return   t + (1.f - t)*sigmoid((x - t) / ((1 - t)*1.5f));
        else
            return -(t + (1.f - t)*sigmoid((-x - t) / ((1 - t)*1.5f)));
    }
}



void JuceDemoPluginAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    inputBuffer = buffer;
    if (wetOn == 0.0f || getNumInputChannels() < 2)
    {
        return;
    }

    const int numSamples = buffer.getNumSamples();
    int dp = 0;
    // Note we use the raw filter instead of the one
    // from the Design namespace.
    dspFilterParams[1] = hpfFrequency; // cutoff frequency
    dspFilter->setParams(dspFilterParams);
    dspFilter->process(numSamples, buffer.getArrayOfWritePointers());
    //return;
    //calculate left and right gain according to pan param
    float lGain = (pan > 0.5f) ? 1.0f - (pan - 0.5f) * 2.0f : 1.0f;
    float rGain = (pan < 0.5f) ? 1.0f - (0.5f - pan) * 2.0f : 1.0f;

    float fMidGain = (midSideAmount > 0.5f) ? 1.0f - (midSideAmount - 0.5f) * 2.0f : 1.0f;
    float fSideGain = (midSideAmount < 0.5f) ? 1.0f - (0.5f - midSideAmount) * 2.0f:1.0f;

    float* leftData = buffer.getWritePointer(0);
    float* leftInData = inputBuffer.getWritePointer(0);

    float* rightData = buffer.getWritePointer(1);
    float* rightInData = inputBuffer.getWritePointer(1);

    float* leftDelayData = delayBuffer.getWritePointer(0);
    float* rightDelayData = delayBuffer.getWritePointer(1);
    dp = delayPosition;
    /*m_ic = juce::IIRCoefficients::makeHighPass (getSampleRate(), m_dFreq);
    m_filterL.setCoefficients( m_ic );
    m_filterR.setCoefficients( m_ic );
    m_filterL.processSamples(leftData,numSamples);
    m_filterR.processSamples(rightData,numSamples);*/

    reverbParams.roomSize = roomSize;
    reverbParams.wetLevel = 1.0f; //this depends on general wet gain control
    reverb.setParameters(reverbParams);
    reverb.processStereo(leftData, rightData, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {

        float m = fMidGain*(leftData[i] + rightData[i]) / 2;
        float s = fSideGain*((rightData[i] - leftData[i]) / 2);

        leftData[i] = distortion((m - s)*lGain*wetGain, saturationAmount);
        rightData[i] = distortion((m + s)*rGain*wetGain, saturationAmount);

        const float inLeft = leftData[i];
        leftData[i] += leftDelayData[dp];
        leftDelayData[dp] = (inLeft)* delayAmount;

        const float inRight = rightData[i];
        rightData[i] += rightDelayData[dp];
        rightDelayData[dp] = (inRight)* delayAmount;

        if (dryOn == 1.0f)
        {
            leftData[i] += leftInData[i];
            rightData[i] += rightInData[i];
        }

        leftData[i] = clipper.clip(leftData[i], -1.0f, 1.0f);
        rightData[i] = clipper.clip(rightData[i], -1.0f, 1.0f);

        if (++dp >= static_cast<int>(opf.Process(delayTime) * knMaxDelayBufferLength))
            dp = 0;
    }
    delayPosition = dp;

    //	// In case we have more outputs than inputs, we'll clear any output
    //	// channels that didn't contain input data, (because these aren't
    //	// guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
AudioProcessorEditor* JuceDemoPluginAudioProcessor::createEditor()
{
    return new JuceDemoPluginAudioProcessorEditor(this);
}

//==============================================================================
void JuceDemoPluginAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute("uiWidth", lastUIWidth);
    xml.setAttribute("uiHeight", lastUIHeight);
    xml.setAttribute("wetGain", wetGain);
    xml.setAttribute("delayAmount", delayAmount);
    xml.setAttribute("delayTime", delayTime);
    xml.setAttribute("pan", pan);
    xml.setAttribute("dryOn", dryOn);
    xml.setAttribute("midSide", midSideAmount);
    xml.setAttribute("roomSize", roomSize);
    xml.setAttribute("hpfFreq", hpfFrequency);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary(xml, destData);
}

void JuceDemoPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            lastUIWidth = xmlState->getIntAttribute("uiWidth", lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute("uiHeight", lastUIHeight);
            wetGain = (float)xmlState->getDoubleAttribute("wetGain", wetGain);
            delayAmount = (float)xmlState->getDoubleAttribute("delayAmount", delayAmount);
            delayTime = (float)xmlState->getDoubleAttribute("delayTime", delayAmount);
            pan = (float)xmlState->getDoubleAttribute("pan", pan);
            dryOn = (float)xmlState->getDoubleAttribute("dryOn", dryOn);
            midSideAmount = (float)xmlState->getDoubleAttribute("midSide", midSideAmount);
            roomSize = (float)xmlState->getDoubleAttribute("roomSize", roomSize);
            hpfFrequency = (float)xmlState->getDoubleAttribute("hpfFreq", hpfFrequency);
        }
    }
}

const String JuceDemoPluginAudioProcessor::getInputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

const String JuceDemoPluginAudioProcessor::getOutputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

bool JuceDemoPluginAudioProcessor::isInputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool JuceDemoPluginAudioProcessor::isOutputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool JuceDemoPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool JuceDemoPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool JuceDemoPluginAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double JuceDemoPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JuceDemoPluginAudioProcessor();
}
