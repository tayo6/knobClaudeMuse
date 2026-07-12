#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GradientKnobAudioProcessorEditor::GradientKnobAudioProcessorEditor (GradientKnobAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Title
    titleLabel.setText ("Circular Gradient Knob", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 200));

titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));

    addAndMakeVisible (titleLabel);

    // Knob
    addAndMakeVisible (knob);

    valueParam = audioProcessor.apvts.getParameter ("value");
    jassert (valueParam != nullptr);

    if (valueParam != nullptr)
    {
        float initial = valueParam->convertFrom0to1 (valueParam->getValue());
        knob.setValue (initial, juce::dontSendNotification);
        lastKnownParamValue = initial;
    }

    knob.onDragStart = [this]
    {
        if (valueParam != nullptr)
            valueParam->beginChangeGesture();
    };

    knob.onValueChanged = [this] (float v)
    {
        if (valueParam != nullptr && ! isUpdatingFromParam)
        {
            valueParam->setValueNotifyingHost (valueParam->convertTo0to1 (v));
            lastKnownParamValue = v;
        }
    };

    knob.onDragEnd = [this]
    {
        if (valueParam != nullptr)
            valueParam->endChangeGesture();
    };

    // Value label (mirrors the "Value: NN" text next to the egui bar)
    valueLabel.setJustificationType (juce::Justification::centredLeft);
    valueLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (160, 160, 160));
    addAndMakeVisible (valueLabel);

    setResizable (true, true);
    setResizeLimits (300, 420, 900, 1100);
    setSize (380, 580);

    startTimerHz (30);
}

GradientKnobAudioProcessorEditor::~GradientKnobAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void GradientKnobAudioProcessorEditor::timerCallback()
{
    // Poll the parameter in case host automation changed it externally.
    if (valueParam == nullptr)
        return;

    float hostValue = valueParam->convertFrom0to1 (valueParam->getValue());
    if (std::abs (hostValue - lastKnownParamValue) > 0.001f)
    {
        isUpdatingFromParam = true;
        knob.setValue (hostValue, juce::dontSendNotification);
        lastKnownParamValue = hostValue;
        isUpdatingFromParam = false;
    }

    valueLabel.setText ("Value: " + juce::String ((int) std::round (knob.getDisplayValue())),
                         juce::dontSendNotification);
    repaint (barBounds.getSmallestIntegerContainer());
}

//==============================================================================
void GradientKnobAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (14, 14, 18));

    // bottom progress bar, mirrors the thin bar drawn beside "Value: NN" in the egui version
    if (! barBounds.isEmpty())
    {
        float t = knob.getDisplayValue() / 100.0f;
        auto fillBounds = barBounds.withWidth (barBounds.getWidth() * t);

        g.setColour (juce::Colour::fromRGB (36, 40, 50));
        g.fillRoundedRectangle (barBounds, 3.0f);

        g.setColour (GradientKnob::lerpColour (t));
        g.fillRoundedRectangle (fillBounds, 3.0f);
    }
}

void GradientKnobAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);

    titleLabel.setBounds (area.removeFromTop (30));
    area.removeFromTop (6);

    auto knobSize = juce::jmin (area.getWidth(), area.getHeight() - 60);
    knobSize = juce::jmax (knobSize, 200);

    auto knobArea = area.removeFromTop (knobSize).withSizeKeepingCentre (knobSize, knobSize);
    knob.setBounds (knobArea);

    area.removeFromTop (18);

    auto bottomRow = area.removeFromTop (24);
    valueLabel.setBounds (bottomRow.removeFromLeft (100));
    bottomRow.removeFromLeft (10);
    barBounds = bottomRow.removeFromLeft (160).toFloat().withSizeKeepingCentre (160.0f, 6.0f);
}