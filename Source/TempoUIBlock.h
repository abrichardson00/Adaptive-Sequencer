/*
  ==============================================================================

    TempoUIBlock.h
    Created: 11 Dec 2022 7:25:01pm
    Author:  User

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

class TempoUIBlock : public juce::Component
{

public:

    /// <summary>
    /// Constructor for TempoUIBlock. Initialize and link individual components
    /// to the provided Assignment3AudioProcessor reference.
    /// </summary>
    /// <param name="_audioProcessor"> pointer to the plugin's Assignment3AudioProcessor.</param>
    TempoUIBlock(Assignment3AudioProcessor* _audioProcessor)
    {
        audioProcessor = _audioProcessor;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

        tempoAttachment = std::make_unique<SliderAttachment>(audioProcessor->parameters, "tempo", tempoSlider);
        addAndMakeVisible(tempoSlider);
        addAndMakeVisible(tempoLabel);
        tempoLabel.setText("Tempo", juce::dontSendNotification);
        tempoLabel.attachToComponent(&tempoSlider, true);
        tempoSlider.onValueChange = [this] {audioProcessor->stateHandler.setTempo(tempoSlider.getValue()); };

        addAndMakeVisible(adaptTempoToggle);
        addAndMakeVisible(adaptTempoLabel);
        adaptTempoLabel.setText("Adapt", juce::dontSendNotification);
        adaptTempoLabel.attachToComponent(&adaptTempoToggle, true);
        adaptTempoToggle.onStateChange = [this] {audioProcessor->stateHandler.setAdaptingTempo(adaptTempoToggle.getToggleState()); };

        addAndMakeVisible(sensitivitySlider);
        addAndMakeVisible(sensitivityLabel);
        sensitivityLabel.setText("Sensitivity", juce::dontSendNotification);
        sensitivityLabel.attachToComponent(&sensitivitySlider, true);
        sensitivitySlider.setRange(0.0, 20.0, 0.1);
        sensitivitySlider.setValue(audioProcessor->stateHandler.getAdaptationSpeed(), juce::dontSendNotification);
        sensitivitySlider.onValueChange = [this] {audioProcessor->stateHandler.setAdaptationSpeed(sensitivitySlider.getValue()); };

        addAndMakeVisible(biasSlider);
        addAndMakeVisible(biasLabel);
        biasLabel.setText("Bias (slower <-> faster)", juce::dontSendNotification);
        biasLabel.attachToComponent(&biasSlider, true);
        biasSlider.setRange(-1.0, 1.0, 0.01);
        biasSlider.setValue(audioProcessor->stateHandler.getAdaptationBias(), juce::dontSendNotification);
        biasSlider.onValueChange = [this] {audioProcessor->stateHandler.setAdaptationBias(biasSlider.getValue()); };
    }

    /*
    When adapting tempo, this gets called to update the slider along with the changing tempo.
    */
    void updateTempo()
    {
        tempoSlider.setValue(audioProcessor->stateHandler.getTempo());
    }

    /*
    Set the background of this custom component to black.
    */
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
    }

    /*
    Set the relative sizes/locations of the child components in this class.
    */
    void resized() override
    {
        int x = getLocalBounds().getX();
        int y = getLocalBounds().getY();
        int width = getLocalBounds().getWidth();
        int sliderLeft = (int)(width / 2);

        tempoSlider.setBounds(x + sliderLeft, y + 10, width - sliderLeft - 10, 20);
        adaptTempoToggle.setBounds(x + 60, y + 10, 20, 20);

        sensitivitySlider.setBounds(x + sliderLeft, y + 30, width - sliderLeft - 10, 20);
        biasSlider.setBounds(x + sliderLeft, y + 50, width - sliderLeft - 10, 20);
    }

private:
    Assignment3AudioProcessor* audioProcessor;

    juce::Slider tempoSlider;
    juce::Label tempoLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;
    juce::ToggleButton adaptTempoToggle;
    juce::Label adaptTempoLabel;
    juce::Slider sensitivitySlider;
    juce::Label sensitivityLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sensitivityAttachment;
    juce::Slider biasSlider;
    juce::Label biasLabel;

    //==============================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoUIBlock)
};