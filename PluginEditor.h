#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

namespace Theme
{
    inline const juce::Colour bg        { 0xff17181c };
    inline const juce::Colour panel     { 0xff23252b };
    inline const juce::Colour panelEdge { 0xff363941 };
    inline const juce::Colour accent    { 0xffe9a23b }; // warm amber, like hardware LEDs
    inline const juce::Colour scope     { 0xff57c7c2 }; // phosphor teal for wave displays
    inline const juce::Colour text      { 0xffdad4c6 };
    inline const juce::Colour textDim   { 0xff8f8b82 };
    inline const juce::Colour wood1     { 0xff6b4327 };
    inline const juce::Colour wood2     { 0xff3d2415 };
}

class NebulaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NebulaLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (13.0f)); }
    juce::Font getPopupMenuFont() override               { return juce::Font (juce::FontOptions (14.0f)); }
};

class LabeledKnob : public juce::Component
{
public:
    LabeledKnob (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& name);
    void resized() override;

private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class ParamCombo : public juce::Component
{
public:
    ParamCombo (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& name);
    void resized() override;

private:
    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class WaveDisplay : public juce::Component, private juce::Timer
{
public:
    WaveDisplay (const WaveBank& bank, std::atomic<float>* source, std::atomic<float>* morph);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    const WaveBank& bank;
    std::atomic<float>* sourceParam;
    std::atomic<float>* morphParam;
    float lastSource = -1.0f, lastMorph = -1.0f;
};

class OscTop : public juce::Component
{
public:
    OscTop (const WaveBank& bank, juce::AudioProcessorValueTreeState& state, int oscIndex);
    void resized() override;

private:
    WaveDisplay display;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class Section : public juce::Component
{
public:
    Section (juce::String title, int columns);

    void addKnob (juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& name);
    void addCombo (juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& name);
    void setTop (std::unique_ptr<juce::Component> component, int height);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::String title;
    int columns;
    std::unique_ptr<juce::Component> top;
    int topHeight = 0;
    std::vector<std::unique_ptr<juce::Component>> items;
};

class NebulaPadEditor : public juce::AudioProcessorEditor
{
public:
    explicit NebulaPadEditor (NebulaPadProcessor&);
    ~NebulaPadEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void selectPreset (int index);

    static constexpr int cheekWidth = 22;
    static constexpr int headerHeight = 56;

    NebulaLookAndFeel lnf;
    NebulaPadProcessor& proc;

    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" };

    std::unique_ptr<Section> oscSections[3];
    std::unique_ptr<Section> filterSection, ampSection, fenvSection, motionSection, fxSection;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NebulaPadEditor)
};
