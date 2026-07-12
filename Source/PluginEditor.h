#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GradientKnob.h"

//==============================================================================
class GradientKnobAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit GradientKnobAudioProcessorEditor (GradientKnobAudioProcessor&);
    ~GradientKnobAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    GradientKnobAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    GradientKnob knob;
    juce::Label valueLabel;

    // simple progress bar under the knob, mirrors the egui bottom bar
    juce::Rectangle<float> barBounds;

    // Manual two-way binding to the "value" parameter (GradientKnob is a
    // bespoke Component, not a juce::Slider, so SliderAttachment doesn't apply).
    juce::RangedAudioParameter* valueParam = nullptr;
    float lastKnownParamValue = -1.0f;
    bool isUpdatingFromParam = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientKnobAudioProcessorEditor)
};