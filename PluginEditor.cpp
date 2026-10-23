#include "PluginEditor.h"
#include "Params.h"

//==============================================================================
NebulaLookAndFeel::NebulaLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Theme::bg);
    setColour (juce::Label::textColourId, Theme::text);

    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1b1c20));
    setColour (juce::ComboBox::outlineColourId, Theme::panelEdge);
    setColour (juce::ComboBox::textColourId, Theme::text);
    setColour (juce::ComboBox::arrowColourId, Theme::accent);
    setColour (juce::ComboBox::focusedOutlineColourId, Theme::accent);

    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1d1e22));
    setColour (juce::PopupMenu::textColourId, Theme::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::accent.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2c32));
    setColour (juce::TextButton::textColourOffId, Theme::text);

    setColour (juce::BubbleComponent::backgroundColourId, juce::Colour (0xff121316));
    setColour (juce::BubbleComponent::outlineColourId, Theme::accent.withAlpha (0.6f));
    setColour (juce::TooltipWindow::textColourId, Theme::text);
}

void NebulaLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Track
    juce::Path track;
    track.addCentredArc (cx, cy, radius - 2.0f, radius - 2.0f, 0.0f, startAngle, endAngle, true);
    g.setColour (Theme::panelEdge);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc (bipolar knobs start from the centre)
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float arcStart = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;

    if (std::abs (angle - arcStart) > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (cx, cy, radius - 2.0f, radius - 2.0f, 0.0f,
                             juce::jmin (arcStart, angle), juce::jmax (arcStart, angle), true);
        g.setColour (Theme::accent);
        g.strokePath (value, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Knob body
    const float kr = radius - 7.0f;
    if (kr <= 2.0f)
        return;

    const juce::Rectangle<float> knob (cx - kr, cy - kr, kr * 2.0f, kr * 2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4b4e57), cx, cy - kr,
                                             juce::Colour (0xff1b1c20), cx, cy + kr, false));
    g.fillEllipse (knob);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (knob, 1.2f);

    const float capR = kr * 0.72f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3c43), cx, cy - capR,
                                             juce::Colour (0xff26282d), cx, cy + capR, false));
    g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

    // Pointer
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -kr + 3.0f, 3.0f, kr * 0.45f, 1.5f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (cx, cy));
    g.setColour (slider.isMouseOverOrDragging() ? Theme::accent : Theme::text);
    g.fillPath (pointer);
}

//==============================================================================
LabeledKnob::LabeledKnob (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled (true, true, nullptr);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions (12.0f)));
    label.setColour (juce::Label::textColourId, Theme::textDim);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramID, slider);

    if (auto* p = state.getParameter (paramID))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
}

void LabeledKnob::resized()
{
    auto b = getLocalBounds();
    label.setBounds (b.removeFromBottom (16));
    slider.setBounds (b);
}

//==============================================================================
ParamCombo::ParamCombo (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& name)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (paramID)))
        box.addItemList (choice->choices, 1);
    addAndMakeVisible (box);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions (12.0f)));
    label.setColour (juce::Label::textColourId, Theme::textDim);
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, paramID, box);
}

void ParamCombo::resized()
{
    auto b = getLocalBounds();
    label.setBounds (b.removeFromBottom (16));
    box.setBounds (b.withSizeKeepingCentre (b.getWidth() - 4, juce::jmin (26, b.getHeight())));
}

//==============================================================================
WaveDisplay::WaveDisplay (const WaveBank& b, std::atomic<float>* source, std::atomic<float>* morph)
    : bank (b), sourceParam (source), morphParam (morph)
{
    startTimerHz (24);
}

void WaveDisplay::timerCallback()
{
    const float s = sourceParam->load(), m = morphParam->load();
    if (s != lastSource || m != lastMorph)
    {
        lastSource = s;
        lastMorph = m;
        repaint();
    }
}

void WaveDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff101114));
    g.fillRoundedRectangle (b, 4.0f);

    g.setColour (Theme::scope.withAlpha (0.08f));
    g.drawHorizontalLine ((int) b.getCentreY(), b.getX() + 4.0f, b.getRight() - 4.0f);

    if (lastSource < 0.0f)
        return;

    const int source = (int) lastSource;
    const float fp = juce::jlimit (0.0f, 1.0f, lastMorph) * (float) (WaveBank::numFrames - 1);
    const int f0 = (int) fp;
    const int f1 = juce::jmin (f0 + 1, WaveBank::numFrames - 1);
    const float frac = fp - (float) f0;
    const float* t0 = bank.getTable (source, f0, 0);
    const float* t1 = bank.getTable (source, f1, 0);

    const auto area = b.reduced (6.0f, 8.0f);
    const int points = juce::jmax (2, (int) area.getWidth());
    juce::Path path;

    for (int i = 0; i < points; ++i)
    {
        const int idx = i * (WaveBank::tableSize - 1) / (points - 1);
        const float v = t0[idx] + frac * (t1[idx] - t0[idx]);
        const float px = area.getX() + (float) i;
        const float py = area.getCentreY() - v * area.getHeight() * 0.48f;

        if (i == 0) path.startNewSubPath (px, py);
        else        path.lineTo (px, py);
    }

    g.setColour (Theme::scope.withAlpha (0.18f));
    g.strokePath (path, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (Theme::scope);
    g.strokePath (path, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
OscTop::OscTop (const WaveBank& bank, juce::AudioProcessorValueTreeState& state, int i)
    : display (bank,
               state.getRawParameterValue (Params::osc (i, "source")),
               state.getRawParameterValue (Params::osc (i, "morph")))
{
    addAndMakeVisible (display);

    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (Params::osc (i, "source"))))
        box.addItemList (choice->choices, 1);
    addAndMakeVisible (box);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, Params::osc (i, "source"), box);
}

void OscTop::resized()
{
    auto b = getLocalBounds();
    box.setBounds (b.removeFromBottom (24));
    b.removeFromBottom (6);
    display.setBounds (b);
}

//==============================================================================
Section::Section (juce::String t, int cols) : title (std::move (t)), columns (juce::jmax (1, cols)) {}

void Section::addKnob (juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& name)
{
    auto k = std::make_unique<LabeledKnob> (state, id, name);
    addAndMakeVisible (*k);
    items.push_back (std::move (k));
}

void Section::addCombo (juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& name)
{
    auto c = std::make_unique<ParamCombo> (state, id, name);
    addAndMakeVisible (*c);
    items.push_back (std::move (c));
}

void Section::setTop (std::unique_ptr<juce::Component> component, int height)
{
    top = std::move (component);
    topHeight = height;
    addAndMakeVisible (*top);
}

void Section::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (b, 6.0f);
    g.setColour (Theme::panelEdge);
    g.drawRoundedRectangle (b, 6.0f, 1.0f);

    auto titleArea = b.removeFromTop (28.0f).reduced (12.0f, 0.0f);
    g.setColour (Theme::accent);
    g.fillEllipse (titleArea.getX(), titleArea.getCentreY() - 3.0f, 6.0f, 6.0f);
    g.setColour (Theme::text);
    g.setFont (juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));
    g.drawText (title, titleArea.withTrimmedLeft (12.0f), juce::Justification::centredLeft);
}

void Section::resized()
{
    auto b = getLocalBounds().reduced (8);
    b.removeFromTop (22);

    if (top != nullptr)
    {
        top->setBounds (b.removeFromTop (topHeight));
        b.removeFromTop (6);
    }

    const int n = (int) items.size();
    if (n == 0)
        return;

    const int rows = (n + columns - 1) / columns;
    const int cellW = b.getWidth() / columns;
    const int cellH = b.getHeight() / rows;

    for (int i = 0; i < n; ++i)
    {
        const int r = i / columns, c = i % columns;
        items[(size_t) i]->setBounds (juce::Rectangle<int> (b.getX() + c * cellW, b.getY() + r * cellH, cellW, cellH).reduced (2));
    }
}

//==============================================================================
NebulaPadEditor::NebulaPadEditor (NebulaPadProcessor& p)
    : AudioProcessorEditor (&p),
      proc (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto& s = p.apvts;

    for (int i = 0; i < 3; ++i)
    {
        auto sec = std::make_unique<Section> ("Oscillator " + juce::String (i + 1), 4);
        sec->setTop (std::make_unique<OscTop> (p.getWaveBank(), s, i), 104);
        sec->addKnob (s, Params::osc (i, "level"),  "Level");
        sec->addKnob (s, Params::osc (i, "morph"),  "Morph");
        sec->addKnob (s, Params::osc (i, "unison"), "Unison");
        sec->addKnob (s, Params::osc (i, "detune"), "Detune");
        sec->addKnob (s, Params::osc (i, "octave"), "Octave");
        sec->addKnob (s, Params::osc (i, "semi"),   "Semi");
        sec->addKnob (s, Params::osc (i, "fine"),   "Fine");
        addAndMakeVisible (*sec);
        oscSections[i] = std::move (sec);
    }

    filterSection = std::make_unique<Section> ("Filter", 3);
    filterSection->addCombo (s, "filter_type", "Type");
    filterSection->addKnob (s, "cutoff",   "Cutoff");
    filterSection->addKnob (s, "reso",     "Resonance");
    filterSection->addKnob (s, "fenv_amt", "Env amount");
    filterSection->addKnob (s, "drive",    "Drive");
    filterSection->addKnob (s, "noise",    "Noise");

    ampSection = std::make_unique<Section> ("Amp envelope", 2);
    ampSection->addKnob (s, "amp_attack",  "Attack");
    ampSection->addKnob (s, "amp_decay",   "Decay");
    ampSection->addKnob (s, "amp_sustain", "Sustain");
    ampSection->addKnob (s, "amp_release", "Release");

    fenvSection = std::make_unique<Section> ("Filter envelope", 2);
    fenvSection->addKnob (s, "flt_attack",  "Attack");
    fenvSection->addKnob (s, "flt_decay",   "Decay");
    fenvSection->addKnob (s, "flt_sustain", "Sustain");
    fenvSection->addKnob (s, "flt_release", "Release");

    motionSection = std::make_unique<Section> ("Motion", 3);
    motionSection->addKnob (s, "lfo1_rate",   "LFO 1 rate");
    motionSection->addKnob (s, "lfo1_morph",  "LFO 1 to morph");
    motionSection->addKnob (s, "chaos",       "Chaos");
    motionSection->addKnob (s, "lfo2_rate",   "LFO 2 rate");
    motionSection->addKnob (s, "lfo2_cutoff", "LFO 2 to cutoff");
    motionSection->addKnob (s, "width",       "Width");

    fxSection = std::make_unique<Section> ("Effects", 8);
    fxSection->addKnob (s, "chorus_rate", "Chorus rate");
    fxSection->addKnob (s, "chorus_mix",  "Chorus");
    fxSection->addKnob (s, "delay_time",  "Delay time");
    fxSection->addKnob (s, "delay_fb",    "Feedback");
    fxSection->addKnob (s, "delay_mix",   "Delay");
    fxSection->addKnob (s, "reverb_size", "Space size");
    fxSection->addKnob (s, "reverb_mix",  "Reverb");
    fxSection->addKnob (s, "master",      "Master");

    for (auto* sec : { filterSection.get(), ampSection.get(), fenvSection.get(), motionSection.get(), fxSection.get() })
        addAndMakeVisible (*sec);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { selectPreset (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    prevButton.onClick = [this]
    {
        const int n = proc.getNumPrograms();
        selectPreset ((proc.getCurrentProgram() - 1 + n) % n);
    };
    nextButton.onClick = [this]
    {
        selectPreset ((proc.getCurrentProgram() + 1) % proc.getNumPrograms());
    };
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);

    keyboard.setLowestVisibleKey (36);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Theme::accent.withAlpha (0.75f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Theme::accent.withAlpha (0.25f));
    addAndMakeVisible (keyboard);

    setSize (1180, 780);
}

NebulaPadEditor::~NebulaPadEditor()
{
    setLookAndFeel (nullptr);
}

void NebulaPadEditor::selectPreset (int index)
{
    if (index < 0 || index >= proc.getNumPrograms())
        return;

    proc.setCurrentProgram (index);
    presetBox.setSelectedId (index + 1, juce::dontSendNotification);
}

void NebulaPadEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::bg);

    // Walnut side cheeks
    auto drawCheek = [&g] (juce::Rectangle<float> r, bool left)
    {
        g.setGradientFill (juce::ColourGradient (left ? Theme::wood2 : Theme::wood1, r.getX(), 0.0f,
                                                 left ? Theme::wood1 : Theme::wood2, r.getRight(), 0.0f, false));
        g.fillRect (r);

        juce::Random grain (left ? 17 : 42);
        for (int i = 0; i < 14; ++i)
        {
            const float gx = r.getX() + grain.nextFloat() * r.getWidth();
            g.setColour (juce::Colours::black.withAlpha (0.08f + grain.nextFloat() * 0.1f));
            g.drawLine (gx, r.getY(), gx + (grain.nextFloat() - 0.5f) * 4.0f, r.getBottom(), 0.8f);
        }
    };

    const auto full = getLocalBounds().toFloat();
    drawCheek (full.withWidth ((float) cheekWidth), true);
    drawCheek (full.withLeft (full.getRight() - (float) cheekWidth), false);

    // Header
    auto header = getLocalBounds().reduced (cheekWidth, 0).removeFromTop (headerHeight).toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff202227), 0.0f, header.getY(),
                                             Theme::bg, 0.0f, header.getBottom(), false));
    g.fillRect (header);

    auto textArea = header.reduced (18.0f, 0.0f);
    g.setColour (Theme::text);
    g.setFont (juce::Font (juce::FontOptions (26.0f).withStyle ("Bold")));
    g.drawText ("Nebula", textArea, juce::Justification::centredLeft);

    g.setColour (Theme::accent);
    g.setFont (juce::Font (juce::FontOptions (26.0f)));
    g.drawText ("Pad", textArea.withTrimmedLeft (98.0f), juce::Justification::centredLeft);

    g.setColour (Theme::textDim);
    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText ("Three-oscillator morphing pad synthesizer", textArea.withTrimmedLeft (156.0f),
                juce::Justification::centredLeft);
}

void NebulaPadEditor::resized()
{
    auto b = getLocalBounds().reduced (cheekWidth, 0);

    auto header = b.removeFromTop (headerHeight).reduced (16, 14);
    nextButton.setBounds (header.removeFromRight (30));
    header.removeFromRight (4);
    presetBox.setBounds (header.removeFromRight (240));
    header.removeFromRight (4);
    prevButton.setBounds (header.removeFromRight (30));

    auto content = b.reduced (10, 6);
    keyboard.setBounds (content.removeFromBottom (78));
    content.removeFromBottom (8);

    auto oscRow = content.removeFromTop (300);
    const int oscW = (oscRow.getWidth() - 16) / 3;
    for (int i = 0; i < 3; ++i)
    {
        oscSections[i]->setBounds (oscRow.removeFromLeft (oscW));
        if (i < 2)
            oscRow.removeFromLeft (8);
    }

    content.removeFromTop (8);
    auto midRow = content.removeFromTop (190);
    const int usable = midRow.getWidth() - 24;
    filterSection->setBounds (midRow.removeFromLeft ((int) (usable * 0.32f)));
    midRow.removeFromLeft (8);
    ampSection->setBounds (midRow.removeFromLeft ((int) (usable * 0.18f)));
    midRow.removeFromLeft (8);
    fenvSection->setBounds (midRow.removeFromLeft ((int) (usable * 0.18f)));
    midRow.removeFromLeft (8);
    motionSection->setBounds (midRow);

    content.removeFromTop (8);
    fxSection->setBounds (content);
}
