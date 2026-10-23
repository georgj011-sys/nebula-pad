#pragma once

#include <juce_core/juce_core.h>
#include <utility>
#include <vector>

// Factory presets. Values are in real units (Hz, seconds, choice index).
// Any parameter not listed falls back to its default.
// Source indices: 0 Saw>Square, 1 Pulse Sweep, 2 Formant Vox, 3 Glass Bell, 4 Organ,
// 5 Spectral Comb, 6 Hollow Sync, 7 Digital Grit, 8 Choir Air, 9 Strings Ensemble,
// 10 Shimmer Octaves, 11 Dark Drone, 12 Glass Pad, 13 Bowed Metal, 14 Vowel Morph, 15 Spectral Cloud
struct PresetDef
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> values;
};

inline const std::vector<PresetDef>& getFactoryPresets()
{
    static const std::vector<PresetDef> presets {
        { "Nebula Init", {} },

        { "Cathedral Choir", {
            { "osc1_source", 8.0f }, { "osc1_morph", 0.4f }, { "osc1_unison", 5.0f }, { "osc1_detune", 0.35f },
            { "osc2_source", 14.0f }, { "osc2_morph", 0.2f }, { "osc2_octave", 1.0f }, { "osc2_level", 0.4f },
            { "osc3_source", 10.0f }, { "osc3_level", 0.25f }, { "osc3_octave", 1.0f },
            { "cutoff", 3200.0f }, { "fenv_amt", 0.15f }, { "lfo1_rate", 0.1f }, { "lfo1_morph", 0.45f },
            { "amp_attack", 2.5f }, { "amp_release", 5.0f },
            { "reverb_size", 0.92f }, { "reverb_mix", 0.45f }, { "chorus_mix", 0.35f } } },

        { "Wild Drift", {
            { "osc1_source", 7.0f }, { "osc1_morph", 0.2f }, { "osc1_unison", 5.0f }, { "osc1_detune", 0.5f },
            { "osc2_source", 15.0f }, { "osc2_level", 0.6f }, { "osc2_morph", 0.5f },
            { "osc3_source", 6.0f }, { "osc3_level", 0.45f }, { "osc3_octave", -1.0f }, { "osc3_morph", 0.6f },
            { "chaos", 0.85f }, { "lfo1_rate", 0.7f }, { "lfo1_morph", 0.7f }, { "lfo2_rate", 0.3f }, { "lfo2_cutoff", 0.55f },
            { "reso", 0.55f }, { "cutoff", 1800.0f }, { "drive", 0.5f },
            { "delay_mix", 0.35f }, { "delay_fb", 0.65f }, { "delay_time", 0.62f }, { "reverb_mix", 0.35f } } },

        { "Glass Horizon", {
            { "osc1_source", 3.0f }, { "osc1_morph", 0.5f },
            { "osc2_source", 12.0f }, { "osc2_level", 0.5f }, { "osc2_morph", 0.6f }, { "osc2_octave", 1.0f },
            { "osc3_source", 10.0f }, { "osc3_level", 0.3f }, { "osc3_octave", 1.0f }, { "osc3_morph", 0.8f },
            { "cutoff", 7000.0f }, { "reso", 0.15f }, { "fenv_amt", 0.1f },
            { "chorus_mix", 0.5f }, { "reverb_mix", 0.5f }, { "reverb_size", 0.88f },
            { "amp_attack", 1.2f }, { "amp_release", 4.5f } } },

        { "Dark Matter", {
            { "osc1_source", 11.0f }, { "osc1_octave", -1.0f }, { "osc1_level", 0.9f }, { "osc1_morph", 0.3f },
            { "osc2_source", 13.0f }, { "osc2_level", 0.45f }, { "osc2_morph", 0.4f },
            { "osc3_source", 0.0f }, { "osc3_level", 0.35f }, { "osc3_unison", 7.0f }, { "osc3_detune", 0.25f },
            { "osc3_morph", 0.2f }, { "osc3_octave", 0.0f },
            { "cutoff", 600.0f }, { "fenv_amt", 0.5f }, { "reso", 0.4f }, { "drive", 0.45f },
            { "flt_attack", 4.0f }, { "flt_decay", 6.0f }, { "flt_sustain", 0.3f },
            { "lfo2_cutoff", 0.3f }, { "lfo2_rate", 0.05f }, { "reverb_mix", 0.35f }, { "chaos", 0.3f } } },

        { "Analog Strings", {
            { "osc1_source", 9.0f }, { "osc1_unison", 5.0f }, { "osc1_detune", 0.3f },
            { "osc2_source", 0.0f }, { "osc2_level", 0.6f }, { "osc2_unison", 7.0f }, { "osc2_detune", 0.45f },
            { "osc2_morph", 0.15f },
            { "osc3_source", 1.0f }, { "osc3_level", 0.3f }, { "osc3_octave", -1.0f }, { "osc3_morph", 0.4f },
            { "cutoff", 4200.0f }, { "fenv_amt", 0.2f }, { "reso", 0.1f },
            { "chorus_mix", 0.6f }, { "chorus_rate", 0.6f },
            { "amp_attack", 0.8f }, { "amp_release", 2.5f }, { "reverb_mix", 0.3f }, { "lfo1_morph", 0.2f } } },

        { "Frozen Breath", {
            { "osc1_source", 8.0f }, { "osc1_morph", 0.7f },
            { "osc2_source", 5.0f }, { "osc2_level", 0.5f }, { "osc2_morph", 0.4f }, { "osc2_octave", 1.0f },
            { "osc3_level", 0.0f }, { "noise", 0.35f },
            { "filter_type", 1.0f }, { "cutoff", 1800.0f }, { "reso", 0.5f },
            { "lfo2_rate", 0.08f }, { "lfo2_cutoff", 0.6f },
            { "reverb_mix", 0.6f }, { "reverb_size", 0.95f }, { "delay_mix", 0.2f },
            { "amp_attack", 3.0f }, { "amp_release", 6.0f } } },

        { "Pulsar Field", {
            { "osc1_source", 1.0f }, { "osc1_morph", 0.3f },
            { "osc2_source", 2.0f }, { "osc2_level", 0.55f }, { "osc2_morph", 0.5f },
            { "osc3_source", 4.0f }, { "osc3_level", 0.35f }, { "osc3_octave", 1.0f }, { "osc3_morph", 0.7f },
            { "lfo1_rate", 4.0f }, { "lfo1_morph", 0.5f }, { "cutoff", 3000.0f }, { "reso", 0.35f },
            { "delay_time", 0.375f }, { "delay_fb", 0.55f }, { "delay_mix", 0.4f },
            { "reverb_mix", 0.3f }, { "amp_attack", 0.4f } } },

        { "Solar Wind", {
            { "osc1_source", 15.0f }, { "osc1_morph", 0.4f }, { "osc1_unison", 7.0f }, { "osc1_detune", 0.4f },
            { "osc2_source", 10.0f }, { "osc2_level", 0.45f }, { "osc2_octave", 1.0f }, { "osc2_morph", 0.6f },
            { "osc3_source", 14.0f }, { "osc3_level", 0.4f }, { "osc3_morph", 0.5f }, { "osc3_octave", 0.0f },
            { "chaos", 0.5f }, { "lfo1_rate", 0.25f }, { "lfo1_morph", 0.6f }, { "width", 1.0f },
            { "cutoff", 5000.0f }, { "reverb_mix", 0.5f }, { "reverb_size", 0.9f }, { "delay_mix", 0.25f } } },

        { "Deep Ocean", {
            { "osc1_source", 11.0f }, { "osc1_morph", 0.6f },
            { "osc2_source", 8.0f }, { "osc2_level", 0.5f }, { "osc2_octave", -1.0f }, { "osc2_morph", 0.3f },
            { "osc3_source", 12.0f }, { "osc3_level", 0.2f }, { "osc3_octave", 1.0f },
            { "cutoff", 900.0f }, { "reso", 0.3f }, { "lfo2_rate", 0.04f }, { "lfo2_cutoff", 0.45f },
            { "reverb_mix", 0.55f }, { "reverb_size", 0.97f },
            { "amp_attack", 3.5f }, { "amp_release", 7.0f } } },
    };

    return presets;
}
