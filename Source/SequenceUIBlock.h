/*
  ==============================================================================

    SequenceUIBlock.h
    Created: 5 Dec 2022 7:27:10pm
    Author:  User

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

class SequenceUIBlock : public juce::Component
{
public:

    /// <summary>
    /// Constructor for SequenceUIBlock. Initialize and link individual components
    /// to the provided Assignment3AudioProcessor reference.
    /// </summary>
    /// <param name="_audioProcessor"> pointer to the plugin's Assignment3AudioProcessor.</param>
    /// <param name="_seqIdx"> the index of the Sequence stored in the StateHandler which this UI component should affect.</param>
    SequenceUIBlock(Assignment3AudioProcessor* _audioProcessor, int _seqIdx)
    {
        audioProcessor = _audioProcessor;
        seqIdx = _seqIdx;

        patternInputLabel.setText("Pattern", juce::dontSendNotification);
        patternInputLabel.attachToComponent(&patternInput, true);
        patternInput.onTextChange = [this] {audioProcessor->stateHandler.getSequencePtr(seqIdx)->setPatternFromString(patternInput.getText()); };

        numBeatsLabel.setText("Beats/divs", juce::dontSendNotification);
        numBeatsLabel.attachToComponent(&textNumBeats, true);
        textNumBeats.onTextChange = [this] {audioProcessor->stateHandler.getSequencePtr(seqIdx)->setNumBeatsFromString(textNumBeats.getText()),
            audioProcessor->stateHandler.getSequencePtr(seqIdx)->setPatternFromString(patternInput.getText()); };

        textNumSubBeats.onTextChange = [this] {audioProcessor->stateHandler.getSequencePtr(seqIdx)->setNumBeatDivisionsFromString(textNumSubBeats.getText()),
            audioProcessor->stateHandler.getSequencePtr(seqIdx)->setPatternFromString(patternInput.getText()); };
        
        midiNoteLabel.setText("note", juce::dontSendNotification);
        midiNoteLabel.attachToComponent(&textMidiNote, true);
        textMidiNote.onTextChange = [this] {audioProcessor->stateHandler.getSequencePtr(seqIdx)->setMidiValueFromString(textMidiNote.getText()); };
        textMidiNote.setText((juce::String)audioProcessor->stateHandler.getSequencePtr(seqIdx)->getMidiValue(), juce::dontSendNotification);


        midiVelocityLabel.setText("vel", juce::dontSendNotification);
        midiVelocityLabel.attachToComponent(&textMidiVelocity, true);
        textMidiVelocity.onTextChange = [this] {audioProcessor->stateHandler.getSequencePtr(seqIdx)->setMidiVelocityFromString(textMidiVelocity.getText()); };
        textMidiVelocity.setText((juce::String) audioProcessor->stateHandler.getSequencePtr(seqIdx)->getMidiVelocity(), juce::dontSendNotification);

        
        updateColour();

        // initialize the patterns and other vals to data from 
        // initialized StateHandler / sequence objects.
        setTextFromSequence();     

        addAndMakeVisible(patternInput);
        addAndMakeVisible(patternInputLabel);

        addAndMakeVisible(textNumBeats);
        addAndMakeVisible(numBeatsLabel);

        addAndMakeVisible(textNumSubBeats);

        addAndMakeVisible(textMidiNote);
        addAndMakeVisible(midiNoteLabel);

        addAndMakeVisible(textMidiVelocity);
        addAndMakeVisible(midiVelocityLabel);
    }
    /*
    Update the background colour to whatever the appropriate colour is for the current Sequence state stored in StateHandler.
    I.e. for animating the traffic-light colours.
    */
    void updateColour()
    {
        backgroundColour = audioProcessor->stateHandler.getStateColour(seqIdx);  
        repaint();
    }

    /*
    Used when initializing the UI to what was initialized in the Assignment3AudioProcessor.
    */
    void setTextFromSequence()
    {
        juce::String patternString = "";
        std::vector<bool>* pattern = audioProcessor->stateHandler.getSequencePtr(seqIdx)->getPatternPtr();
        for (bool p : *pattern)
        {
            if (p) patternString += 1;
            else patternString += 0;
        }
        patternInput.setText(patternString);

        juce::String numBeatsString = (juce::String) audioProcessor->stateHandler.getSequencePtr(seqIdx)->getNumBeats();
        textNumBeats.setText(numBeatsString);

        juce::String numBeatDivisionsString = (juce::String)audioProcessor->stateHandler.getSequencePtr(seqIdx)->getNumBeatDivisions();
        textNumSubBeats.setText(numBeatDivisionsString);
    }

    ~SequenceUIBlock()
    {
    }

    /*
    Paint this component background with the background colour.
    */
    void paint(juce::Graphics& g) override
    {
        g.fillAll(backgroundColour);
    }

    /*
    Set the relative sizes of the child components in this class.
    */
    void resized() override
    {
        patternInput.setBounds(getLocalBounds().removeFromTop(20));
        textNumBeats.setBounds(getLocalBounds().removeFromBottom(20).removeFromLeft(110).removeFromRight(30));
        textNumSubBeats.setBounds(getLocalBounds().removeFromBottom(20).removeFromLeft(140).removeFromRight(30));

        textMidiNote.setBounds(getLocalBounds().removeFromBottom(20).removeFromRight(130).removeFromLeft(50));
        textMidiVelocity.setBounds(getLocalBounds().removeFromBottom(20).removeFromRight(50).removeFromLeft(50));
    }

private:
    Assignment3AudioProcessor* audioProcessor;
    int seqIdx;


    juce::Colour backgroundColour;

    juce::Label patternInputLabel;
    juce::TextEditor patternInput;

    juce::Label numBeatsLabel;
    juce::TextEditor textNumBeats;

    juce::TextEditor textNumSubBeats;

    juce::Label midiNoteLabel;
    juce::TextEditor textMidiNote;

    juce::Label midiVelocityLabel;
    juce::TextEditor textMidiVelocity;

    //==============================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequenceUIBlock)
    
};