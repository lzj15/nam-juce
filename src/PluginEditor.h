#pragma once

#include "PluginProcessor.h"

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
    NAMAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NAMAudioProcessorEditor)
};
