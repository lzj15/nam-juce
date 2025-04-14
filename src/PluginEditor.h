#pragma once

#include "PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"

//==============================================================================
class NAMAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit NAMAudioProcessorEditor(NAMAudioProcessor&);
    ~NAMAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized () override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NAMAudioProcessor& processorRef;

    //==============================================================================
    juce::Slider inputSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputSliderptr;

    juce::Slider gateSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateSliderptr;

    juce::Slider bassSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassSliderptr;

    juce::Slider middleSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> middleSliderptr;

    juce::Slider trebleSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleSliderptr;

    juce::Slider outputSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputSliderptr;

    std::unique_ptr<juce::TextButton> loadButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NAMAudioProcessorEditor)
};
