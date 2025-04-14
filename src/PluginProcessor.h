#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "NeuralAmpModeler.h"

//==============================================================================
class NAMAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    NAMAudioProcessor();
    ~NAMAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources () override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor () override;
    bool hasEditor () const override;

    //==============================================================================
    const juce::String getName () const override;

    bool acceptsMidi () const override;
    bool producesMidi () const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds () const override;

    //==============================================================================
    int getNumPrograms () override;
    int getCurrentProgram () override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void loadNamModel (juce::File modelToLoad);
    bool getNamModelStatus ();
    void clearNAM ();

    void loadImpulseResponse (juce::File irToLoad);
    bool getIrStatus ();
    void clearIR ();

    bool getTriggerStatus ();

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters ();

private:
    //==============================================================================
    NeuralAmpModeler myNAM;

    juce::dsp::Convolution cab;
    bool irFound{false};
    bool irLoaded{false};

    std::string lastModelPath = "null";
    std::string lastModelName = "null";

    std::string lastIrPath = "null";
    std::string lastIrName = "null";

    std::string lastModelSerachDir = "null";
    std::string lastIrSerachDir = "null";

    bool namModelLoaded{false};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NAMAudioProcessor)
};
