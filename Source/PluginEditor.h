/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SequenceUIBlock.h"
#include "TempoUIBlock.h"
#include "EventDetectorUIBlock.h"


//==============================================================================
/**
*/
class Assignment3AudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    Assignment3AudioProcessorEditor(Assignment3AudioProcessor&);
    ~Assignment3AudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Assignment3AudioProcessor& audioProcessor;

    // custom UI component declaration:

    std::unique_ptr<EventDetectorUIBlock> eventDetectorBlock;
    std::unique_ptr<TempoUIBlock> tempoBlock;
    
    std::unique_ptr<SequenceUIBlock> sequence1Block;
    std::unique_ptr<SequenceUIBlock> sequence2Block;
    std::unique_ptr<SequenceUIBlock> sequence3Block;
    std::unique_ptr<SequenceUIBlock> sequence4Block;
    std::unique_ptr<SequenceUIBlock> sequence5Block;
    std::unique_ptr<SequenceUIBlock> sequence6Block;
    std::unique_ptr<SequenceUIBlock> sequence7Block;
    std::unique_ptr<SequenceUIBlock> sequence8Block;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Assignment3AudioProcessorEditor)
};
