/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SequenceUIBlock.h"

//==============================================================================
Assignment3AudioProcessorEditor::Assignment3AudioProcessorEditor (Assignment3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // add the event detector block and tempo block of the UI:

    eventDetectorBlock = std::make_unique<EventDetectorUIBlock>(&audioProcessor);
    addAndMakeVisible(*eventDetectorBlock);
    tempoBlock = std::make_unique<TempoUIBlock>(&audioProcessor);
    addAndMakeVisible(*tempoBlock);

    // add all the sequence blocks:

    sequence1Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 0);
    addAndMakeVisible(*sequence1Block);
    sequence2Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 1);
    addAndMakeVisible(*sequence2Block);   
    sequence3Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 2);
    addAndMakeVisible(*sequence3Block);
    sequence4Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 3);
    addAndMakeVisible(*sequence4Block);
    sequence5Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 4);
    addAndMakeVisible(*sequence5Block);
    sequence6Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 5);
    addAndMakeVisible(*sequence6Block);
    sequence7Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 6);
    addAndMakeVisible(*sequence7Block);
    sequence8Block = std::make_unique<SequenceUIBlock>(&audioProcessor, 7);
    addAndMakeVisible(*sequence8Block);

    // start a timer for updating the colours and tempo slider when adapting tempo.
    Timer::startTimerHz(5);

    setResizable(true, false);
    setResizeLimits(100, 100, 1000, 1000);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (650, 500);
}

Assignment3AudioProcessorEditor::~Assignment3AudioProcessorEditor()
{
}

/*
Every time this timer function callback happens we update the colours and tempo slider.
*/
void Assignment3AudioProcessorEditor::timerCallback()
{
    tempoBlock->updateTempo();
    sequence1Block->updateColour();
    sequence2Block->updateColour();
    sequence3Block->updateColour();
    sequence4Block->updateColour();
    sequence5Block->updateColour();
    sequence6Block->updateColour();
    sequence7Block->updateColour();
    sequence8Block->updateColour();
}

//==============================================================================
void Assignment3AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

/*
We set the layout of the plugin UI here.
*/
void Assignment3AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    int sequenceBlockWidth = (int)(getWidth() / 2) - 10;
    int sequenceBlockHeight = 50;


    eventDetectorBlock->setBounds(0, 0, getWidth(), 120);
    tempoBlock->setBounds(0, 130, getWidth(), 80);

    int startY = 220;

    sequence1Block->setBounds(0, startY,                                  sequenceBlockWidth, sequenceBlockHeight);
    sequence2Block->setBounds(0, startY + (sequenceBlockHeight + 10),     sequenceBlockWidth, sequenceBlockHeight);
    sequence3Block->setBounds(0, startY + 2 * (sequenceBlockHeight + 10), sequenceBlockWidth, sequenceBlockHeight);
    sequence4Block->setBounds(0, startY + 3 * (sequenceBlockHeight + 10), sequenceBlockWidth, sequenceBlockHeight);

    sequence5Block->setBounds(sequenceBlockWidth + 20, startY,                                  sequenceBlockWidth, sequenceBlockHeight);
    sequence6Block->setBounds(sequenceBlockWidth + 20, startY + (sequenceBlockHeight + 10),     sequenceBlockWidth, sequenceBlockHeight);
    sequence7Block->setBounds(sequenceBlockWidth + 20, startY + 2 * (sequenceBlockHeight + 10), sequenceBlockWidth, sequenceBlockHeight);
    sequence8Block->setBounds(sequenceBlockWidth + 20, startY + 3 * (sequenceBlockHeight + 10), sequenceBlockWidth, sequenceBlockHeight);
}
