#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NAMAudioProcessorEditor::NAMAudioProcessorEditor(NAMAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(950, 650);
}

NAMAudioProcessorEditor::~NAMAudioProcessorEditor()
{
}

//==============================================================================
void NAMAudioProcessorEditor::paint(juce::Graphics& g)
{
}

void NAMAudioProcessorEditor::resized()
{
}
