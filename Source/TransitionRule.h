/*
  ==============================================================================

    TransitionRule.h
    Created: 7 Dec 2022 3:57:18pm
    Author:  User

  ==============================================================================
*/

#pragma once
#include <vector>
#include "EventDetector.h"

// forward declaration:
// when defining TransitionRule, we need to know of StateHandler's existence, but we can't actually
// define and #include StateHandler.h first because StateHandler needs to know about TransitionRule!
// basically, forward declarations are a way to deal with these 'dual-dependency-loop' situations.
class StateHandler;


class TransitionRule
{
public:

    /*
    Enumeration used here for storing the effects a TransitionRule can have on a Sequence.
    One could obviously just use a boolean value true = turning on, false = turning off... But
    I find this enum makes other parts of the code far more readable.
    */
    enum Effect {turnOff, turnOn};

    // empty destructor function
    virtual ~TransitionRule() { ; }

    /// <summary>
    /// Initialize the TransitionRule. This is used by multiple (but not all) 
    /// of the child classes which inherit from TransitionRule. 
    /// </summary>
    /// <param name="_stateHandler"> pointer to the plugin's StateHandler object.</param>
    /// <param name="_statesChanged"> vector of indices of which Sequence objects to affect.</param>
    /// <param name="_effects"> vector of Effects to apply to the sequences.</param>
    /// <param name="_oneWayTransition"> is this a one way transition - i.e. we won't undo any effects when triggered() returns false?</param>
    void initialize(StateHandler* _stateHandler, std::vector<int> _statesChanged, std::vector<TransitionRule::Effect> _effects, bool _oneWayTransition)
    {
        stateHandler = _stateHandler;
        statesChanged = _statesChanged;
        effects = _effects;
        oneWayTransition = _oneWayTransition;
    }

    /// <summary>
    /// Checks the criteria for whether the transition should occur. This is a virtual function
    /// which needs to be overridden by a child class which actually accesses and checks some 
    /// relevant StateHandler information.
    /// </summary>
    /// <returns> boolean value of whether this transition should now occur.</returns>
    virtual bool triggered() 
    { 
        return false;
    }

    // ================================================================
    // functions to get access any relevant TransitionRule information:

    StateHandler* getStateHandler()
    {
        return stateHandler;
    }

    std::vector<int>* getStatesChanged()
    {
        return &statesChanged;
    }

    TransitionRule::Effect getEffect(int index)
    {
        return effects[index];
    }

    bool isOneWay()
    {
        return oneWayTransition;
    };

protected:
    StateHandler *stateHandler;
    std::vector<int> statesChanged;
    std::vector<TransitionRule::Effect> effects;
    bool oneWayTransition;
};
