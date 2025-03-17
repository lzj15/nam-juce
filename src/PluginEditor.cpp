#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NamJUCEAudioProcessorEditor::NamJUCEAudioProcessorEditor(NamJUCEAudioProcessor& p) : AudioProcessorEditor(&p), audioProcessor(p), namEditor(p)
{

    setSize(950, 650);

    // Main NAM Editor
    addAndMakeVisible(&namEditor);
    namEditor.setBounds(getLocalBounds());

}

NamJUCEAudioProcessorEditor::~NamJUCEAudioProcessorEditor()
{
}

//==============================================================================
void NamJUCEAudioProcessorEditor::paint(juce::Graphics& g) {}

void NamJUCEAudioProcessorEditor::resized() {}


void NamJUCEAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {}

void NamJUCEAudioProcessorEditor::timerCallback() {}
