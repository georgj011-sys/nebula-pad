#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "WaveBank.h"

namespace Params
{
    inline juce::String osc (int index, const char* name)
    {
        return "osc" + juce::String (index + 1) + "_" + name;
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        AudioProcessorValueTreeState::ParameterLayout layout;

        auto skewed = [] (float lo, float hi, float centre)
        {
            NormalisableRange<float> r (lo, hi);
            r.setSkewForCentre (centre);
            return r;
        };

        auto addFloat = [&layout] (const String& id, const String& name,
                                   NormalisableRange<float> range, float def, const String& label = {})
        {
            layout.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, range, def,
                                                               AudioParameterFloatAttributes().withLabel (label)));
        };

        auto addInt = [&layout] (const String& id, const String& name, int lo, int hi, int def)
        {
            layout.add (std::make_unique<AudioParameterInt> (ParameterID { id, 1 }, name, lo, hi, def));
        };

        const NormalisableRange<float> unit (0.0f, 1.0f);

        const float defLevel[3]  = { 0.8f, 0.55f, 0.35f };
        const int   defSource[3] = { 0, 8, 10 };
        const int   defOctave[3] = { 0, 0, 1 };

        for (int i = 0; i < 3; ++i)
        {
            const String n = "Osc " + String (i + 1) + " ";
            addFloat (osc (i, "level"), n + "Level", unit, defLevel[i]);
            layout.add (std::make_unique<AudioParameterChoice> (ParameterID { osc (i, "source"), 1 }, n + "Source",
                                                                WaveBank::getSourceNames(), defSource[i]));
            addFloat (osc (i, "morph"),  n + "Morph", unit, 0.3f);
            addInt   (osc (i, "octave"), n + "Octave", -2, 2, defOctave[i]);
            addInt   (osc (i, "semi"),   n + "Semitones", -12, 12, 0);
            addFloat (osc (i, "fine"),   n + "Fine", NormalisableRange<float> (-100.0f, 100.0f), 0.0f, "ct");
            addInt   (osc (i, "unison"), n + "Unison", 1, 7, 3);
            addFloat (osc (i, "detune"), n + "Detune", unit, 0.3f);
        }

        addFloat ("noise", "Noise", unit, 0.0f);

        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "filter_type", 1 }, "Filter Type",
                                                            StringArray { "Low pass", "Band pass", "High pass" }, 0));
        addFloat ("cutoff",   "Cutoff", skewed (20.0f, 20000.0f, 1000.0f), 2500.0f, "Hz");
        addFloat ("reso",     "Resonance", unit, 0.2f);
        addFloat ("fenv_amt", "Filter Env Amount", NormalisableRange<float> (-1.0f, 1.0f), 0.3f);
        addFloat ("drive",    "Drive", unit, 0.15f);

        const auto timeRange = skewed (0.001f, 20.0f, 1.0f);
        addFloat ("amp_attack",  "Amp Attack",  timeRange, 1.5f, "s");
        addFloat ("amp_decay",   "Amp Decay",   timeRange, 2.0f, "s");
        addFloat ("amp_sustain", "Amp Sustain", unit, 0.85f);
        addFloat ("amp_release", "Amp Release", timeRange, 3.0f, "s");

        addFloat ("flt_attack",  "Filter Attack",  timeRange, 2.0f, "s");
        addFloat ("flt_decay",   "Filter Decay",   timeRange, 3.0f, "s");
        addFloat ("flt_sustain", "Filter Sustain", unit, 0.4f);
        addFloat ("flt_release", "Filter Release", timeRange, 3.0f, "s");

        const auto rateRange = skewed (0.01f, 20.0f, 1.0f);
        addFloat ("lfo1_rate",   "LFO 1 Rate", rateRange, 0.15f, "Hz");
        addFloat ("lfo1_morph",  "LFO 1 to Morph", unit, 0.35f);
        addFloat ("lfo2_rate",   "LFO 2 Rate", rateRange, 0.08f, "Hz");
        addFloat ("lfo2_cutoff", "LFO 2 to Cutoff", unit, 0.25f);
        addFloat ("chaos",       "Chaos", unit, 0.2f);
        addFloat ("width",       "Stereo Width", unit, 0.8f);

        addFloat ("chorus_rate", "Chorus Rate", skewed (0.05f, 5.0f, 0.8f), 0.4f, "Hz");
        addFloat ("chorus_mix",  "Chorus Mix", unit, 0.3f);
        addFloat ("delay_time",  "Delay Time", NormalisableRange<float> (0.05f, 2.0f), 0.45f, "s");
        addFloat ("delay_fb",    "Delay Feedback", NormalisableRange<float> (0.0f, 0.95f), 0.45f);
        addFloat ("delay_mix",   "Delay Mix", unit, 0.2f);
        addFloat ("reverb_size", "Reverb Size", unit, 0.8f);
        addFloat ("reverb_mix",  "Reverb Mix", unit, 0.35f);
        addFloat ("master",      "Master", unit, 0.7f);

        return layout;
    }
}
