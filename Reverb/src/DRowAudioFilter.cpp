/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "includes.h"
#include "DRowAudioFilter.h"
#include "DRowAudioEditorComponent.h"


//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DRowAudioFilter();
}

//==============================================================================
DRowAudioFilter::DRowAudioFilter()
{
    zeromem (&lastPosInfo, sizeof (lastPosInfo));
    lastPosInfo.timeSigNumerator = 4;
    lastPosInfo.timeSigDenominator = 4;
    lastPosInfo.bpm = 120;
	
	params[DELTIME].init(parameterNames[DELTIME], UnitGeneric, String::empty,
						10, 0.001, 1000, 10);
	params[DELTIME].setSkewFactor(0.5);
//	params[DELTIME].setStep(0.001);
	
	params[ALLPASS].init(parameterNames[ALLPASS], UnitBoolean, String::empty,
						 0, 0, 1, 0);
	params[ALLPASS].setStep(1);
	
	params[FILTERCF].init(parameterNames[FILTERCF], UnitHertz, String::empty,
						  200, 50, 16000, 200);
	params[FILTERCF].setSkewFactor(0.5);
	
	params[DIFFUSION].setValue(0.7);
}

DRowAudioFilter::~DRowAudioFilter()
{
}

//==============================================================================
const String DRowAudioFilter::getName() const
{
    return "dRowAudio: Reverb";
}

int DRowAudioFilter::getNumParameters()
{
    return noParams;
}

float DRowAudioFilter::getParameter (int index)
{
	if (index >= 0 && index < noParams)
		return params[index].getNormalisedValue();
	else return 0.0f;
}

double DRowAudioFilter::getScaledParameter (int index)
{
	if (index >= 0 && index < noParams)
		return params[index].getValue();
	else return 0.0f;
}

void DRowAudioFilter::setParameter (int index, float newValue)
{
	for (int i = 0; i < noParams; i++)
	{
		if (index == i) {
			if (params[i].getNormalisedValue() != newValue) {
				params[i].setNormalisedValue(newValue);
				sendChangeMessage (this);
			}
		}
	}
	
	updateFilters();
}

void DRowAudioFilter::setScaledParameter (int index, float newValue)
{
	for (int i = 0; i < noParams; i++)
	{
		if (index == i) {
			if (params[i].getValue() != newValue) {
				params[i].setValue(newValue);
				sendChangeMessage (this);
			}
		}
	}
	
	updateFilters();
}

void DRowAudioFilter::setScaledParameterNotifyingHost(int index, float newValue)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			if (params[i].getValue() != newValue)
				setParameterNotifyingHost(index, params[i].normaliseValue(newValue));
}

const String DRowAudioFilter::getParameterName (int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return String(parameterNames[i]);	
	
    return String::empty;
}

const String DRowAudioFilter::getParameterText (int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return String(params[i].getValue(), 2);	
	
    return String::empty;
}

dRowParameter* DRowAudioFilter::getParameterPointer(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return &params[i];	
	
    return 0;
}
//=====================================================================
// methods for AU Compatibility
double DRowAudioFilter::getParameterMin(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return params[i].getMin();	
    return 0.0f;
}

double DRowAudioFilter::getParameterMax(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return params[i].getMax();	
    return 0.0f;
}

double DRowAudioFilter::getParameterDefault(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return params[i].getDefault();	
    return 0.0f;
}

ParameterUnit DRowAudioFilter::getParameterUnit(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return params[i].getUnit();	
    return (ParameterUnit)0;
}

double  DRowAudioFilter::getParameterStep(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return params[i].getStep();	
    return 0.0f;
}
double  DRowAudioFilter::getParameterSkewFactor(int index)
{
	for (int i = 0; i < noParams; i++)
		if (index == i)
			return params[i].getSkewFactor();	
    return 0.0f;
}

void DRowAudioFilter::smoothParameters()
{
	for (int i = 0; i < noParams; i++)
		params[i].smooth();
}
//=====================================================================

const String DRowAudioFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String DRowAudioFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool DRowAudioFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool DRowAudioFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool DRowAudioFilter::acceptsMidi() const
{
    return true;
}

bool DRowAudioFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void DRowAudioFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	currentSampleRate = sampleRate;

    // do your pre-playback setup stuff here..
    keyboardState.reset();
	
	updateFilters();
}

void DRowAudioFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void DRowAudioFilter::processBlock (AudioSampleBuffer& buffer,
									MidiBuffer& midiMessages)
{
	const int numInputChannels = getNumInputChannels();
	if (numInputChannels > 0)
	{
		int numSamples = buffer.getNumSamples();
		float* pfSample[numInputChannels];
		for (int channel = 0; channel < getNumInputChannels(); channel++)
			pfSample[channel] = buffer.getSampleData(channel, 0);
		
		
		smoothParameters();
		
		float gain = params[GAIN].getSmoothedNormalisedValue();
//		float ffCoeff = params[FFCOEFF].getSmoothedValue();
		float fbCoeff = params[FBCOEFF].getSmoothedValue();
		float delayTime = params[DELTIME].getSmoothedValue();
		float filterCf = params[FILTERCF].getSmoothedValue();
		float allpassCoeff = params[DIFFUSION].getSmoothedValue();

		combFilter1L.setGain(gain);			combFilter1R.setGain(gain);
		combFilter1L.setFBCoeff(fbCoeff);	combFilter1R.setFBCoeff(fbCoeff);
		combFilter1L.setDelayTime(currentSampleRate, delayTime);
		combFilter1R.setDelayTime(currentSampleRate, delayTime);
		combFilter1L.setLowpassCutoff(currentSampleRate, filterCf);
		combFilter1R.setLowpassCutoff(currentSampleRate, filterCf);

//		fbCoeff *= 0.9f;
		delayTime *= filterMult1;
		
		combFilter2L.setGain(gain);			combFilter2R.setGain(gain);
		combFilter2L.setFBCoeff(fbCoeff);	combFilter2R.setFBCoeff(fbCoeff);
		combFilter2L.setDelayTime(currentSampleRate, delayTime);
		combFilter2R.setDelayTime(currentSampleRate, delayTime);
		combFilter2L.setLowpassCutoff(currentSampleRate, filterCf);
		combFilter2R.setLowpassCutoff(currentSampleRate, filterCf);

//		fbCoeff *= 0.93f;
		delayTime *= filterMult2;

		combFilter3L.setGain(gain);			combFilter3R.setGain(gain);
		combFilter3L.setFBCoeff(fbCoeff);	combFilter3R.setFBCoeff(fbCoeff);
		combFilter3L.setDelayTime(currentSampleRate, delayTime);
		combFilter3R.setDelayTime(currentSampleRate, delayTime);
		combFilter3L.setLowpassCutoff(currentSampleRate, filterCf);
		combFilter3R.setLowpassCutoff(currentSampleRate, filterCf);

		delayTime *= filterMult3;
		
		combFilter4L.setGain(gain);			combFilter4R.setGain(gain);
		combFilter4L.setFBCoeff(fbCoeff);	combFilter4R.setFBCoeff(fbCoeff);
		combFilter4L.setDelayTime(currentSampleRate, delayTime);
		combFilter4R.setDelayTime(currentSampleRate, delayTime);
		combFilter4L.setLowpassCutoff(currentSampleRate, filterCf);
		combFilter4R.setLowpassCutoff(currentSampleRate, filterCf);
		
		// allpass section
		delayTime *= filterMult4;
		allpassFilter1L.setGain(allpassCoeff);		allpassFilter1R.setGain(allpassCoeff);
		allpassFilter1L.setDelayTime(currentSampleRate, delayTime);
		allpassFilter1R.setDelayTime(currentSampleRate, delayTime);
		
		delayTime *= filterMult5;
		allpassFilter2L.setGain(allpassCoeff);		allpassFilter2R.setGain(allpassCoeff);
		allpassFilter2L.setDelayTime(currentSampleRate, delayTime);
		allpassFilter2R.setDelayTime(currentSampleRate, delayTime);

		delayTime *= filterMult6;
		allpassFilter3L.setGain(allpassCoeff);		allpassFilter3R.setGain(allpassCoeff);
		allpassFilter3L.setDelayTime(currentSampleRate, delayTime);
		allpassFilter3R.setDelayTime(currentSampleRate, delayTime);
		
//		combFilter1L.processSamples(pfSample[0], numSamples);
//		combFilter1R.processSamples(pfSample[1], numSamples);
		
		//========================================================================
		while (--numSamples >= 0)
		{
			float inL = *pfSample[0] * 0.25f;
			float inR = *pfSample[1] * 0.25f;
			
			*pfSample[0] = combFilter1L.processSingleSample(inL)
							+ combFilter2L.processSingleSample(inL)
							+ combFilter3L.processSingleSample(inL)
							+ combFilter4L.processSingleSample(inL);
			
			*pfSample[1] = combFilter1R.processSingleSample(inR)
							+ combFilter2R.processSingleSample(inR)
							+ combFilter3R.processSingleSample(inR)
							+ combFilter4R.processSingleSample(inR);
			
			// allpass section for disperstion
			*pfSample[0] = allpassFilter1L.processSingleSample(*pfSample[0]);
			*pfSample[0] = allpassFilter2L.processSingleSample(*pfSample[0]);
			*pfSample[0] = allpassFilter3L.processSingleSample(*pfSample[0]);
			
			*pfSample[1] = allpassFilter1R.processSingleSample(*pfSample[1]);
			*pfSample[1] = allpassFilter2R.processSingleSample(*pfSample[1]);
			*pfSample[1] = allpassFilter3R.processSingleSample(*pfSample[1]);
			
			// process channels as interleaved
			for (int channel = 0; channel < numInputChannels; channel++)
			{
				// incriment sample pointers
				pfSample[channel]++;			
			}
		}
		//========================================================================	
		
	}

	
    // in case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    // if any midi messages come in, use them to update the keyboard state object. This
    // object sends notification to the UI component about key up/down changes
    keyboardState.processNextMidiBuffer (midiMessages,
                                         0, buffer.getNumSamples(),
                                         true);

    // have a go at getting the current time from the host, and if it's changed, tell
    // our UI to update itself.
    AudioPlayHead::CurrentPositionInfo pos;

    if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (pos))
    {
        if (memcmp (&pos, &lastPosInfo, sizeof (pos)) != 0)
        {
            lastPosInfo = pos;
            sendChangeMessage (this);
        }
    }
    else
    {
        zeromem (&lastPosInfo, sizeof (lastPosInfo));
        lastPosInfo.timeSigNumerator = 4;
        lastPosInfo.timeSigDenominator = 4;
        lastPosInfo.bpm = 120;
    }
}

//==============================================================================
AudioProcessorEditor* DRowAudioFilter::createEditor()
{
    return new DRowAudioEditorComponent (this);
}

//==============================================================================
void DRowAudioFilter::getStateInformation (MemoryBlock& destData)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
    XmlElement xmlState (T("MYPLUGINSETTINGS"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
//    xmlState.setAttribute (T("gainLevel"), gain);

    // you could also add as many child elements as you need to here..


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void DRowAudioFilter::setStateInformation (const void* data, int sizeInBytes)
{
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
        {
            // ok, now pull out our parameters..
//            gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);


            sendChangeMessage (this);
        }

        delete xmlState;
    }
}

void DRowAudioFilter::updateFilters()
{
	
}