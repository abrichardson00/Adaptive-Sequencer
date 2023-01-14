/*
  ==============================================================================

    CustomTransitionRules.h
    Created: 9 Dec 2022 1:23:32pm
    Author:  User

    A range of child classes are defined here which inherit from TransitionRule.

  ==============================================================================
*/

#pragma once

#include "TransitionRule.h"
#include "StateHandler.h"

/*
Transition if the density of detected events (i.e. the frequency 
of detected events) is above a certain threshold.
*/
class EventDensityTransition : public TransitionRule
{
public:

    void setThreshold(float _threshold)
    {
        threshold = _threshold;
    }

    bool triggered() override
    {
        return (stateHandler->getEventDetectorPtr()->getDensity() > threshold);
    }

private:
    float threshold;

};

/*
Transition if the mean volume level estimate is greater than a provided threshold.
*/
class MeanAmplitudeTransition : public TransitionRule
{
public:

    void setThreshold(float _threshold)
    {
        threshold = _threshold;
    }

    bool triggered() override
    {
        return (stateHandler->getEventDetectorPtr()->getAverageVolume() > threshold);
    }

private:
    float threshold;
};

/*
Transitions if CONSTANTLY DECREASING for a set amount of time.
Be wary that [NOT constantly decreasing] does not imply [constantly increasing].
*/
class DecreasingAmplitudeTransition : public TransitionRule
{
public:

    /*
    Can switch the role around - transition on increase or decrease.
    */
    void setDecreaseTransition(bool _transitionOnDecrease)
    {
        transitionOnDecrease = _transitionOnDecrease;
    }

    /*
    Set how far back in the input audio we are considering when 
    calculating whether decreasing or not.
    */
    void setLookBack(float _lookBack)
    {
        lookBack = _lookBack;
    }

    bool triggered() override
    {
        if (transitionOnDecrease) return (stateHandler->getEventDetectorPtr()->isVolumeDecreasing(lookBack));
        else return !(stateHandler->getEventDetectorPtr()->isVolumeDecreasing(lookBack));
    }

private:
    bool transitionOnDecrease = true;
    float lookBack;
};

/*
Transition if a EventDetector detected event is within a threshold distance of a certain beat.
*/
class EventOnBeatTransition : public TransitionRule
{
public:

    /// <summary>
    /// Set some extra necessary variables for this transition.
    /// </summary>
    /// <param name="_beat"> which beat we're considering.</param>
    /// <param name="_subBeat"> which sub-beat we're considering.</param>
    /// <param name="_numBeats"> the total number of beats in the pattern we're assuming</param>
    /// <param name="_numSubBeats"> the total number of sub-beats we're assuming. </param>
    /// <param name="_threshold"> how close is an event to the beat to be considered 'on the beat'?</param>
    void setBeatAndThreshold(int _beat, int _subBeat, int _numBeats, int _numSubBeats, float _threshold)
    {
        beat = _beat;
        subBeat = _subBeat;
        numBeats = _numBeats;
        numSubBeats = _numSubBeats;
        threshold = _threshold;
    }

    /*
    Returns true if beat position is within a threshold distance of desired beat,
    and either a event or release-event is occurring.
    */
    bool triggered() override
    {
        float dist = stateHandler->distToBeat(beat, subBeat, numBeats, numSubBeats);
        if (dist < threshold)
        {
            return stateHandler->getEventDetectorPtr()->getEventOccurring() 
                || stateHandler->getEventDetectorPtr()->getEventReleaseOccurring();
        }
        else return false;

    }

private:
    int beat;
    int subBeat;
    int numBeats;
    int numSubBeats;
    float threshold;
};


/*
A transition rule to compose on top of another existing transition rule.
Doesn't set sequences on/off, but changes what midi note is triggered when an 'event' is detected.

The triggered() always returns false, but is still always called by a StateHandler and changes the midi note as
a function side effect (which feels a bit cheeky, but at the moment it's the cleanest approach without having to rework everything).

Midi note transitions happen instantly, and to be noticable on the next event detected, setting oneWayTransition to true may be necessary.
*/
class SetTriggerMidiTransition : public TransitionRule
{
public:

    /// <summary>
    /// This TransitionRule has it's own custom initialize function.
    /// </summary>
    /// <param name="_stateHandler"> pointer to the plugin's StateHandler object.</param>
    /// <param name="_transition"> an existing TransitionRule object to use for the triggered() criteria.</param>
    /// <param name="_eventRelease"> Are we changing the release-event midi output or the rhythmic event-on output?</param>
    /// <param name="_midiIndices"> which of the possible midi outputs are we turning on/off.</param>
    /// <param name="_midiStates"> on/off (true/false) vector of values.</param>
    /// <param name="_oneWayTransition"> is this a one way transition - i.e. we won't undo any effects when triggered() returns false?</param>
    void initialize(StateHandler* _stateHandler, TransitionRule* _transition, bool _eventRelease, std::vector<int> _midiIndices, std::vector<bool> _midiStates, bool _oneWayTransition)
    {
        stateHandler = _stateHandler;
        transition = _transition;
        eventRelease = _eventRelease;
        midiIndices = _midiIndices;
        midiStates = _midiStates;
        oneWayTransition = _oneWayTransition;
    }

    /*
    Always returns false, but changes the StateHandler's midi value which is used whenever an event is triggered.
    */
    bool triggered() override
    {
        if (transition->triggered()) 
        {            
            for (int i = 0; i < midiIndices.size(); i++)
            {           
                if (eventRelease) stateHandler->setEventReleaseMidiValueOn(midiIndices[i], midiStates[i]);
                else stateHandler->setEventMidiValueOn(midiIndices[i], midiStates[i]);
            }
        }
        else if (!(oneWayTransition))
        {
            for (int i = 0; i < midiIndices.size(); i++)
            {
                if (eventRelease) stateHandler->setEventReleaseMidiValueOn(midiIndices[i], !(midiStates[i]));
                else stateHandler->setEventMidiValueOn(midiIndices[i], !(midiStates[i]));
            }
        }
        
        return false;
    }

private:
    TransitionRule* transition;
    bool eventRelease; // are we changing midi data for 'event on' or 'release event'
    std::vector<int> midiIndices;
    std::vector<bool> midiStates;
};


// =======================
// some logical operations

/*
AND operation:
A transition which combines 2 existing TransitionRule objects. The function triggered() returns true if
both TransitionRule objects return true for their triggered() functions.
*/
class AndTransition : public TransitionRule
{
public:

    /*
    This TransitionRule has it's own custom initialize function.
    */
    void initialize(TransitionRule* _transition1, TransitionRule* _transition2, std::vector<int> _statesChanged, std::vector<TransitionRule::Effect> _effects, bool _oneWayTransition)
    {
        transition1 = _transition1;
        transition2 = _transition2;
        statesChanged = _statesChanged;
        effects = _effects;
        oneWayTransition = _oneWayTransition;
    }

    bool triggered() override
    {      
        return ((transition1->triggered()) && (transition2->triggered()));
    }

private:
    TransitionRule* transition1;
    TransitionRule* transition2;
};


/*
OR operation:
A transition which combines 2 existing TransitionRule objects. The function triggered() returns true if
either TransitionRule object return true for their triggered() function.
*/
class OrTransition : public TransitionRule
{
public:
    void initialize(TransitionRule* _transition1, TransitionRule* _transition2, std::vector<int> _statesChanged, std::vector<TransitionRule::Effect> _effects, bool _oneWayTransition)
    {
        transition1 = _transition1;
        transition2 = _transition2;
        statesChanged = _statesChanged;
        effects = _effects;
        oneWayTransition = _oneWayTransition;
    }

    bool triggered() override
    {
        return ((transition1->triggered()) || (transition2->triggered()));
    }

private:
    TransitionRule* transition1;
    TransitionRule* transition2;
};


/*
NOT operation: compose a transition rule from another, applying a NOT operation
to the provided transition rule's triggered() function.
*/
class NotTransition : public TransitionRule
{
public:

    void initialize(TransitionRule* _transition, std::vector<int> _statesChanged, std::vector<TransitionRule::Effect> _effects, bool _oneWayTransition)
    {
        transition = _transition;
        statesChanged = _statesChanged;
        effects = _effects;
        oneWayTransition = _oneWayTransition;
    }

    bool triggered() override
    {
        return !(transition->triggered());
    }

private:
    TransitionRule* transition;
};