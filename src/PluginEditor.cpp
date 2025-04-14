#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NAMAudioProcessorEditor::NAMAudioProcessorEditor(NAMAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(500, 500);

    //==============================================================================
    addAndMakeVisible(inputSlider);
    inputSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    inputSliderptr.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.apvts, "INPUT_ID", inputSlider));

    addAndMakeVisible(gateSlider);
    gateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gateSliderptr.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.apvts, "GATE_ID", gateSlider));

    addAndMakeVisible(bassSlider);
    bassSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bassSliderptr.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.apvts, "BASS_ID", bassSlider));

    addAndMakeVisible(middleSlider);
    middleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    middleSliderptr.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.apvts, "MIDDLE_ID", middleSlider));

    addAndMakeVisible(trebleSlider);
    trebleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    trebleSliderptr.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.apvts, "TREBLE_ID", trebleSlider));

    addAndMakeVisible(outputSlider);
    outputSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outputSliderptr.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.apvts, "OUTPUT_ID", outputSlider));

    loadButton.reset(new juce::TextButton("Load Model"));
    addAndMakeVisible(loadButton.get());
    loadButton->setBounds(50, 400, 100, 50);

    loadButton->onClick = [this]
    {
        juce::File searchLocation = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
        juce::FileChooser chooser("Choose a model to load", searchLocation, "*.nam", true, false);
        if (chooser.browseForFileToOpen())
        {
            juce::File model;
            model = chooser.getResult();
            processorRef.loadNamModel(model);
        }
    };
}

NAMAudioProcessorEditor::~NAMAudioProcessorEditor()
{
}

//==============================================================================
void NAMAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void NAMAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    inputSlider.setBounds(50, 50, 400, 50);
    gateSlider.setBounds(50, 100, 400, 50);
    bassSlider.setBounds(50, 150, 400, 50);
    middleSlider.setBounds(50, 200, 400, 50);
    trebleSlider.setBounds(50, 250, 400, 50);
    outputSlider.setBounds(50, 300, 400, 50);
}
