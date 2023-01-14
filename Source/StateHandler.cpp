/*
  ==============================================================================

    StateHandler.cpp
    Created: 9 Dec 2022 12:50:21pm
    Author:  User

    For documentation, see the header file! All the functions are implemented
    here, but it's a big file, so best to find the definition one needs in the
    header file and then ctrl-F to find here.

    Some documentation on how functions work are obviously included in the
    implementations here.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "StateHandler.h"


void StateHandler::initialize(float _sampleRate, float _tempo, EventDetector* _eventDetector)
{
    sampleRate = _sampleRate;
    tempo = _tempo;
    eventDetector = _eventDetector;

    beatsPerSample = tempo / (60.0f * sampleRate);
    beatPosition = 0.0f;
    numSequences = 0;
    numTransitionRules = 0;
}


void StateHandler::setSampleRate(float _sampleRate)
{
    sampleRate = _sampleRate;
    beatsPerSample = tempo / (60.0f * sampleRate);
}


void StateHandler::setState(int index, State state)
{
    states[index] = state;
}


juce::Colour StateHandler::getStateColour(int index)
{
    if (states[index] == State::off) return juce::Colours::red;
    else if (states[index] == State::on) return juce::Colours::green;
    else if (states[index] == State::turningOff) return juce::Colours::orange;
    else if (states[index] == State::turningOn) return juce::Colours::orange;
    else return juce::Colours::black; // <- an 'error' colour... should never happen.
}


// ======================================================================
// a load of getters and setters for handling rhythmic event midi outputs

std::vector<int>* StateHandler::getEventMidiValues()
{
    return &eventMidiValues;
}

void StateHandler::setEventMidiValue(int index, int midiVal)
{
    eventMidiValues[index] = midiVal;
}

std::vector<int>* StateHandler::getEventMidiVelocities()
{
    return &eventMidiVelocities;
}

void StateHandler::setEventMidiVelocity(int index, int _eventMidiVelocity)
{
    eventMidiVelocities[index] = _eventMidiVelocity;
}

bool StateHandler::getEventMidiValuesOn(int index)
{
    return eventMidiValuesOn[index];
}

void StateHandler::setEventMidiValueOn(int index, bool on)
{
    eventMidiValuesOn[index] = on;
}


// handling the event release midi data

std::vector<int>* StateHandler::getEventReleaseMidiValues()
{
    return &eventReleaseMidiValues;
}

void StateHandler::setEventReleaseMidiValue(int index, int midiVal)
{
    eventReleaseMidiValues[index] = midiVal;
}

std::vector<int>* StateHandler::getEventReleaseMidiVelocities()
{
    return &eventReleaseMidiVelocities;
}

void StateHandler::setEventReleaseMidiVelocity(int index, int _eventReleaseMidiVelocity)
{
    eventReleaseMidiVelocities[index] = _eventReleaseMidiVelocity;
}

bool StateHandler::getEventReleaseMidiValuesOn(int index)
{
    return eventReleaseMidiValuesOn[index];
}

void StateHandler::setEventReleaseMidiValueOn(int index, bool on)
{
    eventReleaseMidiValuesOn[index] = on;
}

// =======================================================================


void StateHandler::addSequence(Sequence* sequencePtr)
{
    sequences.push_back(sequencePtr);
    states.push_back(State::off);
    numSequences += 1;
}

void StateHandler::addTransitionRule(TransitionRule* transitionRulePtr)
{
    transitionRules.push_back(transitionRulePtr);
    numTransitionRules += 1;
}

EventDetector* StateHandler::getEventDetectorPtr()
{
    return eventDetector;
}

void StateHandler::applyEffect(int i, TransitionRule::Effect effect)
{
    if ((states[i] == State::off || states[i] == State::turningOff) && effect == TransitionRule::Effect::turnOn)
    {
        states[i] = State::turningOn;
    }
    else if ((states[i] == State::on || states[i] == State::turningOn) && effect == TransitionRule::Effect::turnOff)
    {
        states[i] = State::turningOff;
    }
}

void StateHandler::undoEffect(int i, TransitionRule::Effect effect)
{
    if ((states[i] == State::off || states[i] == State::turningOff) && effect == TransitionRule::Effect::turnOff)
    {
        // state is off or turning off, and effect was turnOff, so turn on to undo the effect:
        states[i] = State::turningOn;
    }
    else if ((states[i] == State::on || states[i] == State::turningOn) && effect == TransitionRule::Effect::turnOn)
    {
        // state is on or turning on, and effect was turnOn so turn off to undo the effect:
        states[i] = State::turningOff;
    }
}

void StateHandler::updateState()
{
    for (TransitionRule* transition : transitionRules)
    {
        if (transition->triggered())
        {
            std::vector<int>* statesChanged = transition->getStatesChanged();
            for (int i = 0; i < statesChanged->size(); i++)
            {
                applyEffect((*statesChanged)[i], transition->getEffect(i));
            }
        }
        else
        {
            if (!(transition->isOneWay()))
            {
                
                std::vector<int>* statesChanged = transition->getStatesChanged();
                for (int i = 0; i < statesChanged->size(); i++)
                {
                    undoEffect((*statesChanged)[i], transition->getEffect(i));
                }
            }
        }
    }
}

void StateHandler::updateSequences(int numSamples)
{
    float beatsInBlock = (numSamples * beatsPerSample);
    beatPosition = fmod(beatPosition + beatsInBlock, maxNumBeats);

    eventDetector->setBeatPosition(beatPosition);
    for (int i = 0; i < numSequences; i++)
    {
        if (fmod(beatPosition, sequences[i]->getNumBeats()) < beatsInBlock)
        {
            // this sequence has just looped back to this start
            // so change any states from turningOff -> off, and turningOn -> on
            if (states[i] == State::turningOff) states[i] = State::off;
            if (states[i] == State::turningOn)  states[i] = State::on;
        }

        // update the sequences which are currently on or still transitioning
        if (states[i] == State::on || states[i] == State::turningOff) sequences[i]->setBeatPosition(beatPosition);
    }
}


// ===================================================================
// some tempo stuff: getters / setters and tempo adaptation...
// ===================================================================

void StateHandler::setTempo(float _tempo)
{
    // only make change if slider is actually clicked and dragged
    if (fabs(_tempo - tempo) > 0.01)
    {
        tempo = _tempo;
        beatsPerSample = tempo / (60.0f * sampleRate);
    }
}

float StateHandler::getTempo()
{
    return tempo;
}

bool StateHandler::isAdaptingTempo()
{
    return adaptingTempo;
}

void StateHandler::setAdaptingTempo(bool _adaptingTempo)
{
    adaptingTempo = _adaptingTempo;
}

float StateHandler::getAdaptationSpeed()
{
    return adaptationSpeed;
}

void StateHandler::setAdaptationSpeed(float _adaptationSpeed)
{
    adaptationSpeed = _adaptationSpeed;
}

float StateHandler::getAdaptationBias()
{
    return adaptationBias;
}

void StateHandler::setAdaptationBias(float _adaptationBias)
{
    adaptationBias = _adaptationBias;
}

// ===============================================================

void StateHandler::updateTempo()
{
    if (isAdaptingTempo())
    {
        if (eventDetector->getEventOccurring())
        {
            float subBeatPosition = (fmod(beatPosition, 1.0f) * subBeatsConsidered);
            int currentSubBeat = (int)subBeatPosition;
            // int prevSubBeat = (currentSubBeat - 1); 
            int nextSubBeat = (currentSubBeat + 1); // dont think we actually want to bother with mod subBeatsConsidered here

            float distToCurrent = subBeatPosition - currentSubBeat;
            float distToNext = nextSubBeat - subBeatPosition;

            // weight the tempo updates by 1 / (1 + event density).
            // so that when events are less frequent they individually have bigger effects on the tempo change
            // -> also, frequent event triggers are generally not less accurately played, so this stops
            // faster playing from just always changing the tempo in any undesired way.
            float invOnePlusDensity = 1.0f / (1.0f + eventDetector->getDensity());

            // get some 0 <-> 1 values for slow down preference and speed up preference
            float rightBias = 0.5f * (adaptationBias + 1);
            float leftBias = 1 - rightBias;

            if (distToCurrent < distToNext)
            {
                // event detected is 'late' by some amount
                // so decrease tempo variable slightly to match a potentially
                // slower played tempo.
                setTempo(tempo - (leftBias * invOnePlusDensity * adaptationSpeed * distToCurrent));
            }
            else
            {
                // event detected is 'early' by some amount
                // so increase tempo variable a little bit
                setTempo(tempo + (rightBias * invOnePlusDensity * adaptationSpeed * distToNext));

            }
        }
    }
}


float StateHandler::distToBeat(int beat, int subBeat, int numBeats, int numSubBeats)
{
    float pos = fmod(beatPosition, numBeats);
    return fabs(pos - (beat + (((float)subBeat) / numSubBeats)));
}

int StateHandler::getNumSequences()
{
    return numSequences;
}

Sequence* StateHandler::getSequencePtr(int seqIndex)
{
    return sequences[seqIndex];
}

float StateHandler::getBeatPosition()
{
    return beatPosition;
}