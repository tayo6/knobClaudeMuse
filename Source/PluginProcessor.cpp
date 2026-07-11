#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GradientKnobAudioProcessor::GradientKnobAudioProcessor()
     : AudioProcessor (BusesProperties()
                       #if ! JucePlugin_IsMidiEffect
                        #if ! JucePlugin_IsSynth
                         .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        #endif
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       #endif
                         ),
       apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

GradientKnobAudioProcessor::~GradientKnobAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout GradientKnobAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "value", 1 },
        "Value",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f),
        50.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String GradientKnobAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GradientKnobAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GradientKnobAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GradientKnobAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GradientKnobAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GradientKnobAudioProcessor::getNumPrograms()
{
    return 1;
}

int GradientKnobAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GradientKnobAudioProcessor::setCurrentProgram (int)
{
}

const juce::String GradientKnobAudioProcessor::getProgramName (int)
{
    return {};
}

void GradientKnobAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void GradientKnobAudioProcessor::prepareToPlay (double, int)
{
}

void GradientKnobAudioProcessor::releaseResources()
{
}

bool GradientKnobAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
   #endif
}

void GradientKnobAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Pass-through: this plugin's purpose is the UI control, not DSP.
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

//==============================================================================
bool GradientKnobAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GradientKnobAudioProcessor::createEditor()
{
    return new GradientKnobAudioProcessorEditor (*this);
}

//==============================================================================
void GradientKnobAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void GradientKnobAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GradientKnobAudioProcessor();
}
