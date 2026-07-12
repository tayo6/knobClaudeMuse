#include "GradientKnob.h"

//==============================================================================
GradientKnob::GradientKnob()
{
    setWantsKeyboardFocus (true);
    startTimerHz (60);
}

GradientKnob::~GradientKnob()
{
    stopTimer();
}

//==============================================================================
juce::Colour GradientKnob::lerpColour (float t)
{
    t = juce::jlimit (0.0f, 1.0f, t);

    if (t <= 0.5f)
    {
        float k = t * 2.0f;
        return juce::Colour::fromFloatRGBA (
            (34.0f  + (250.0f - 34.0f)  * k) / 255.0f,
            (197.0f + (204.0f - 197.0f) * k) / 255.0f,
            (94.0f  + (21.0f  - 94.0f)  * k) / 255.0f,
            1.0f);
    }
    else
    {
        float k = (t - 0.5f) * 2.0f;
        return juce::Colour::fromFloatRGBA (
            (250.0f + (220.0f - 250.0f) * k) / 255.0f,
            (204.0f + (38.0f  - 204.0f) * k) / 255.0f,
            (21.0f  + (38.0f  - 21.0f)  * k) / 255.0f,
            1.0f);
    }
}

juce::Point<float> GradientKnob::angleToPos (juce::Point<float> centre, float radius, float angleRad)
{
    return { centre.x + std::cos (angleRad) * radius,
             centre.y + std::sin (angleRad) * radius };
}

float GradientKnob::mapDegToVal (float deg)
{
    if (deg >= startDeg)
        return (deg - startDeg) / sweepDeg * 100.0f;
    else if (deg <= deadStart)
        return (deg + 240.0f) / sweepDeg * 100.0f;

    return 0.0f;
}

//==============================================================================
void GradientKnob::setValue (float newValue, juce::NotificationType notification)
{
    newValue = juce::jlimit (0.0f, 100.0f, newValue);
    if (juce::approximatelyEqual (newValue, targetValue))
        return;

    targetValue = newValue;

    if (notification != juce::dontSendNotification && onValueChanged != nullptr)
        onValueChanged (targetValue);

    repaint();
}

//==============================================================================
void GradientKnob::timerCallback()
{
    constexpr float dt = 1.0f / 60.0f;
    constexpr float speed = 18.0f;
    float lerp = 1.0f - std::exp (-dt * speed);

    float diff = targetValue - currentValue;
    if (std::abs (diff) < 0.01f)
    {
        if (currentValue != targetValue)
        {
            currentValue = targetValue;
            repaint();
        }
        return;
    }

    currentValue += diff * lerp;
    repaint();
}

//==============================================================================
void GradientKnob::resized()
{
}

void GradientKnob::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();

    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 40.0f;
    const float strokeW = radius * 0.17f;
    const float tickR   = radius * 1.30f;

    const float startRad = juce::degreesToRadians (startDeg);
    const float sweepRad = juce::degreesToRadians (sweepDeg);

    const float curT   = currentValue / 100.0f;
    const float curRad = startRad + curT * sweepRad;
    auto curColour = lerpColour (curT);

    // --- soft glow halo -----------------------------------------------
    {
        auto glowColour = curColour.withAlpha (0.055f);
        float glowR = radius + 20.0f;
        juce::ColourGradient grad (glowColour, centre, glowColour.withAlpha (0.0f),
                                    centre.translated (glowR, 0.0f), true);
        g.setGradientFill (grad);
        g.fillEllipse (juce::Rectangle<float>(glowR * 2.0f, glowR * 2.0f).withCentre (centre));
    }

    // --- tick marks ------------------------------------------------------
    for (int i = 0; i <= 40; ++i)
    {
        float t = i / 40.0f;
        float rad = juce::degreesToRadians (startDeg + t * sweepDeg);
        bool major = (i % 10 == 0);
        bool mid   = (i % 5 == 0);

        float len = major ? 16.0f : (mid ? 11.0f : 7.0f);
        float w   = major ? 2.8f  : 1.6f;

        float dist = std::abs (t - curT);
        float scale = (dist < 0.10f) ? (1.0f + (0.10f - dist) / 0.10f * 0.6f) : 1.0f;

        juce::Colour col = (t <= curT + 0.001f) ? lerpColour (t) : juce::Colour::fromRGB (95, 95, 95);

        auto p1 = angleToPos (centre, tickR, rad);
        auto p2 = angleToPos (centre, tickR + len * scale, rad);

        g.setColour (col);
        g.drawLine ({ p1, p2 }, w * scale);
        g.fillEllipse (juce::Rectangle<float>(w * scale, w * scale).withCentre (p1));
        g.fillEllipse (juce::Rectangle<float>(w * scale, w * scale).withCentre (p2));
    }

    // --- background track arc --------------------------------------------
    auto greyColour = juce::Colour::fromRGB (48, 52, 62);
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, radius, radius,
                              0.0f, startRad, startRad + sweepRad, true);
        g.setColour (greyColour);
        g.strokePath (track, juce::PathStrokeType (strokeW, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }

    // --- filled gradient progress arc ------------------------------------
    if (curT > 0.001f)
    {
        const int steps = 100;
        for (int s = 0; s < steps; ++s)
        {
            float t0 = (float) s       / steps * curT;
            float t1 = (float) (s + 1) / steps * curT;
            float a0 = startRad + t0 * sweepRad;
            float a1 = startRad + t1 * sweepRad;
            float mid = (t0 + t1) * 0.5f;

            juce::Path seg;
            seg.startNewSubPath (angleToPos (centre, radius, a0));
            seg.lineTo (angleToPos (centre, radius, a1));

            g.setColour (lerpColour (mid));
            g.strokePath (seg, juce::PathStrokeType (strokeW, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }
    }

    // --- knob handle -------------------------------------------------------
    auto knobPos = angleToPos (centre, radius, curRad);

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (juce::Rectangle<float>(34.0f, 34.0f).withCentre (knobPos));

    g.setColour (juce::Colours::white);
    g.fillEllipse (juce::Rectangle<float>(27.0f, 27.0f).withCentre (knobPos));

    g.setColour (curColour);
    g.drawEllipse (juce::Rectangle<float>(27.0f, 27.0f).withCentre (knobPos), 3.2f);

    g.fillEllipse (juce::Rectangle<float>(6.0f, 6.0f).withCentre (knobPos));

    // --- centre readout ------------------------------------------------
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (40.0f, juce::Font::plain));
    juce::String pctText = juce::String ((int) std::round (currentValue)) + "%";
    g.drawText (pctText, bounds.withY (centre.y - 30.0f).withHeight (60.0f), juce::Justification::centred);

    g.setColour (juce::Colour::fromRGB (130, 130, 130));
    g.setFont (juce::Font (12.0f, juce::Font::plain));
    g.drawText ("drag or arrow keys", bounds.withY (centre.y + 26.0f).withHeight (20.0f),
                juce::Justification::centred);
}

//==============================================================================
void GradientKnob::mouseDown (const juce::MouseEvent& e)
{
    isDragging = true;
    if (onDragStart != nullptr)
        onDragStart();

    mouseDrag (e);
}

void GradientKnob::mouseDrag (const juce::MouseEvent& e)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();

    auto v = e.position - centre;
    float deg = juce::radiansToDegrees (std::atan2 (v.y, v.x));
    if (deg < 0.0f)
        deg += 360.0f;

    if (! (deg > deadStart && deg < deadEnd))
    {
        float nv = juce::jlimit (0.0f, 100.0f, mapDegToVal (deg));
        if (std::abs (nv - targetValue) < 70.0f)
            setValue (nv, juce::sendNotification);
    }
}

void GradientKnob::mouseUp (const juce::MouseEvent&)
{
    isDragging = false;
    if (onDragEnd != nullptr)
        onDragEnd();
}

void GradientKnob::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    float delta = (wheel.deltaY > 0.0f ? 1.0f : -1.0f) * 2.0f;
    setValue (targetValue + delta, juce::sendNotification);
}

bool GradientKnob::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::rightKey || key == juce::KeyPress::upKey)
    {
        setValue (targetValue + 2.0f, juce::sendNotification);
        return true;
    }
    if (key == juce::KeyPress::leftKey || key == juce::KeyPress::downKey)
    {
        setValue (targetValue - 2.0f, juce::sendNotification);
        return true;
    }
    return false;
}