#include "NeuralAmpModeler.h"
#include <filesystem>
#include <iostream>

NeuralAmpModeler::NeuralAmpModeler()
{
    mToneStack = std::make_unique<dsp::tone_stack::BasicNamToneStack>();
    nam::activations::Activation::enable_fast_tanh();

    mNoiseGateTrigger.AddListener(&mNoiseGateGain);
}

NeuralAmpModeler::~NeuralAmpModeler()
{
}

void NeuralAmpModeler::prepare(juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    this->samplesPerBlock = spec.maximumBlockSize;

    outputBuffer.setSize(1, spec.maximumBlockSize, false, false, false);
    outputBuffer.clear();

    resetModel();
    mToneStack->Reset(this->sampleRate, this->samplesPerBlock);

    mNoiseGateTrigger.SetSampleRate(this->sampleRate);
}

void NeuralAmpModeler::processBlock(juce::AudioBuffer<float>& buffer)
{
    this->applyDSPStaging();
    this->updateParameters();

    auto* channelDataLeft = buffer.getWritePointer(0);
    auto* channelDataRight = buffer.getWritePointer(1);
    auto* outputData = outputBuffer.getWritePointer(0);

    float** inputPointer = &channelDataLeft;
    float** outputPointer = &outputData;
    float** processedOutput;
    float** triggerOutput = inputPointer;

    if (noiseGateActive) // Process gate trigger
        triggerOutput = mNoiseGateTrigger.Process(inputPointer, 1, buffer.getNumSamples());

    if (mModel != nullptr)
    {
        // Input Gain
        buffer.applyGain(dB_to_linear(params[Parameters::kInputLevel]->load()));

        mModel->process(*inputPointer, *outputPointer, buffer.getNumSamples());

        // Normalize loudness
        if (this->outputNormalized)
            normalizeOutput(outputPointer, 1, buffer.getNumSamples());

        processedOutput = outputPointer;
    }
    else
    {
        processedOutput = inputPointer;
    }

    // Apply the noise gate
    float** gateGainOutput = noiseGateActive ? mNoiseGateGain.Process(processedOutput, 1, buffer.getNumSamples()) : processedOutput;

    // Tone Stack
    float** toneStackOutPointers = toneStackActive ? mToneStack->Process(gateGainOutput, 1, buffer.getNumSamples()) : gateGainOutput;

    doDualMono(buffer, toneStackOutPointers);

    // Output Gain
    buffer.applyGain(dB_to_linear(params[Parameters::kOutputLevel]->load()));
}

bool NeuralAmpModeler::loadModel(const std::string modelPath)
{
    try
    {
        auto dspPath = std::filesystem::u8path(modelPath);
        std::unique_ptr<nam::DSP> model = nam::get_dsp(dspPath);
        std::unique_ptr<ResamplingNAM> temp = std::make_unique<ResamplingNAM>(std::move(model), this->sampleRate);

        temp->Reset(this->sampleRate, this->samplesPerBlock);

        mStagedModel = std::move(temp);

        return true;
    }
    catch (std::runtime_error& e)
    {
        if (mStagedModel != nullptr)
        {
            mStagedModel = nullptr;
        }

        std::cout << "woops" << std::endl;
        std::cerr << "Failed to read DSP module" << std::endl;
        std::cerr << e.what() << std::endl;

        return false;
    }
}

bool NeuralAmpModeler::isModelLoaded()
{
    return this->modelLoaded;
}

void NeuralAmpModeler::clearModel()
{
    this->shouldRemoveModel = true;
}

void NeuralAmpModeler::applyDSPStaging()
{
    // Remove marked modules
    if (shouldRemoveModel)
    {
        mModel = nullptr;
        shouldRemoveModel = false;
        //_UpdateLatency();
    }

    // Move things from staged to live
    if (mStagedModel != nullptr)
    {
        // Move from staged to active DSP
        mModel = std::move(mStagedModel);
        mStagedModel = nullptr;
        modelLoaded = true;
        //_UpdateLatency();
    }
}

void NeuralAmpModeler::resetModel()
{
    if (mStagedModel != nullptr)
        mStagedModel->Reset(this->sampleRate, this->samplesPerBlock);

    else if (mModel != nullptr)
        mModel->Reset(this->sampleRate, this->samplesPerBlock);
}

void NeuralAmpModeler::normalizeOutput(float** input, int numChannels, int numSamples)
{
    if (!mModel)
        return;
    if (!mModel->HasLoudness())
        return;

    const double loudness = mModel->GetLoudness();
    const double targetLoudness = -18.0;
    const double gain = pow(10.0, (targetLoudness - loudness) / 20.0);

    for (int c = 0; c < numChannels; c++)
    {
        for (int f = 0; f < numSamples; f++)
        {
            input[c][f] *= gain;
        }
    }
}

void NeuralAmpModeler::updateParameters()
{
    outputNormalized = bool(params[Parameters::kOutNorm]->load());

    // Tone Stack
    toneStackActive = bool(params[Parameters::kEQActive]->load());

    // Noise Gate
    noiseGateActive = int(params[Parameters::kNoiseGateThreshold]->load()) < -100 ? false : true;

    if (toneStackActive)
    {
        mToneStack->SetParam("bass", params[Parameters::kToneBass]->load());
        mToneStack->SetParam("middle", params[Parameters::kToneMid]->load());
        mToneStack->SetParam("treble", params[Parameters::kToneTreble]->load());
    }

    if (noiseGateActive)
    {
        const dsp::noise_gate::TriggerParams triggerParams(
            this->ns_time, params[Parameters::kNoiseGateThreshold]->load(), this->ns_ratio, this->ns_openTime, this->ns_holdTime, this->ns_closeTime);

        mNoiseGateTrigger.SetParams(triggerParams);
    }
}

void NeuralAmpModeler::hookParameters(juce::AudioProcessorValueTreeState& apvts)
{
    params[Parameters::kInputLevel] = apvts.getRawParameterValue("INPUT_ID");
    params[Parameters::kNoiseGateThreshold] = apvts.getRawParameterValue("GATE_ID");
    params[Parameters::kToneBass] = apvts.getRawParameterValue("BASS_ID");
    params[Parameters::kToneMid] = apvts.getRawParameterValue("MIDDLE_ID");
    params[Parameters::kToneTreble] = apvts.getRawParameterValue("TREBLE_ID");
    params[Parameters::kOutputLevel] = apvts.getRawParameterValue("OUTPUT_ID");
    params[Parameters::kEQActive] = apvts.getRawParameterValue("TONE_STACK_ON_ID");
    params[Parameters::kOutNorm] = apvts.getRawParameterValue("NORMALIZE_ID");

    DBG("NAM Parameters Hooked!");
}

double NeuralAmpModeler::dB_to_linear(double db_value)
{
    return std::pow(10.0, db_value / 20.0);
}

void NeuralAmpModeler::doDualMono(juce::AudioBuffer<float>& mainBuffer, float** input)
{
    auto channelDataLeft = mainBuffer.getWritePointer(0);
    auto channelDataRight = mainBuffer.getWritePointer(1);

    for (int sample = 0; sample < mainBuffer.getNumSamples(); ++sample)
    {
        channelDataRight[sample] = input[0][sample];
        channelDataLeft[sample] = input[0][sample];
    }
}
