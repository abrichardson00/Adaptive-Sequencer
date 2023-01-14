/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EventDetector.h"
#include "StateHandler.h"
#include "Sequence.h"
#include "CustomTransitionRules.h"


//==============================================================================
/**
*/
class Assignment3AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Assignment3AudioProcessor();
    ~Assignment3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    //====================================================================================
    // I have a couple of public variables here, because it feels weirder to make
    // getters/setters for them in the PluginProcessor file. 
    // They do need to be accessed by my UI components, so for now I'm making them public:
    
    juce::AudioProcessorValueTreeState parameters;
    StateHandler stateHandler;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Assignment3AudioProcessor)

    // use this to make sure prepareToPlay only initializes 
    // certain things when called for the first time.
    bool hasPrepareToPlayBeenCalledOnce = false;
    
    // parameter stuff:

    std::atomic<float>* windowDurationParameter;
    std::atomic<float>* eventDurationParameter;
    std::atomic<float>* detectionThresholdParameter;
    std::atomic<float>* releaseDetectionThresholdParameter;
    std::atomic<float>* eventOnBeatBiasParameter;
    std::atomic<float>* tempoParameter;

    // ====================================
    // all the relevent custom class stuff:

    EventDetector eventDetector;

    EventDensityTransition eventDensityTransition1;
    EventDensityTransition eventDensityTransition2;
    MeanAmplitudeTransition amplitudeTransition1;
    MeanAmplitudeTransition amplitudeTransition2;
    DecreasingAmplitudeTransition decreasingAmplitudeTransition1;
    DecreasingAmplitudeTransition notDecreasingAmplitudeTransition1;
    SetTriggerMidiTransition setTriggerMidiTransition1;
    SetTriggerMidiTransition setTriggerMidiTransition2;
    SetTriggerMidiTransition setTriggerMidiTransition3;
    EventOnBeatTransition eventOnBeatTransition1;
    EventOnBeatTransition eventOnBeatTransition2;

    AndTransition and1;
    AndTransition and2;
    AndTransition and3;
    AndTransition and4;
    AndTransition and5;
    OrTransition or1;
    OrTransition or2;
    OrTransition or3;
    OrTransition or4;
    NotTransition not1;
    NotTransition not2;
    NotTransition not3;

    Sequence sequence1;
    Sequence sequence2;
    Sequence sequence3;
    Sequence sequence4;
    Sequence sequence5;
    Sequence sequence6;
    Sequence sequence7;
    Sequence sequence8;
};
