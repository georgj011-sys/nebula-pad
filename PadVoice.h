#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>
#include "WaveBank.h"

struct PadSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

struct VoiceParams
{
    struct Osc
    {
        std::atomic<float>* level  = nullptr;
        std::atomic<float>* source = nullptr;
        std::atomic<float>* morph  = nullptr;
        std::atomic<float>* octave = nullptr;
        std::atomic<float>* semi   = nullptr;
        std::atomic<float>* fine   = nullptr;
        std::atomic<float>* unison = nullptr;
        std::atomic<float>* detune = nullptr;
    };

    Osc osc[3];
    std::atomic<float>* noise = nullptr;
    std::atomic<float>* filterType = nullptr;
    std::atomic<float>* cutoff = nullptr;
    std::atomic<float>* reso = nullptr;
    std::atomic<float>* fenvAmt = nullptr;
    std::atomic<float>* drive = nullptr;
    std::atomic<float>* ampA = nullptr;
    std::atomic<float>* ampD = nullptr;
    std::atomic<float>* ampS = nullptr;
    std::atomic<float>* ampR = nullptr;
    std::atomic<float>* fltA = nullptr;
    std::atomic<float>* fltD = nullptr;
    std::atomic<float>* fltS = nullptr;
    std::atomic<float>* fltR = nullptr;
    std::atomic<float>* lfo1Rate = nullptr;
    std::atomic<float>* lfo1Morph = nullptr;
    std::atomic<float>* lfo2Rate = nullptr;
    std::atomic<float>* lfo2Cutoff = nullptr;
    std::atomic<float>* chaos = nullptr;
    std::atomic<float>* width = nullptr;
};

class PadVoice : public juce::SynthesiserVoice
{
public:
    static constexpr int maxUnison = 7;
    static constexpr int controlBlock = 32;

    PadVoice (const WaveBank& b, const VoiceParams& p) : bank (b), params (p) {}

    void prepare (double sampleRate, int blockSize)
    {
        sr = sampleRate;
        ampEnv.setSampleRate (sampleRate);
        fltEnv.setSampleRate (sampleRate);
        filter.prepare ({ sampleRate, (juce::uint32) juce::jmax (1, blockSize), 2 });
        filter.reset();
    }

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<PadSound*> (s) != nullptr; }

    void startNote (int midiNote, float vel, juce::SynthesiserSound*, int pitchWheel) override
    {
        noteFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
        velocity = vel;
        pitchWheelMoved (pitchWheel);

        // Random start phases keep stacked unison voices from sounding phasey.
        for (auto& osc : phases)
            for (auto& ph : osc)
                ph = rng.nextFloat() * (float) WaveBank::tableSize;

        lfo1Phase = rng.nextFloat();
        lfo2Phase = rng.nextFloat();
        chaosVal = 0.0f;
        chaosTarget = rng.nextFloat() * 2.0f - 1.0f;
        chaosCounter = 0;

        updateEnvelopes();
        ampEnv.noteOn();
        fltEnv.noteOn();
        filter.reset();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnv.noteOff();
            fltEnv.noteOff();
        }
        else
        {
            clearCurrentNote();
            ampEnv.reset();
            fltEnv.reset();
        }
    }

    void pitchWheelMoved (int value) override
    {
        bendSemis = ((float) value - 8192.0f) / 8192.0f * 2.0f;
    }

    void controllerMoved (int, int) override {}

    using juce::SynthesiserVoice::renderNextBlock;

    void renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        updateEnvelopes();

        const int fType = (int) get (params.filterType);
        filter.setType (fType == 0 ? juce::dsp::StateVariableTPTFilterType::lowpass
                      : fType == 1 ? juce::dsp::StateVariableTPTFilterType::bandpass
                                   : juce::dsp::StateVariableTPTFilterType::highpass);

        const float reso = get (params.reso);
        filter.setResonance (0.7071f + reso * reso * 12.0f);

        const float baseCutoff = get (params.cutoff);
        const float fenvAmt    = get (params.fenvAmt);
        const float drive      = get (params.drive);
        const float noise      = get (params.noise);
        const float lfo1Rate   = get (params.lfo1Rate);
        const float lfo1Morph  = get (params.lfo1Morph);
        const float lfo2Rate   = get (params.lfo2Rate);
        const float lfo2Cut    = get (params.lfo2Cutoff);
        const float chaos      = get (params.chaos);
        const float width      = get (params.width);

        const float velGain   = 0.35f + 0.65f * velocity;
        const float driveGain = 1.0f + drive * 6.0f;
        const float driveComp = 1.0f / (1.0f + drive * 1.5f);

        float* outL = out.getWritePointer (0);
        float* outR = out.getNumChannels() > 1 ? out.getWritePointer (1) : nullptr;

        while (numSamples > 0)
        {
            const int n = juce::jmin (controlBlock, numSamples);
            const float twoPi = juce::MathConstants<float>::twoPi;

            // LFOs (per voice, free running)
            const float lfo1 = std::sin (twoPi * lfo1Phase);
            const float lfo2 = std::sin (twoPi * lfo2Phase);
            lfo1Phase += lfo1Rate * (float) n / (float) sr;
            lfo2Phase += lfo2Rate * (float) n / (float) sr;
            lfo1Phase -= std::floor (lfo1Phase);
            lfo2Phase -= std::floor (lfo2Phase);

            // Chaos: smoothed random drift that pushes pitch, morph and filter
            chaosCounter -= n;
            if (chaosCounter <= 0)
            {
                chaosTarget = rng.nextFloat() * 2.0f - 1.0f;
                chaosCounter = (int) (sr * (0.12 + rng.nextDouble() * 0.5));
            }
            chaosVal += (chaosTarget - chaosVal) * juce::jmin (1.0f, (float) n / (float) (sr * 0.25));
            const float chaosMod = chaos * chaosVal;

            // Filter envelope at control rate
            const float fenv = fltEnv.getNextSample();
            for (int i = 1; i < n; ++i)
                fltEnv.getNextSample();

            float cutoff = baseCutoff * std::exp2 (fenvAmt * fenv * 6.0f + lfo2Cut * lfo2 * 2.5f + chaosMod * 1.5f);
            cutoff = juce::jlimit (20.0f, (float) (sr * 0.45), cutoff);
            filter.setCutoffFrequency (cutoff);

            OscBlock blocks[3];

            for (int o = 0; o < 3; ++o)
            {
                const auto& op = params.osc[o];
                auto& b = blocks[o];

                const float level = get (op.level);
                b.active = level > 0.0001f;
                if (! b.active)
                    continue;

                const int source = (int) get (op.source);

                float morph = get (op.morph) + lfo1Morph * 0.5f * lfo1 + chaosMod * 0.35f * (o == 1 ? -1.0f : 1.0f);
                morph = juce::jlimit (0.0f, 1.0f, morph);

                const float fp = morph * (float) (WaveBank::numFrames - 1);
                const int f0 = juce::jlimit (0, WaveBank::numFrames - 1, (int) fp);
                const int f1 = juce::jmin (f0 + 1, WaveBank::numFrames - 1);
                b.frac = fp - (float) f0;

                const float semis = get (op.octave) * 12.0f + get (op.semi) + get (op.fine) * 0.01f
                                  + bendSemis + chaosMod * 0.25f;
                const float freq = noteFreq * std::exp2 (semis / 12.0f);
                const float detune = get (op.detune);

                const int mip = WaveBank::levelForFrequency (freq * (1.0 + detune * 0.03), sr);
                b.t0 = bank.getTable (source, f0, mip);
                b.t1 = bank.getTable (source, f1, mip);

                b.unison = juce::jlimit (1, maxUnison, (int) get (op.unison));
                const float norm = level / std::sqrt ((float) b.unison);

                for (int u = 0; u < b.unison; ++u)
                {
                    const float pos = b.unison == 1 ? 0.0f : (float) u / (float) (b.unison - 1) * 2.0f - 1.0f;
                    const float cents = pos * detune * 40.0f;
                    b.inc[u] = freq * std::exp2 (cents / 1200.0f) * (float) WaveBank::tableSize / (float) sr;

                    const float pan = pos * width;
                    b.gL[u] = norm * std::sqrt (0.5f * (1.0f - pan));
                    b.gR[u] = norm * std::sqrt (0.5f * (1.0f + pan));
                }
            }

            for (int i = 0; i < n; ++i)
            {
                float l = 0.0f, r = 0.0f;

                for (int o = 0; o < 3; ++o)
                {
                    const auto& b = blocks[o];
                    if (! b.active)
                        continue;

                    for (int u = 0; u < b.unison; ++u)
                    {
                        float& ph = phases[o][u];
                        const int i0 = (int) ph;
                        const float fr = ph - (float) i0;

                        const float s0 = b.t0[i0] + fr * (b.t0[i0 + 1] - b.t0[i0]);
                        const float s1 = b.t1[i0] + fr * (b.t1[i0 + 1] - b.t1[i0]);
                        const float s  = s0 + b.frac * (s1 - s0);

                        l += s * b.gL[u];
                        r += s * b.gR[u];

                        ph += b.inc[u];
                        if (ph >= (float) WaveBank::tableSize)
                            ph -= (float) WaveBank::tableSize;
                    }
                }

                if (noise > 0.0f)
                {
                    l += (rng.nextFloat() * 2.0f - 1.0f) * noise * 0.4f;
                    r += (rng.nextFloat() * 2.0f - 1.0f) * noise * 0.4f;
                }

                l = std::tanh (l * 0.4f * driveGain) * driveComp;
                r = std::tanh (r * 0.4f * driveGain) * driveComp;

                l = filter.processSample (0, l);
                r = filter.processSample (1, r);

                const float amp = ampEnv.getNextSample() * velGain;

                if (outR != nullptr)
                {
                    outL[startSample + i] += l * amp;
                    outR[startSample + i] += r * amp;
                }
                else
                {
                    outL[startSample + i] += 0.5f * (l + r) * amp;
                }
            }

            startSample += n;
            numSamples -= n;

            if (! ampEnv.isActive())
            {
                clearCurrentNote();
                filter.reset();
                break;
            }
        }
    }

private:
    struct OscBlock
    {
        const float* t0 = nullptr;
        const float* t1 = nullptr;
        float frac = 0.0f;
        float inc[maxUnison] {};
        float gL[maxUnison] {};
        float gR[maxUnison] {};
        int unison = 1;
        bool active = false;
    };

    static float get (std::atomic<float>* p) noexcept { return p->load (std::memory_order_relaxed); }

    void updateEnvelopes()
    {
        ampEnv.setParameters ({ get (params.ampA), get (params.ampD), get (params.ampS), get (params.ampR) });
        fltEnv.setParameters ({ get (params.fltA), get (params.fltD), get (params.fltS), get (params.fltR) });
    }

    const WaveBank& bank;
    const VoiceParams& params;

    double sr = 44100.0;
    float noteFreq = 440.0f;
    float velocity = 1.0f;
    float bendSemis = 0.0f;

    float phases[3][maxUnison] {};
    float lfo1Phase = 0.0f, lfo2Phase = 0.0f;
    float chaosVal = 0.0f, chaosTarget = 0.0f;
    int chaosCounter = 0;

    juce::ADSR ampEnv, fltEnv;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::Random rng;
};
