#pragma once
#include <JuceHeader.h>
#include <ff_meters.h>

using namespace juce;

class knobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum KnobTypes
    {
        Main = 0,
        Minimal
    };

    knobLookAndFeel(KnobTypes knobType)
    {
        switch (knobType)
        {
            case KnobTypes::Main: knobImage = juce::ImageFileFormat::loadFrom(BinaryData::knob_png, BinaryData::knob_pngSize); break;
            case KnobTypes::Minimal:
                knobImage = juce::ImageFileFormat::loadFrom(BinaryData::knob_minimal_png, BinaryData::knob_minimal_pngSize);
                break;
            default: knobImage = juce::ImageFileFormat::loadFrom(BinaryData::knob_png, BinaryData::knob_pngSize); break;
        }
    }

public:
    Slider::SliderLayout getSliderLayout(Slider& slider) override
    {
        // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints

        int minXSpace = 0;
        int minYSpace = 0;

        auto textBoxPos = slider.getTextBoxPosition();

        if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
            minXSpace = 30;
        else
            minYSpace = 15;

        auto localBounds = slider.getLocalBounds();

        auto textBoxWidth = jmax(0, jmin(slider.getTextBoxWidth(), localBounds.getWidth() - minXSpace));
        auto textBoxHeight = jmax(0, jmin(slider.getTextBoxHeight(), localBounds.getHeight() - minYSpace));

        Slider::SliderLayout layout;

        // 2. set the textBox bounds

        if (textBoxPos != Slider::NoTextBox)
        {
            if (slider.isBar())
            {
                layout.textBoxBounds = localBounds;
            }
            else
            {
                layout.textBoxBounds.setWidth(textBoxWidth);
                layout.textBoxBounds.setHeight(textBoxHeight);

                if (textBoxPos == Slider::TextBoxLeft)
                    layout.textBoxBounds.setX(0);
                else if (textBoxPos == Slider::TextBoxRight)
                    layout.textBoxBounds.setX(localBounds.getWidth() - textBoxWidth);
                else /* above or below -> centre horizontally */
                    layout.textBoxBounds.setX((localBounds.getWidth() - textBoxWidth) / 2);

                if (textBoxPos == Slider::TextBoxAbove)
                    layout.textBoxBounds.setY(0);
                else if (textBoxPos == Slider::TextBoxBelow)
                    layout.textBoxBounds.setY(localBounds.getHeight() - textBoxHeight);
                else /* left or right -> centre vertically */
                    layout.textBoxBounds.setY((localBounds.getHeight() - textBoxHeight) / 2);
            }
        }

        // 3. set the slider bounds

        layout.sliderBounds = localBounds;

        if (slider.isBar())
        {
            layout.sliderBounds.reduce(1, 1); // bar border
        }
        else
        {
            if (textBoxPos == Slider::TextBoxLeft)
                layout.sliderBounds.removeFromLeft(textBoxWidth);
            else if (textBoxPos == Slider::TextBoxRight)
                layout.sliderBounds.removeFromRight(textBoxWidth);
            else if (textBoxPos == Slider::TextBoxAbove)
                layout.sliderBounds.removeFromTop(textBoxHeight);
            else if (textBoxPos == Slider::TextBoxBelow)
                layout.sliderBounds.removeFromBottom(textBoxHeight + 15);

            const int thumbIndent = getSliderThumbRadius(slider);

            if (slider.isHorizontal())
                layout.sliderBounds.reduce(thumbIndent, 0);
            else if (slider.isVertical())
                layout.sliderBounds.reduce(0, thumbIndent);
        }

        return layout;
    }

    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& textEditor) override
    {
        if (textEditor.isEnabled())
        {
            if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly())
            {
                const int border = 2;

                // g.setColour (textEditor.findColour (TextEditor::focusedOutlineColourId));
                g.setColour(juce::Colours::transparentBlack);
                g.drawRect(0, 0, width, height, border);

                g.setOpacity(1.0f);
                auto shadowColour = textEditor.findColour(TextEditor::shadowColourId).withMultipliedAlpha(0.75f);
                // drawBevel (g, 0, 0, width, height + 2, border + 2, shadowColour, shadowColour);
            }
            else
            {
                g.setColour(textEditor.findColour(TextEditor::outlineColourId));
                g.drawRect(0, 0, width, height);

                g.setOpacity(1.0f);
                auto shadowColour = textEditor.findColour(TextEditor::shadowColourId);
                // drawBevel (g, 0, 0, width, height + 2, 3, shadowColour, shadowColour);
            }
        }
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        if (knobImage.isValid())
        {
            const double rotation = (slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum());

            const int frames = knobImage.getHeight() / knobImage.getWidth();
            const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
            const float radius = juce::jmin(width / 2.0f, height / 2.0f);
            const float centerX = x + width * 0.5f;
            const float centerY = y + height * 0.5f;
            const float rx = centerX - radius - 1.0f;
            const float ry = centerY - radius;

            g.drawImage(knobImage, (int)rx, (int)ry, 2 * (int)radius, 2 * (int)radius, 0, frameId * knobImage.getWidth(), knobImage.getWidth(),
                        knobImage.getWidth());
        }
        else
        {
            static const float textPpercent = 0.35f;
            juce::Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f, 0.5f * height, width * textPpercent, 0.5f * height);

            g.setColour(juce::Colours::white);

            g.drawFittedText(juce::String("No Image"), text_bounds.getSmallestIntegerContainer(),
                             juce::Justification::horizontallyCentred | juce::Justification::centred, 1);
        }
    }

private:
    juce::Image knobImage;
};

//===================================================================
//===================================================================

class MeterLookAndFeel : public foleys::LevelMeterLookAndFeel
{
    juce::Rectangle<float> drawBackground(juce::Graphics& g, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::transparentWhite);
        if (meterType & foleys::LevelMeter::HasBorder)
        {
            const auto corner = std::min(bounds.getWidth(), bounds.getHeight()) * 0.01f;
            g.fillRoundedRectangle(bounds, corner);
            g.setColour(findColour(foleys::LevelMeter::lmOutlineColour));
            g.drawRoundedRectangle(bounds.reduced(3), corner, 2);
            return bounds.reduced(3 + corner);
        }
        else
        {
            g.fillRect(bounds);
            return bounds;
        }
    }

    void drawMeterBars(juce::Graphics& g, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds,
                       const foleys::LevelMeterSource* source, int fixedNumChannels = -1, int selectedChannel = -1)
    {
        if (source == nullptr)
            return;

        const juce::Rectangle<float> innerBounds = getMeterInnerBounds(bounds, meterType);
        const int numChannels = source->getNumChannels();
        if (meterType & foleys::LevelMeter::Minimal)
        {
            if (meterType & foleys::LevelMeter::Horizontal)
            {
                const float height = innerBounds.getHeight() / (2 * numChannels - 1);
                juce::Rectangle<float> meter = innerBounds.withHeight(height);
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    meter.setY(height * channel * 2);
                    {
                        juce::Rectangle<float> meterBarBounds = getMeterBarBounds(meter, meterType);
                        drawMeterBar(g, meterType, meterBarBounds, source->getRMSLevel(channel), source->getMaxLevel(channel));
                        const float reduction = source->getReductionLevel(channel);
                        if (reduction < 1.0)
                            drawMeterReduction(g, meterType, meterBarBounds.withBottom(meterBarBounds.getCentreY()), reduction);
                    }

                    juce::Rectangle<float> clip = getMeterClipIndicatorBounds(meter, meterType);
                    if (!clip.isEmpty())
                        drawClipIndicator(g, meterType, clip, source->getClipFlag(channel));
                    juce::Rectangle<float> maxNum = getMeterMaxNumberBounds(meter, meterType);

                    if (!maxNum.isEmpty())
                        drawMaxNumber(g, meterType, maxNum, source->getMaxOverallLevel(channel));

                    if (channel < numChannels - 1)
                    {
                        meter.setY(height * (channel * 2 + 1));
                        juce::Rectangle<float> ticks = getMeterTickmarksBounds(meter, meterType);
                        if (!ticks.isEmpty())
                            drawTickMarks(g, meterType, ticks);
                    }
                }
            }
            else
            {
                const float width = innerBounds.getWidth() / (2 * numChannels - 1);
                juce::Rectangle<float> meter = innerBounds.withWidth(width);
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    meter.setX(width * channel * 2);
                    {
                        juce::Rectangle<float> meterBarBounds = getMeterBarBounds(meter, meterType);
                        drawMeterBar(g, meterType, getMeterBarBounds(meter, meterType), source->getRMSLevel(channel), source->getMaxLevel(channel));
                        const float reduction = source->getReductionLevel(channel);
                        if (reduction < 1.0)
                            drawMeterReduction(g, meterType, meterBarBounds.withLeft(meterBarBounds.getCentreX()), reduction);
                    }
                    juce::Rectangle<float> clip = getMeterClipIndicatorBounds(meter, meterType);
                    if (!clip.isEmpty())
                        drawClipIndicator(g, meterType, clip, source->getClipFlag(channel));
                    juce::Rectangle<float> maxNum =
                        getMeterMaxNumberBounds(innerBounds.withWidth(innerBounds.getWidth() / numChannels)
                                                    .withX(innerBounds.getX() + channel * (innerBounds.getWidth() / numChannels)),
                                                meterType);
                    if (!maxNum.isEmpty())
                        drawMaxNumber(g, meterType, maxNum, source->getMaxOverallLevel(channel));
                    if (channel < numChannels - 1)
                    {
                        meter.setX(width * (channel * 2 + 1));
                        juce::Rectangle<float> ticks = getMeterTickmarksBounds(meter, meterType);
                        if (!ticks.isEmpty())
                            drawTickMarks(g, meterType, ticks);
                    }
                }
            }
        }
        else if (meterType & foleys::LevelMeter::SingleChannel)
        {
            if (selectedChannel >= 0)
                drawMeterChannel(g, meterType, innerBounds, source, selectedChannel);
        }
        else
        {
            const int numDrawnChannels = fixedNumChannels < 0 ? numChannels : fixedNumChannels;
            for (int channel = 0; channel < numChannels; ++channel)
                drawMeterChannel(g, meterType, getMeterBounds(innerBounds, meterType, numDrawnChannels, channel), source, channel);
        }
    }

    void drawMeterChannel(juce::Graphics& g, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds,
                          const foleys::LevelMeterSource* source, int selectedChannel)
    {
        if (source == nullptr)
            return;

        juce::Rectangle<float> meter = getMeterBarBounds(bounds, meterType);
        if (!meter.isEmpty())
        {
            if (meterType & foleys::LevelMeter::Reduction)
            {
                drawMeterBar(g, meterType, meter, source->getReductionLevel(selectedChannel), 0.0f);
            }
            else
            {
                drawMeterBar(g, meterType, meter, source->getRMSLevel(selectedChannel), source->getMaxLevel(selectedChannel));
                const float reduction = source->getReductionLevel(selectedChannel);
                if (reduction < 1.0)
                {
                    if (meterType & foleys::LevelMeter::Horizontal)
                        drawMeterReduction(g, meterType, meter.withBottom(meter.getCentreY()), reduction);
                    else
                        drawMeterReduction(g, meterType, meter.withLeft(meter.getCentreX()), reduction);
                }
            }
        }

        if (source->getClipFlag(selectedChannel))
        {
            juce::Rectangle<float> clip = getMeterClipIndicatorBounds(bounds, meterType);
            if (!clip.isEmpty())
                drawClipIndicator(g, meterType, clip, true);
        }

        juce::Rectangle<float> maxes = getMeterMaxNumberBounds(bounds, meterType);
        if (!maxes.isEmpty())
        {
            if (meterType & foleys::LevelMeter::Reduction)
                drawMaxNumber(g, meterType, maxes, source->getReductionLevel(selectedChannel));
            else
                drawMaxNumber(g, meterType, maxes, source->getMaxOverallLevel(selectedChannel));
        }
    }

    void drawTickMarks(juce::Graphics&, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds) override {}

    void drawMaxNumber(juce::Graphics& g, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds, float maxGain) override {}

    void drawMaxNumberBackground(juce::Graphics& g, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds) override {}

    void drawClipIndicatorBackground(juce::Graphics& g, foleys::LevelMeter::MeterFlags meterType, juce::Rectangle<float> bounds) override {}
};

//===================================================================
//===================================================================

class SliderOnLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos,
                          const Slider::SliderStyle style, Slider& slider)
    {

        if (slider.isBar())
        {
            g.setColour(juce::Colours::transparentBlack);
            g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float>(x), y + 0.5f, sliderPos - x, height - 1.0f)
                                             : juce::Rectangle<float>(x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
        }
        else
        {
            auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

            auto trackWidth = jmin(13.0f, slider.isHorizontal() ? height * 0.25f : width * 13.25f);


            Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f, slider.isHorizontal() ? y + height * 0.5f : height + y);

            Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x, slider.isHorizontal() ? startPoint.y : y);

            Path backgroundTrack;
            backgroundTrack.startNewSubPath(startPoint);
            backgroundTrack.lineTo(endPoint);
            g.setColour(juce::Colours::transparentBlack);
            g.strokePath(backgroundTrack, {trackWidth, PathStrokeType::beveled, PathStrokeType::rounded});

            Path valueTrack;
            Point<float> minPoint, maxPoint, thumbPoint;

            if (isTwoVal || isThreeVal)
            {
                minPoint = {slider.isHorizontal() ? minSliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : minSliderPos};

                if (isThreeVal)
                    thumbPoint = {slider.isHorizontal() ? sliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : sliderPos};

                maxPoint = {slider.isHorizontal() ? maxSliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : maxSliderPos};
            }
            else
            {
                auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
                auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

                minPoint = startPoint;
                maxPoint = {kx, ky};
            }

            auto thumbWidth = getSliderThumbRadius(slider);

            valueTrack.startNewSubPath(minPoint);
            valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
            g.setColour(juce::Colours::transparentBlack);
            g.strokePath(valueTrack, {trackWidth, PathStrokeType::curved, PathStrokeType::rounded});
            g.setColour(juce::Colours::black);

            if (!isTwoVal)
            {
                // g.setColour(slider.findColour(Slider::thumbColourId));
                // g.fillRect(Rectangle<float>(static_cast<float> (thumbWidth*3), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint
                // : maxPoint));

            }

            if (isTwoVal || isThreeVal)
            {
                auto sr = jmin(trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
                auto pointerColour = slider.findColour(Slider::thumbColourId);

                if (slider.isHorizontal())
                {
                    drawPointer(g, minSliderPos - sr, jmax(0.0f, y + height * 0.5f - trackWidth * 2.0f), trackWidth * 2.0f, pointerColour, 2);

                    drawPointer(
                        g, maxSliderPos - trackWidth, jmin(y + height - trackWidth * 2.0f, y + height * 0.5f), trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer(g, jmax(0.0f, x + width * 0.5f - trackWidth * 2.0f), minSliderPos - trackWidth, trackWidth * 2.0f, pointerColour, 1);

                    drawPointer(g, jmin(x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr, trackWidth * 2.0f, pointerColour, 3);
                }
            }
        }
    }


private:

};

//===================================================================
//===================================================================

class SliderOffLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos,
                          const Slider::SliderStyle style, Slider& slider)
    {

        if (slider.isBar())
        {
            g.setColour(juce::Colours::transparentBlack);
            g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float>(x), y + 0.5f, sliderPos - x, height - 1.0f)
                                             : juce::Rectangle<float>(x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
        }
        else
        {
            auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

            auto trackWidth = jmin(13.0f, slider.isHorizontal() ? height * 0.25f : width * 13.25f);


            Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f, slider.isHorizontal() ? y + height * 0.5f : height + y);

            Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x, slider.isHorizontal() ? startPoint.y : y);

            Path backgroundTrack;
            backgroundTrack.startNewSubPath(startPoint);
            backgroundTrack.lineTo(endPoint);
            g.setColour(juce::Colours::transparentBlack);
            g.strokePath(backgroundTrack, {trackWidth, PathStrokeType::beveled, PathStrokeType::rounded});

            Path valueTrack;
            Point<float> minPoint, maxPoint, thumbPoint;

            if (isTwoVal || isThreeVal)
            {
                minPoint = {slider.isHorizontal() ? minSliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : minSliderPos};

                if (isThreeVal)
                    thumbPoint = {slider.isHorizontal() ? sliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : sliderPos};

                maxPoint = {slider.isHorizontal() ? maxSliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : maxSliderPos};
            }
            else
            {
                auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
                auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

                minPoint = startPoint;
                maxPoint = {kx, ky};
            }

            auto thumbWidth = getSliderThumbRadius(slider);

            valueTrack.startNewSubPath(minPoint);
            valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
            g.setColour(juce::Colours::transparentBlack);
            g.strokePath(valueTrack, {trackWidth, PathStrokeType::curved, PathStrokeType::rounded});
            g.setColour(juce::Colours::black);

            if (!isTwoVal)
            {
                // g.setColour(slider.findColour(Slider::thumbColourId));
                // g.fillRect(Rectangle<float>(static_cast<float> (thumbWidth*3), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint
                // : maxPoint));

            }

            if (isTwoVal || isThreeVal)
            {
                auto sr = jmin(trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
                auto pointerColour = slider.findColour(Slider::thumbColourId);

                if (slider.isHorizontal())
                {
                    drawPointer(g, minSliderPos - sr, jmax(0.0f, y + height * 0.5f - trackWidth * 2.0f), trackWidth * 2.0f, pointerColour, 2);

                    drawPointer(
                        g, maxSliderPos - trackWidth, jmin(y + height - trackWidth * 2.0f, y + height * 0.5f), trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer(g, jmax(0.0f, x + width * 0.5f - trackWidth * 2.0f), minSliderPos - trackWidth, trackWidth * 2.0f, pointerColour, 1);

                    drawPointer(g, jmin(x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr, trackWidth * 2.0f, pointerColour, 3);
                }
            }
        }
    }


private:

};

//===================================================================
//===================================================================

class CustomSlider : public juce::Slider
{
public:
    CustomSlider(int sliderIndex = 0) { this->sliderIndex = sliderIndex; };

    void setCustomSlider(int sliderIndex) { this->sliderIndex = sliderIndex; };

    String getTextFromValue(double value) override
    {
        switch (sliderIndex)
        {
            case SliderTypes::Doubler:
                if (value == 0)
                    return "OFF";
                else
                    return juce::String(value) + " ms";
                break;
            case SliderTypes::Gate:
                if (value == -101)
                    return "OFF";
                else
                    return juce::String(value) + " dB";
                break;
            case SliderTypes::Filters:
                if (value <= 20 || value >= 20000)
                    return "OFF";
                else
                    return juce::String(value) + " Hz";
                break;
            default: return String(value) + this->getTextValueSuffix(); break;
        }
    };

    enum SliderTypes
    {
        Default = 0,
        Doubler,
        Gate,
        Filters
    };

private:
    int sliderIndex{0};
};
