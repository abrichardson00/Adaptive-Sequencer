/*
  ==============================================================================

    Sequence.h
    Created: 2 Dec 2022 11:19:01pm
    Author:  User

  ==============================================================================
*/

#pragma once

#include <vector>

class Sequence
{
public:

    /// <summary>
    /// Initialize this Sequence.
    /// </summary>
    /// <param name="_midiValue"> the midi value this Sequence will output.</param>
    /// <param name="_midiVelocity"> the midi velocity this Sequence will output.</param>
    /// <param name="_numBeats"> the total number of beats in the Sequence.</param>
    /// <param name="_numBeatDivisions"> the total number of sub-divisions each beat has.</param>
    void initialize(int _midiValue, int _midiVelocity, int _numBeats, int _numBeatDivisions)
    {
        midiValue = _midiValue;
        midiVelocity = _midiVelocity;
        numBeats = _numBeats;
        numBeatDivisions = _numBeatDivisions;
        patternSize = numBeats*numBeatDivisions;
        pattern.clear(); // <- incase initialize() called more than once by a prepareToPlay() function
        for (int i = 0; i < patternSize; i++)
        {
            pattern.push_back(false);
        }
        prevIndex = -1;

    }

    /// <summary>
    /// Set the pattern from a String provided in the UI.
    /// </summary>
    /// <param name="inputText"> the String provided in the UI.</param>
    void setPatternFromString(juce::String inputText)
    {
        int newPatternLength = 0;
        if (inputText.length() > patternSize)
        {
            // not ideal ^, so just limit it to patternSize (numBeats * numBeatDivisions)
            newPatternLength = patternSize;
        }
        else
        {
            newPatternLength = inputText.length();
            for (int i = 0; i < newPatternLength; i++)
            {
                if (inputText.substring(i,i+1).equalsIgnoreCase("0"))
                {
                    setPatternValue(i, false);
                }
                else
                {
                    setPatternValue(i, true);
                }
            }

            // append extra zeros / falses if not enough text input provided.
            for (int i = newPatternLength; i < patternSize; i++)
            {
                setPatternValue(i, false);
            }
        } 
    }

    /// <summary>
    /// Set the number of beats from a TextEditor String in the UI.
    /// </summary>
    /// <param name="inputText"> the String from the UI component to use.</param>
    void setNumBeatsFromString(juce::String inputText)
    {
        if (isNumber(inputText))
        {
            numBeats = std::stoi(inputText.toStdString());
            resizePatternVector(numBeats * numBeatDivisions);
        }
    }

    /// <summary>
    /// Set the number of beat divisions from a TextEditor String in the UI.
    /// </summary>
    /// <param name="inputText"> the String from the UI component to use.</param>
    void setNumBeatDivisionsFromString(juce::String inputText)
    {
        if (inputText.length() > 0)
        {
            if (isNumber(inputText))
            {
                numBeatDivisions = std::stoi(inputText.toStdString());
                resizePatternVector(numBeats * numBeatDivisions);
            }
        }
    }

    /// <summary>
    /// Set the midi value for the Sequence from a TextEditor String in the UI.
    /// </summary>
    /// <param name="midiValueString"> the String from the UI component to use.</param>
    void setMidiValueFromString(juce::String midiValueString)
    {
        if (isNumber(midiValueString))
        {
            midiValue = stringToInt(midiValueString);
        }
    }

    /// <summary>
    /// Set the midi velocity for the Sequence from a TextEditor String in the UI.
    /// </summary>
    /// <param name="midiVelString"> the String from the UI component to use.</param>
    void setMidiVelocityFromString(juce::String midiVelString)
    {
        if (isNumber(midiVelString))
        {
            midiVelocity = stringToInt(midiVelString);
        }
    }

    /*
    Returns true or false for if sequence is outputting a midi note at current moment.
    If has returned true once, then makes sure True can't be returned until next() sets currentOutput to True again.
    */
    bool isOutputting()
    {
        int beatNo = ((int)fmod(beatPosition, numBeats));
        int subBeatNo = (int)(fmod(beatPosition, 1.0f) * numBeatDivisions);
        int currentIndex = (numBeatDivisions * beatNo) + subBeatNo;

        if (prevIndex == currentIndex) return false;
        else
        {
            setPrevIndex(currentIndex);
            return pattern[currentIndex];
        }
    }

    // =========================
    // some getters and setters:

    std::vector<bool>* getPatternPtr()
    {
        return &pattern;
    }

    void setPatternValue(int index, bool value)
    {
        pattern[index] = value;
    }
    bool getPatternValue(int index)
    {
        return pattern[index];
    }

    void setBeatPosition(float _beatPosition)
    {
        beatPosition = _beatPosition;
    }

    void setPrevIndex(int _prevIndex)
    {
        prevIndex = _prevIndex;
    }

    int getMidiValue()
    {
        return midiValue;
    }

    void setMidiValue(int _midiValue)
    {
        midiValue = _midiValue;
    }

    int getMidiVelocity()
    {
        return midiVelocity;
    }

    void setMidiVelocity(int _midiVelocity)
    {
        midiVelocity = _midiVelocity;
    }

    int getNumBeats()
    {
        return numBeats;
    }

    int getNumBeatDivisions()
    {
        return numBeatDivisions;
    }

private:
    int prevIndex;
    int numBeats;
    int numBeatDivisions;
    int patternSize;

    std::vector<bool> pattern;

    float beatPosition;
    
    int midiValue;
    int midiVelocity;

    // =============================
    // some private helper functions

    static bool isNumber(juce::String inputText)
    {
        if (inputText.length() > 0)
        {
            bool isNumber = true;
            for (int i = 0; i < inputText.length(); i++)
            {
                isNumber = isNumber && std::isdigit(inputText[i]);
            }
            return isNumber;
        }
        else return false;
    }

    static int stringToInt(juce::String inputText)
    {
        return std::stoi(inputText.toStdString());
    }

    void resizePatternVector(int newSize)
    {
        if (newSize > patternSize)
        {
            // size increased: add on some extra elements to the pattern vector
            for (int i = patternSize; i < numBeats * numBeatDivisions; i++)
            {
                pattern.push_back(false);
            }
        }
        else if (newSize < patternSize)
        {
            // size decreased: remove some elements of pattern vector
            pattern.resize(newSize);
        }

        patternSize = newSize;
    }
};