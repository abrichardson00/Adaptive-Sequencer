# Adaptive-Sequencer
An ‘auto-drummer’ which adapts it’s drumming in real-time over varying guitar playing. Multiple sequences are defined and toggled on/off by certain 'TransitionRules' detected from the audio input. For example, if average guitar volume is above a threshold, turn on sequence 5. If density of detected rhythmic events decreases below a threshold, turn off sequence 2. 

The code aims to be a general framework in which logical combinations of multiple 'TransitionRules' can trigger multiple changes to which sequences are active.


Note: see AdaptiveSequencerReport.pdf for a more in-depth description.

Rhythmic event detection can also trigger midi outputs (which are switchable) alongside the guitar playing - i.e. layering kick drum hit with the start of a guitar strum. These rhythmic events can also update the tempo - nudging it faster or slower depending on whether events occur slightly before or after expected beat subdivisions.

# User Interface

![plugin UI](plugin_UI.png?raw=true "plugic UI")

The colours represent whether a Sequence object is on (green), off (red) or either changing state (orange), and are updated in real
time when using the plugin.

# Brief Overview of Classes

![class interactions](class_interactions.png?raw=true "class interactions")

