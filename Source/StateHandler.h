/*
  ==============================================================================

    StateHandler.h
    Created: 2 Dec 2022 10:34:32pm
    Author:  User

  ==============================================================================
*/

#pragma once

#include "EventDetector.h"
#include <vector>
#include "Sequence.h"
#include "TransitionRule.h"
#include <JuceHeader.h>


/*
A StateHandler object maintains a list of sequencesand transition rules, and at every 
update it updates a global "beatPosition" for the sequences, and checks and
applies any effects from  the list of transition rules.
*/
class StateHandler {
public:

    /*
    I've decided to use an enumeration here for representing Sequence states. 
    Defining a class for it seems like over-kill considering how many classes I
    already have. But referring to these enum values in StateHandler code helps
    keep things readable.
    */
    enum State { turningOff = -1, off = 0, turningOn = 1, on = 2 };
    
    /// <summary>
    /// Initialize member variables of the StateHandler. For now, be careful not to call this more than once.
    /// </summary>
    /// <param name="_sampleRate"> the sample rate the plugin is using.</param>
    /// <param name="_tempo"> the tempo of which to update the beatPosition.</param>
    /// <param name="_eventDetector"> a pointer to an initialized EventDetector object.</param>
    void initialize(float _sampleRate, float _tempo, EventDetector* _eventDetector);
    
    /// <summary>
    /// Updates the sampleRate variable and the beatsPerSample variable
    /// which depends on sampleRate.
    /// </summary>
    /// <param name="_sampleRate"> the new sample rate. </param>
    void setSampleRate(float _sampleRate);

    /// <summary>
    /// Set a state of a sequence (i.e. set it to on, off, turningOn or turningOff)
    /// </summary>
    /// <param name="index"> which sequence to affect.</param>
    /// <param name="state"> the new state for the sequence.</param>
    void setState(int index, State state);

    /// <summary>
    /// Called by each SequenceUIBlock object as part of animating the UI
    /// to show which Sequence objects are currently active.
    /// </summary>
    /// <param name="index"> which Sequence in the StateHandler's vector.</param>
    /// <returns> the colour (red, orange or green) for on, off or turningOn/Off. </returns>
    juce::Colour getStateColour(int index);

    // ==================================
    // some getters/setters for midi stuff

    void setEventMidiValue(int index, int midiVal);
    std::vector<int>* getEventMidiValues();
    void setEventMidiVelocity(int index, int _eventMidiVelocity);
    std::vector<int>* getEventMidiVelocities();
    void setEventMidiValueOn(int index, bool on);
    bool getEventMidiValuesOn(int index);

    void setEventReleaseMidiValue(int index, int midiVal);
    std::vector<int>* getEventReleaseMidiValues();
    void setEventReleaseMidiVelocity(int index, int _eventReleaseMidiVelocity);
    std::vector<int>* getEventReleaseMidiVelocities();
    void setEventReleaseMidiValueOn(int index, bool on);
    bool getEventReleaseMidiValuesOn(int index);

    // ==================================

    /// <summary>
    /// Add a sequence to the StateHandler (part of the 
    /// initialization process)
    /// </summary>
    /// <param name="sequencePtr"> pointer to a Sequence object.</param>
    void addSequence(Sequence* sequencePtr);

    /// <summary>
    /// Add a TransitionRule to the StateHandler (part of 
    /// the initialization process)
    /// </summary>
    /// <param name="transitionRulePtr"> pointer to the TransitionRule object to add.</param>
    void addTransitionRule(TransitionRule* transitionRulePtr);

    /*
    Getter for the stored EventDetector pointer. Handy to allow TransitionRule
    objects to access it, since they all store a pointer to the StateHandler.
    */
    EventDetector* getEventDetectorPtr();

    /// <summary>
    /// Apply a provided effect, handling any logic, e.g. can't turnOff if state already off.
    /// </summary>
    /// <param name="i"> which sequence state to affect </param>
    /// <param name="effect"> the effect to apply </param>
    void applyEffect(int i, TransitionRule::Effect effect);

    /// <summary>
    /// Undoes an applied effect (i.e. applying the opposite effect to the one provided, 
    /// handling any logic like don't turnOff if state already off etc.).
    /// </summary>
    /// <param name="i">: which sequence state to affect </param>
    /// <param name="effect">: the effect to undo </param>
    void undoEffect(int i, TransitionRule::Effect effect);

    /*
    Loops through all the TransitionRule objects and applies effects to states
    if TransitionRule.triggered() is true.
    Also undoes the applied effect if not triggered, depending though on whether a
    TransitionRule is 'one-way' or not.
    */
    void updateState();

    /// <summary>
    /// Updates the global beatPosition variable for all the Sequence objects.
    /// </summary>
    /// <param name="numSamples"> the number of samples the plugin is processing </param>
    void updateSequences(int numSamples);

    // ===========================================================
    // some tempo stuff: getters / setters and tempo adaptation...
 
    void setTempo(float _tempo);
    float getTempo();
    bool isAdaptingTempo();
    void setAdaptingTempo(bool _adaptingTempo);
    
    float getAdaptationSpeed();
    void setAdaptationSpeed(float _adaptationSpeed);
    float getAdaptationBias();
    void setAdaptationBias(float _adaptationBias);

    /// <summary>
    /// Function which adapts the tempo of StateHandler if an event is occurring.
    /// </summary>
    void updateTempo();

    /// <summary>
    /// Computes the distance of current beatPosition to a specified beat
    /// </summary>
    /// <param name="beat"> beat number </param>
    /// <param name="subBeat"> sub beat number </param>
    /// <param name="numBeats"> total number of beats </param>
    /// <param name="numSubBeats"> total number of sub-beats being considered </param>
    /// <returns></returns>
    float distToBeat(int beat, int subBeat, int numBeats, int numSubBeats);

    // ==================
    // some more getters:

    int getNumSequences();
    Sequence* getSequencePtr(int seqIndex);
    float getBeatPosition();  

private:
    
    // maintaining / updating beat position
    float sampleRate;
    float tempo;
    float beatsPerSample;
    float beatPosition;
    const int maxNumBeats = 64;

    // for tempo adaptation:
    bool adaptingTempo = false;
    float adaptationSpeed = 5.0f;
    float adaptationBias = 0.0f;
    int subBeatsConsidered = 4;

    // sequences
    int numSequences;
    std::vector<Sequence*> sequences;
    std::vector<State> states;
    int numTransitionRules;
    std::vector<TransitionRule*> transitionRules;

    // the event detector
    EventDetector *eventDetector;
    std::vector<int> eventMidiValues = { 36, 46, 52 };
    std::vector<bool> eventMidiValuesOn = { true, false, false };
    std::vector<int> eventMidiVelocities = { 110, 110, 110} ;
    std::vector<int> eventReleaseMidiValues = { 39, 53 };
    std::vector<bool> eventReleaseMidiValuesOn = { true, false };
    std::vector<int> eventReleaseMidiVelocities = { 110, 110 };

};

