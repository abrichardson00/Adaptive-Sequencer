/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Assignment3AudioProcessor::Assignment3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    parameters(*this, nullptr, "ParamTree",
        {
        std::make_unique<juce::AudioParameterFloat>("window_duration", "Window Duration", 0.01, 0.18, 0.05),
        std::make_unique<juce::AudioParameterFloat>("event_duration", "Event Duration", 0.05, 1.0, 0.2),
        std::make_unique<juce::AudioParameterFloat>("detection_threshold", "Detection Threshold", -5.0, 10.0, 3.0),
        std::make_unique<juce::AudioParameterFloat>("release_detection_threshold", "Release Detection Threshold", -5.0, 10.0, 3.0),
        std::make_unique<juce::AudioParameterFloat>("event_on_beat_bias", "Event on beat bias", 0.0, 1.0, 1.0),
        std::make_unique<juce::AudioParameterFloat>("tempo", "Tempo", 10, 200, 90)
        })
{
    windowDurationParameter = parameters.getRawParameterValue("window_duration");
    eventDurationParameter = parameters.getRawParameterValue("event_duration");
    detectionThresholdParameter = parameters.getRawParameterValue("detection_threshold");
    releaseDetectionThresholdParameter = parameters.getRawParameterValue("release_detection_threshold");
    eventOnBeatBiasParameter = parameters.getRawParameterValue("event_on_beat_bias");
    tempoParameter = parameters.getRawParameterValue("tempo");
}


Assignment3AudioProcessor::~Assignment3AudioProcessor()
{
}

//==============================================================================
const juce::String Assignment3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Assignment3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Assignment3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Assignment3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Assignment3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Assignment3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Assignment3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Assignment3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Assignment3AudioProcessor::getProgramName (int index)
{
    return {};
}

void Assignment3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Assignment3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    eventDetector.initialize(sampleRate, samplesPerBlock, 0.2f, 0.2f, 4, 1.0f); 
      
    /*
    This if-statement seems like a cheeky work-around that should be improved at a later date.
    But for now this is by far the cleanest solution to not clear any Sequence changes made in 
    // the UI. The other approach would be editing all the initialize functions to not 
    re-initialize specific things, which at the moment seems worse to me.
    */
    if (hasPrepareToPlayBeenCalledOnce)
    {
        // maybe prepareToPlay was called because sampleRate were changed...
        stateHandler.setSampleRate(sampleRate);
    }
    else
    {
        // otherwise: time to initialize everything:

        float tempo = *tempoParameter;
        stateHandler.initialize(sampleRate, tempo, &eventDetector);
        hasPrepareToPlayBeenCalledOnce = true;

        // below: - initialize sequence objects
        //        - set startinf off state (which sequence(s) are on?)
        //        - initalize and add TransitionRule objects to stateHandler


        // ========================
        // initial Sequence objects

        // kick patterns --------------------------
        sequence1.initialize(51, 60, 1, 4);
        sequence1.setPatternValue(0, true);
        sequence1.setPatternValue(3, true);
        stateHandler.addSequence(&sequence1);

        
        // snare patterns -------------------------

        // 00001000
        sequence2.initialize(39, 100, 2, 4);
        sequence2.setPatternValue(4, true);
        //sequence2.setPatternValue(7, true);
        stateHandler.addSequence(&sequence2);

        // 0101010001010010
        sequence3.initialize(38, 10, 4, 4);
        sequence3.setPatternValue(1, true);
        sequence3.setPatternValue(3, true);
        sequence3.setPatternValue(5, true);
        sequence3.setPatternValue(9, true);
        sequence3.setPatternValue(11, true);
        sequence3.setPatternValue(14, true);
        stateHandler.addSequence(&sequence3);


        // ========================================
        // hihat patterns -------------------------

        sequence4.initialize(42, 100, 4, 2);
        sequence4.setPatternValue(0, true);
        sequence4.setPatternValue(2, true);
        sequence4.setPatternValue(4, true);
        sequence4.setPatternValue(6, true);
        stateHandler.addSequence(&sequence4);

        sequence5.initialize(46, 110, 2, 2);
        sequence5.setPatternValue(1, true);
        sequence5.setPatternValue(3, true);
        stateHandler.addSequence(&sequence5);

        // low tom pattern ---------------------------
        sequence6.initialize(41, 110, 1, 4);
        sequence6.setPatternValue(0, true);
        sequence6.setPatternValue(2, true);
        sequence6.setPatternValue(3, true);
        stateHandler.addSequence(&sequence6);

        // ride ----------------------------
        sequence7.initialize(53, 110, 1, 2);
        sequence7.setPatternValue(0, true);
        sequence7.setPatternValue(1, true);
        stateHandler.addSequence(&sequence7);


        // crash pattern -------------------------- 
        sequence8.initialize(52, 110, 1, 2);
        sequence8.setPatternValue(0, true);
        stateHandler.addSequence(&sequence8);

        // ==============================================================
        // after sequences added, need to turn an initial sequence on:

        stateHandler.setState(3, StateHandler::State::on);

        // =============================
        // now need some transition rules:
        // =============================

        // 0: ride   | if decreasing amplitude OR very loud
        // 1: snare1 | if (event density > threshold) OR loud
        // 2: snare2 | not loud
        // 3: hihat1 | always on
        // 4: hihat2 | if (event density > threshold) OR (loud and not decreasing amplitude)
        // 5: tom    | (on beat 2 and loud [one way turn on]) OR (event density > threshold)
        // 6: ride2  | if loud and not decreasing amplitude
        // 7: china  | if very loud

        //      ^^ almost drum samples used for recording 1 (except recording 1 had 0: kick)
        //         These are varied for recording 2, but the sequence transition rules themselves 
        //         stay the same.

        // if event density < threshold, play hihat on event too
        // play china too if loud and event on off-beats e.g. 1,3 of 0,1,2,3

        using E = TransitionRule::Effect;

        // ====================================
        // initialize transition detectors:
        eventDensityTransition1.initialize(&stateHandler, { 2, 4 }, { E::turnOff, E::turnOn }, false);
        eventDensityTransition1.setThreshold(0.8f);
        eventDensityTransition2.initialize(&stateHandler, { 5 }, { E::turnOn }, false);
        eventDensityTransition2.setThreshold(1.3f);

        amplitudeTransition1.initialize(&stateHandler, { 1 }, { E::turnOn }, false);
        amplitudeTransition1.setThreshold(0.04f);
        amplitudeTransition2.initialize(&stateHandler, { 7 }, { E::turnOn }, false);
        amplitudeTransition2.setThreshold(0.07f);

        eventOnBeatTransition1.initialize(&stateHandler, {}, {}, false);
        eventOnBeatTransition1.setBeatAndThreshold(0, 1, 1, 2, 0.3f);
        eventOnBeatTransition2.initialize(&stateHandler, {}, {}, false);
        eventOnBeatTransition2.setBeatAndThreshold(1, 0, 2, 2, 0.3f);
  
        decreasingAmplitudeTransition1.initialize(&stateHandler, { 0 }, { E::turnOn }, false);
        decreasingAmplitudeTransition1.setDecreaseTransition(true);
        decreasingAmplitudeTransition1.setLookBack(0.3f);

        notDecreasingAmplitudeTransition1.initialize(&stateHandler, { 0 }, { E::turnOn }, false);
        notDecreasingAmplitudeTransition1.setLookBack(0.3f);

        
        // ====================================================================
        // any composing with NOT, OR, AND operations, then add to stateHandler

        or1.initialize(&amplitudeTransition2, &decreasingAmplitudeTransition1, { 0 }, { E::turnOn }, false);
        stateHandler.addTransitionRule(&or1); // 0
        or2.initialize(&eventDensityTransition1, &amplitudeTransition1, { 1 }, { E::turnOn }, false);
        stateHandler.addTransitionRule(&or2); // 1
        not1.initialize(&amplitudeTransition1, { 2 }, { E::turnOn }, false);
        stateHandler.addTransitionRule(&not1); // 2
        // ... 3 -> always on
        and1.initialize(&amplitudeTransition1, &notDecreasingAmplitudeTransition1, { 6 }, { E::turnOn }, false);
        or3.initialize(&and1, &eventDensityTransition1, { 4 }, { E::turnOn }, false);
        stateHandler.addTransitionRule(&or3); // 4
        and3.initialize(&amplitudeTransition1, &eventOnBeatTransition2, {}, {}, true);
        stateHandler.addTransitionRule(&and3); // 5
        stateHandler.addTransitionRule(&eventDensityTransition2);
        stateHandler.addTransitionRule(&and1); // 6        
        stateHandler.addTransitionRule(&amplitudeTransition2); // 7

        // ====================================
        // event detected midi note transitions
       
        // china on with kick 
        and4.initialize(&eventOnBeatTransition1, &amplitudeTransition1, {}, {}, false);
        setTriggerMidiTransition1.initialize(&stateHandler, &and4, false, { 2 }, { true }, false);
        stateHandler.addTransitionRule(&setTriggerMidiTransition1);

        // open hihat on with kick
        not2.initialize(&eventDensityTransition1, {}, {}, false);
        setTriggerMidiTransition2.initialize(&stateHandler, &not2, false, { 1 }, { true }, false);
        stateHandler.addTransitionRule(&setTriggerMidiTransition2);

        // release event midi snare on: volume above threshold and event not directly on beat 2
        not3.initialize(&eventOnBeatTransition2, {}, {}, false);
        and5.initialize(&not3, &amplitudeTransition1, {}, {}, false);
        setTriggerMidiTransition3.initialize(&stateHandler, &and5, true, { 0 }, { true }, false);
        stateHandler.addTransitionRule(&setTriggerMidiTransition3);
    }
}

void Assignment3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Assignment3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Assignment3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    

    eventDetector.setWindowDuration(*windowDurationParameter);
    eventDetector.setEventDuration(*eventDurationParameter);
    eventDetector.setDetectionThreshold(*detectionThresholdParameter);
    eventDetector.setReleaseDetectionThreshold(*releaseDetectionThresholdParameter);
    eventDetector.setEventOnBeatBias(*eventOnBeatBiasParameter);
    // stateHandler.setTempo(*tempoParameter); // <- doesn't work if wanting to adapt tempo

    int numSamples = buffer.getNumSamples();
    // for now just assuming mono input on first channel
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    // update stuff:
    eventDetector.pushAudioBufferIntoBigBuffer(leftChannel, numSamples);
    eventDetector.detectHit();
    stateHandler.updateState();
    stateHandler.updateTempo();
    stateHandler.updateSequences(numSamples); 
    

    // create midi outputs for when rhythmic events are detected
    if (eventDetector.getEventOccurring())
    {
        for (int i = 0; i < stateHandler.getEventMidiValues()->size(); i++)
        {
            if (stateHandler.getEventMidiValuesOn(i))
            {
                
                int midiValue = (*stateHandler.getEventMidiValues())[i];
                juce::uint8 midiVelocity = (*stateHandler.getEventMidiVelocities())[i];
                //DBG("eventMidi index: " << i);
                auto noteOnMessage = juce::MidiMessage::noteOn(1, midiValue, midiVelocity);
                auto noteOffMessage = juce::MidiMessage::noteOff(1, midiValue, midiVelocity);

                midiMessages.addEvent(noteOnMessage, 0);
                midiMessages.addEvent(noteOffMessage, 1);
            } 
        }
        
    }

    // midi events for release events
    if (eventDetector.getEventReleaseOccurring())
    {
        for (int i = 0; i < stateHandler.getEventReleaseMidiValues()->size(); i++)
        {
            if (stateHandler.getEventReleaseMidiValuesOn(i))
            {

                int midiValue = (*stateHandler.getEventReleaseMidiValues())[i];
                juce::uint8 midiVelocity = (*stateHandler.getEventReleaseMidiVelocities())[i];
                //DBG("eventMidi index: " << i);
                auto noteOnMessage = juce::MidiMessage::noteOn(1, midiValue, midiVelocity);
                auto noteOffMessage = juce::MidiMessage::noteOff(1, midiValue, midiVelocity);

                midiMessages.addEvent(noteOnMessage, 0);
                midiMessages.addEvent(noteOffMessage, 1);
            }
        }
    }

    // generate the sequencer midi outputs
    for (int i = 0; i < stateHandler.getNumSequences(); i++)
    {
        if (stateHandler.getSequencePtr(i)->isOutputting())
        {
            int midiValue = stateHandler.getSequencePtr(i)->getMidiValue();
            juce::uint8 midiVelocity = stateHandler.getSequencePtr(i)->getMidiVelocity();
            
            auto noteOnMessage = juce::MidiMessage::noteOn(1, midiValue, midiVelocity); 
            auto noteOffMessage = juce::MidiMessage::noteOff(1, midiValue, midiVelocity);

            midiMessages.addEvent(noteOnMessage, 0);
            midiMessages.addEvent(noteOffMessage, 1);
        }
        
    }
}

//==============================================================================
bool Assignment3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Assignment3AudioProcessor::createEditor()
{
    return new Assignment3AudioProcessorEditor(*this);
    //return new juce::GenericAudioProcessorEditor(*this); // Assignment3AudioProcessorEditor (*this);
}

//==============================================================================
void Assignment3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Assignment3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Assignment3AudioProcessor();
}
