/*
  ==============================================================================

    EventDetectorUIBlock.h
    Created: 11 Dec 2022 7:25:17pm
    Author:  User

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

class EventDetectorUIBlock : public juce::Component
{
public:

    /// <summary>
    /// Constructor for EventDetectorUIBlock. Initialize and link individual components
    /// to the provided Assignment3AudioProcessor reference.
    /// </summary>
    /// <param name="_audioProcessor"> pointer to the plugin's Assignment3AudioProcessor.</param>
    EventDetectorUIBlock(Assignment3AudioProcessor* _audioProcessor)
    {
        audioProcessor = _audioProcessor;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        
        windowDurationAttachment = std::make_unique<SliderAttachment>(audioProcessor->parameters, "window_duration", windowDurationSlider);
        addAndMakeVisible(windowDurationSlider);
        addAndMakeVisible(windowDurationLabel);
        windowDurationLabel.setText("Window Duration", juce::dontSendNotification);
        windowDurationLabel.attachToComponent(&windowDurationSlider, true);

        eventDurationAttachment = std::make_unique<SliderAttachment>(audioProcessor->parameters, "event_duration", eventDurationSlider);
        addAndMakeVisible(eventDurationSlider);
        addAndMakeVisible(eventDurationLabel);
        eventDurationLabel.setText("Event Duration", juce::dontSendNotification);
        eventDurationLabel.attachToComponent(&eventDurationSlider, true);

        detectionThresholdAttachment = std::make_unique<SliderAttachment>(audioProcessor->parameters, "detection_threshold", detectionThresholdSlider);
        addAndMakeVisible(detectionThresholdSlider);
        addAndMakeVisible(detectionThresholdLabel);
        detectionThresholdLabel.setText("Detection Threshold", juce::dontSendNotification);
        detectionThresholdLabel.attachToComponent(&detectionThresholdSlider, true);

        releaseDetectionThresholdAttachment = std::make_unique<SliderAttachment>(audioProcessor->parameters, "release_detection_threshold", releaseDetectionThresholdSlider);
        addAndMakeVisible(releaseDetectionThresholdSlider);
        addAndMakeVisible(releaseDetectionThresholdLabel);
        releaseDetectionThresholdLabel.setText("Release Detection Threshold", juce::dontSendNotification);
        releaseDetectionThresholdLabel.attachToComponent(&releaseDetectionThresholdSlider, true);

        eventOnBeatBiasAttachment = std::make_unique<SliderAttachment>(audioProcessor->parameters, "event_on_beat_bias", eventOnBeatBiasSlider);
        addAndMakeVisible(eventOnBeatBiasSlider);
        addAndMakeVisible(eventOnBeatBiasLabel);
        eventOnBeatBiasLabel.setText("Event-on-beat bias", juce::dontSendNotification);
        eventOnBeatBiasLabel.attachToComponent(&eventOnBeatBiasSlider, true);
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
        int halfWidth = (int)(getLocalBounds().getWidth() / 2);
       
        windowDurationSlider.setBounds(halfWidth, y + 10, halfWidth, 20);
        eventDurationSlider.setBounds(halfWidth, y + 30, halfWidth, 20);
        detectionThresholdSlider.setBounds(halfWidth, y + 50, halfWidth, 20);
        releaseDetectionThresholdSlider.setBounds(halfWidth, y + 70, halfWidth, 20);
        eventOnBeatBiasSlider.setBounds(halfWidth, y + 90, halfWidth, 20);

    }

private:
    Assignment3AudioProcessor* audioProcessor;

    juce::Slider windowDurationSlider;
    juce::Label windowDurationLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> windowDurationAttachment;

    juce::Slider eventDurationSlider;
    juce::Label eventDurationLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eventDurationAttachment;

    juce::Slider detectionThresholdSlider;
    juce::Label detectionThresholdLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detectionThresholdAttachment;

    juce::Slider releaseDetectionThresholdSlider;
    juce::Label releaseDetectionThresholdLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseDetectionThresholdAttachment;

    juce::Slider eventOnBeatBiasSlider;
    juce::Label eventOnBeatBiasLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eventOnBeatBiasAttachment;
};