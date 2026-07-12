#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    A circular gradient knob replicating the original egui "Circular Gradient
    Knob" widget:
      - 300 degree arc sweep (120deg start -> 420deg end, 60-120deg dead zone)
      - green -> yellow -> red gradient based on value (0-100)
      - background track arc + filled progress arc
      - radial tick marks that light up and scale near the current value
      - soft glow halo behind the knob, colored by current value
      - white knob handle with colored ring + colored dot
      - center "NN%" text readout
      - drag (circular) and mouse-wheel / arrow-key interaction
      - animated smoothing toward a target value (spring-like ease)
*/
class GradientKnob : public juce::Component,
                      private juce::Timer
{
public:
    GradientKnob();
    ~GradientKnob() override;

    //==============================================================================
    void setValue (float newValue, juce::NotificationType notification);
    float getValue() const noexcept { return targetValue; }

    /** Smoothed value currently being displayed (0-100), useful for hosts. */
    float getDisplayValue() const noexcept { return currentValue; }

    std::function<void (float)> onValueChanged;   // fired with 0-100 value while dragging
    std::function<void()> onDragStart;
    std::function<void()> onDragEnd;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed (const juce::KeyPress&) override;

    /** Green -> yellow -> red gradient colour for a normalised value (0-1).
        Public so other UI elements (e.g. an external progress bar) can match the knob's colour. */
    static juce::Colour lerpColour (float t);

private:
    void timerCallback() override;

    static juce::Point<float> angleToPos (juce::Point<float> centre, float radius, float angleRad);
    static float mapDegToVal (float deg);

    float targetValue  = 50.0f;
    float currentValue = 50.0f;

    bool isDragging = false;

    static constexpr float startDeg = 120.0f;
    static constexpr float endDeg   = 420.0f;
    static constexpr float sweepDeg = 300.0f;
    static constexpr float deadStart = 60.0f;
    static constexpr float deadEnd   = 120.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientKnob)
};