#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"
#include "Presets.h"

NebulaPadProcessor::NebulaPadProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", Params::createLayout())
{
    auto p = [this] (const juce::String& id)
    {
        auto* v = apvts.getRawParameterValue (id);
        jassert (v != nullptr);
        return v;
    };

    for (int i = 0; i < 3; ++i)
    {
        auto& o = voiceParams.osc[i];
        o.level  = p (Params::osc (i, "level"));
        o.source = p (Params::osc (i, "source"));
        o.morph  = p (Params::osc (i, "morph"));
        o.octave = p (Params::osc (i, "octave"));
        o.semi   = p (Params::osc (i, "semi"));
        o.fine   = p (Params::osc (i, "fine"));
        o.unison = p (Params::osc (i, "unison"));
        o.detune = p (Params::osc (i, "detune"));
    }

    voiceParams.noise      = p ("noise");
    voiceParams.filterType = p ("filter_type");
    voiceParams.cutoff     = p ("cutoff");
    voiceParams.reso       = p ("reso");
    voiceParams.fenvAmt    = p ("fenv_amt");
    voiceParams.drive      = p ("drive");
    voiceParams.ampA       = p ("amp_attack");
    voiceParams.ampD       = p ("amp_decay");
    voiceParams.ampS       = p ("amp_sustain");
    voiceParams.ampR       = p ("amp_release");
    voiceParams.fltA       = p ("flt_attack");
    voiceParams.fltD       = p ("flt_decay");
    voiceParams.fltS       = p ("flt_sustain");
    voiceParams.fltR       = p ("flt_release");
    voiceParams.lfo1Rate   = p ("lfo1_rate");
    voiceParams.lfo1Morph  = p ("lfo1_morph");
    voiceParams.lfo2Rate   = p ("lfo2_rate");
    voiceParams.lfo2Cutoff = p ("lfo2_cutoff");
    voiceParams.chaos      = p ("chaos");
    voiceParams.width      = p ("width");

    synth.addSound (new PadSound());
    for (int i = 0; i < 12; ++i)
        synth.addVoice (new PadVoice (*waveBank, voiceParams));
}

void NebulaPadProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<PadVoice*> (synth.getVoice (i)))
            v->prepare (sampleRate, samplesPerBlock);

    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    chorus.prepare (spec);
    chorus.reset();
    reverb.prepare (spec);
    reverb.reset();

    const size_t delaySize = (size_t) (sampleRate * 2.1) + 1;
    delayL.assign (delaySize, 0.0f);
    delayR.assign (delaySize, 0.0f);
    delayWritePos = 0;
    delayLpL = delayLpR = 0.0f;
    delaySamples.reset (sampleRate, 0.3);
    delaySamples.setCurrentAndTargetValue (param ("delay_time") * (float) sampleRate);

    lastMasterGain = param ("master");
}

bool NebulaPadProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void NebulaPadProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
    synth.renderNextBlock (buffer, midi, 0, numSamples);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // Chorus
    const float chorusMix = param ("chorus_mix");
    if (chorusMix > 0.001f)
    {
        chorus.setRate (param ("chorus_rate"));
        chorus.setDepth (0.35f);
        chorus.setCentreDelay (9.0f);
        chorus.setFeedback (0.15f);
        chorus.setMix (chorusMix);
        chorus.process (context);
    }

    // Ping-pong delay
    processDelay (buffer);

    // Reverb
    juce::Reverb::Parameters rp;
    const float revMix = param ("reverb_mix");
    rp.roomSize   = 0.3f + 0.69f * param ("reverb_size");
    rp.damping    = 0.45f;
    rp.wetLevel   = revMix;
    rp.dryLevel   = 1.0f - revMix * 0.6f;
    rp.width      = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    reverb.process (context);

    // Master
    const float master = param ("master");
    buffer.applyGainRamp (0, numSamples, lastMasterGain, master);
    lastMasterGain = master;
}

void NebulaPadProcessor::processDelay (juce::AudioBuffer<float>& buffer)
{
    if (delayL.empty() || buffer.getNumChannels() < 2)
        return;

    const float mix = param ("delay_mix");
    const float fb  = param ("delay_fb");
    delaySamples.setTargetValue (param ("delay_time") * (float) currentSampleRate);

    const int size = (int) delayL.size();
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float d = delaySamples.getNextValue();
        float readPos = (float) delayWritePos - d;
        if (readPos < 0.0f)
            readPos += (float) size;

        const int i0 = juce::jlimit (0, size - 1, (int) readPos);
        const int i1 = (i0 + 1) % size;
        const float fr = readPos - (float) i0;

        const float dl = delayL[(size_t) i0] + fr * (delayL[(size_t) i1] - delayL[(size_t) i0]);
        const float dr = delayR[(size_t) i0] + fr * (delayR[(size_t) i1] - delayR[(size_t) i0]);

        // Gentle damping in the feedback path so repeats get darker
        delayLpL += 0.5f * (dl - delayLpL);
        delayLpR += 0.5f * (dr - delayLpR);

        const float inL = L[i], inR = R[i];
        delayL[(size_t) delayWritePos] = 0.5f * (inL + inR) + delayLpR * fb;
        delayR[(size_t) delayWritePos] = delayLpL * fb;

        L[i] = inL + dl * mix;
        R[i] = inR + dr * mix;

        delayWritePos = (delayWritePos + 1) % size;
    }
}

juce::AudioProcessorEditor* NebulaPadProcessor::createEditor()
{
    return new NebulaPadEditor (*this);
}

int NebulaPadProcessor::getNumPrograms()
{
    return (int) getFactoryPresets().size();
}

const juce::String NebulaPadProcessor::getProgramName (int index)
{
    const auto& presets = getFactoryPresets();
    if (index >= 0 && index < (int) presets.size())
        return presets[(size_t) index].name;
    return {};
}

void NebulaPadProcessor::setCurrentProgram (int index)
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= (int) presets.size())
        return;

    currentProgram = index;
    const auto& preset = presets[(size_t) index];

    for (auto* rawParam : getParameters())
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (rawParam))
        {
            float normalised = p->getDefaultValue();

            for (const auto& kv : preset.values)
            {
                if (kv.first == p->paramID)
                {
                    normalised = p->convertTo0to1 (kv.second);
                    break;
                }
            }

            p->setValueNotifyingHost (normalised);
        }
    }
}

void NebulaPadProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void NebulaPadProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        currentProgram = (int) tree.getProperty ("program", 0);
        apvts.replaceState (tree);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NebulaPadProcessor();
}
