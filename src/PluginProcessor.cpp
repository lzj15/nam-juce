/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================
NAMAudioProcessor::NAMAudioProcessor()
    : AudioProcessor(BusesProperties()
        #if !JucePlugin_IsMidiEffect
        #if !JucePlugin_IsSynth
            .withInput("Input", juce::AudioChannelSet::mono(), true)
        #endif
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        #endif
        ),
      apvts(*this, nullptr, "Parameters", createParameters())
{
}

NAMAudioProcessor::~NAMAudioProcessor()
{
}

//==============================================================================
const juce::String NAMAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NAMAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NAMAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NAMAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NAMAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NAMAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int NAMAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NAMAudioProcessor::setCurrentProgram(int index) {}

const juce::String NAMAudioProcessor::getProgramName(int index)
{
    return {};
}

void NAMAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void NAMAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    spec.maximumBlockSize = samplesPerBlock;

    myNAM.prepare(spec);
    myNAM.hookParameters(apvts);

    cab.reset();
    cab.prepare(spec);
}

void NAMAudioProcessor::loadNamModel(juce::File modelToLoad)
{
    std::string model_path = modelToLoad.getFullPathName().toStdString();
    this->suspendProcessing(true);
    namModelLoaded = myNAM.loadModel(model_path);
    this->suspendProcessing(false);

    auto addons = apvts.state.getOrCreateChildWithName("addons", nullptr);
    lastModelPath = model_path;
    lastModelName = modelToLoad.getFileNameWithoutExtension().toStdString();
    addons.setProperty("model_path", juce::String(lastModelPath), nullptr);

    auto search_paths = apvts.state.getOrCreateChildWithName("search_paths", nullptr);
    lastModelSerachDir = modelToLoad.getParentDirectory().getFullPathName().toStdString();
    search_paths.setProperty("LastModelSearchDir", juce::String(lastModelSerachDir), nullptr);
}

bool NAMAudioProcessor::getTriggerStatus()
{
    auto t_state = myNAM.getTrigger();
    return t_state->isGating();
}

bool NAMAudioProcessor::getNamModelStatus()
{
    return this->namModelLoaded;
}

void NAMAudioProcessor::clearNAM()
{
    this->suspendProcessing(true);
    myNAM.clearModel();
    lastModelPath = "null";
    lastModelName = "null";

    auto addons = apvts.state.getOrCreateChildWithName("addons", nullptr);
    addons.setProperty("model_path", juce::String(lastModelPath), nullptr);

    namModelLoaded = false;

    this->suspendProcessing(false);
}


void NAMAudioProcessor::loadImpulseResponse(juce::File irToLoad)
{
    this->suspendProcessing(true);

    this->clearIR();
    std::string ir_path = irToLoad.getFullPathName().toStdString();
    cab.loadImpulseResponse(
        irToLoad, juce::dsp::Convolution::Stereo::no, juce::dsp::Convolution::Trim::no, 0, juce::dsp::Convolution::Normalise::yes);
    irLoaded = true;
    irFound = true;

    auto addons = apvts.state.getOrCreateChildWithName("addons", nullptr);
    lastIrPath = ir_path;
    lastIrName = irToLoad.getFileNameWithoutExtension().toStdString();
    addons.setProperty("ir_path", juce::String(lastIrPath), nullptr);

    auto search_paths = apvts.state.getOrCreateChildWithName("search_paths", nullptr);
    lastIrSerachDir = irToLoad.getParentDirectory().getFullPathName().toStdString();
    search_paths.setProperty("LastIrSearchDir", juce::String(lastIrSerachDir), nullptr);

    this->suspendProcessing(false);
}

void NAMAudioProcessor::clearIR()
{
    cab.reset();
    irLoaded = false;
    lastIrPath = "null";
    lastIrName = "null";

    auto addons = apvts.state.getOrCreateChildWithName("addons", nullptr);
    addons.setProperty("ir_path", juce::String(lastIrPath), nullptr);
}

bool NAMAudioProcessor::getIrStatus()
{
    return irLoaded;
}

void NAMAudioProcessor::releaseResources()
{
}

bool NAMAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
        juce::ignoreUnused(layouts);
        return true;
    #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

            // This checks if the input layout matches the output layout
    #if !JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
    #endif

        return true;
    #endif
}

void NAMAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);

    auto* channelDataLeft = buffer.getWritePointer(0);
    auto* channelDataRight = buffer.getWritePointer(1);

    myNAM.processBlock(buffer);

    if (bool(*apvts.getRawParameterValue("CAB_ON_ID")) && irLoaded)
    {
        cab.process(juce::dsp::ProcessContextReplacing<float>(block));
        if (irFound)
            buffer.applyGain(juce::Decibels::decibelsToGain(6.0f));
    }

    // Do Dual Mono
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        channelDataRight[sample] = channelDataLeft[sample];
}


//==============================================================================
bool NAMAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NAMAudioProcessor::createEditor()
{
    //return new NAMAudioProcessorEditor(*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void NAMAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void NAMAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout NAMAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    myNAM.createParameters(parameters);

    parameters.push_back(std::make_unique<juce::AudioParameterBool>("CAB_ON_ID", "CAB_ON", true, "CAB_ON"));

    auto normRange = juce::NormalisableRange<float>(0.0, 20.0, 0.1f);

    return {parameters.begin(), parameters.end()};
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NAMAudioProcessor();
}
