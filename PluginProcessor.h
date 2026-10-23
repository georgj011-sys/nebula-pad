#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "WaveBank.h"
#include "PadVoice.h"

class NebulaPadProcessor : public juce::AudioProcessor
{
public:
    NebulaPadProcessor();
    ~NebulaPadProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Nebula Pad"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    const WaveBank& getWaveBank() const { return *waveBank; }

    juce::SharedResourcePointer<WaveBank> waveBank;
    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

private:
    void processDelay (juce::AudioBuffer<float>& buffer);
    float param (const char* id) const { return apvts.getRawParameterValue (id)->load(); }

    VoiceParams voiceParams;
    juce::Synthesiser synth;

    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;

    std::vector<float> delayL, delayR;
    int delayWritePos = 0;
    float delayLpL = 0.0f, delayLpR = 0.0f;
    juce::SmoothedValue<float> delaySamples;

    float lastMasterGain = 0.7f;
    double currentSampleRate = 44100.0;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NebulaPadProcessor)
};
