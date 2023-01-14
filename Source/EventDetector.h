/*
  ==============================================================================

    EventDetector.h
    Created: 30 Nov 2022 8:44:10pm
    Author:  User

  ==============================================================================
*/

#pragma once

class EventDetector
{
public:

    /// <summary>
    /// The initializer function for an EventDetector object (call this first!).
    /// </summary>
    /// <param name="_sampleRate"> the sample rate the plugin is working with</param>
    /// <param name="_audioBufferSize"> the input buffer size the plugin is working with </param>
    /// <param name="_windowDuration"> period of time to look back when detecting events </param>
    /// <param name="_eventDuration"> cooldown period for when a detected event can follow a previous event</param>
    /// <param name="_numEventSubBeats"> the number of sub-beats for when considering nearest sub-beat in event detection </param>
    /// <param name="_eventOnBeatBias"> (0.0 to 1.0) extent of which to consider distance to nearest sub-beat when detecting events </param>
    void initialize(double _sampleRate, int _audioBufferSize, float _windowDuration, float _eventDuration, int _numEventSubBeats, float _eventOnBeatBias)
    {
        sampleRate = _sampleRate;
        audioBufferSize = _audioBufferSize;
        windowDuration = _windowDuration; 
        eventDuration = _eventDuration;
        numEventSubBeats = _numEventSubBeats;
        eventOnBeatBias = _eventOnBeatBias;

        windowSizeInSamples = (int) (windowDuration * sampleRate);
        if (windowSizeInSamples > bigBufferSize) windowSizeInSamples = bigBufferSize;
        edgeDetectionStartIndex = bigBufferSize - windowSizeInSamples;
        triggerKernelEdgePosition = (int) ((1 - edgePositionRatio) * windowSizeInSamples) + edgeDetectionStartIndex;
        
        numSamplesBetweenEvents = (int)(eventDuration * sampleRate);
        samplesUntilEventFinish = numSamplesBetweenEvents; // <- this will count down
    }


    // ============================================================
    //  load of setters below, functionality obvious from name

    void setDetectionThreshold(float _detectionThreshold)
    {
        // if repeatly called in DSP loop (i.e. getting slider value?), dont want 
        // to clear buffer all the time if not actually changing parameters 
        if (_detectionThreshold != detectionThreshold)
        {
            reset();
            detectionThreshold = _detectionThreshold;
        }
    }

    void setReleaseDetectionThreshold(float _releaseDetectionThreshold)
    {
        // if repeatly called in DSP loop (i.e. getting slider value?), dont want 
        // to clear buffer all the time if not actually changing parameters 
        if (_releaseDetectionThreshold != releaseDetectionThreshold)
        {
            reset();
            releaseDetectionThreshold = _releaseDetectionThreshold;
        }
    }

    void setEventDuration(float _eventDuration)
    {
        // if repeatly called in DSP loop (i.e. getting slider value?), dont want 
        // to clear buffer all the time if not actually changing parameters 
        if (_eventDuration != eventDuration) {
            reset();
            eventDuration = _eventDuration;
            numSamplesBetweenEvents = (int) (eventDuration * sampleRate);
            samplesUntilEventFinish = numSamplesBetweenEvents;
        }
    }

    void setEdgePositionRatio(float _edgePositionRatio)
    {
        // if repeatly called in DSP loop (i.e. getting slider value?), dont want 
        // to clear buffer all the time if not actually changing parameters 
        if (_edgePositionRatio != edgePositionRatio)
        {
            reset();
            edgePositionRatio = _edgePositionRatio;
            triggerKernelEdgePosition = (int)((1 - edgePositionRatio) * windowSizeInSamples) + edgeDetectionStartIndex;
        }
    }

    void setWindowDuration(float _windowDuration)
    {
        // if repeatly called in DSP loop (i.e. getting slider value?), dont want 
        // to clear buffer all the time if not actually changing parameters 
        if (_windowDuration != windowDuration)
        {
            reset();
            windowDuration = _windowDuration;
            windowSizeInSamples = (int)(windowDuration * sampleRate);
            if (windowSizeInSamples > bigBufferSize) windowSizeInSamples = bigBufferSize;
            edgeDetectionStartIndex = bigBufferSize - windowSizeInSamples;
            triggerKernelEdgePosition = (int)((1 - edgePositionRatio) * windowSizeInSamples) + edgeDetectionStartIndex;
        }
    }


    void setEventOnBeatBias(float _eventOnBeatBias)
    {
        // valid value and actually setting to a different value
        if ((_eventOnBeatBias >= 0.0f) && (_eventOnBeatBias <= 1.0f) && (eventOnBeatBias != _eventOnBeatBias))
        {
            eventOnBeatBias = _eventOnBeatBias;
        }
    }
    
    void setBeatPosition(float _beatPosition)
    {
        beatPosition = _beatPosition;
    }
    
    // ==========================================================



    /// <summary>
    /// Computes an estimate of the density of detected rhythmic events 
    /// (considering the intervals beteen the previous 4 detected events).
    /// </summary>
    /// <returns> density value </returns>
    float getDensity()
    {
        // consider a density from currentTimeBetweenEvents as an extra element of prevEventFreqEstimates
        // ONLY IF it decreases the final density estimate -> i.e. if playing nothing, density is gradually decreased.
        float sumStoredIntervals = 0.0f;
        for (float interval : prevEventIntervals)
        {
            sumStoredIntervals += interval;
        }
        float sumExcludingLast = sumStoredIntervals - prevEventIntervals[numPrevEventsConsidered-1];
        float meanStoredInterval = sumStoredIntervals / numPrevEventsConsidered;
        
        if (currentTimeBetweenEvents > meanStoredInterval)
        {
            return 1.0f / ((currentTimeBetweenEvents + sumExcludingLast) / numPrevEventsConsidered);
        }
        else
        {
            return 1.0f / meanStoredInterval;
        }
    }

    /// <summary>
    /// Calculate whether a segment of audio if generally decreasing in amplitude over a period of time.
    /// </summary>
    /// <param name="lookBack"> how many seconds of previous audio we consider </param>
    /// <returns> boolean is decreasing or not </returns>
    bool isVolumeDecreasing(float lookBack)
    {
        int index = (int) (lookBack / intervalBetweenAddingVolumes);

        if (index >= averageBigBufferVolumes.size()) index = averageBigBufferVolumes.size() - 1;

        bool isDecreasing = true;
        for (int i = 0; i < index ; i++)
        {
            isDecreasing = isDecreasing && ((averageBigBufferVolumes[i] - averageBigBufferVolumes[i+1]) < -0.001f);
        }
        return isDecreasing;
    }

    


    /// <summary>
    /// Adds audio samples from an input buffer into a large buffer for later processing
    /// </summary>
    /// <param name="audioBufferPointer"> pointer to the input audio buffer</param>
    /// <param name="numSamples"> number of samples being added </param>
    void pushAudioBufferIntoBigBuffer(float *audioBufferPointer, int numSamples)
    {
        for (int i = 0; i < bigBufferSize - numSamples; i++)
        {
            bigBuffer[i] = bigBuffer[i + numSamples];
            
        }

        for (int i = 0; i < numSamples; i++)
        {
            bigBuffer[i + bigBufferSize - numSamples] = audioBufferPointer[i];
        }

        // update some variables used to keep track of things ----------------------------------------
        currentTimeBetweenEvents += numSamples / sampleRate; // for estimating density of events
        timeUntilAddVolume -= numSamples / sampleRate; // for occassionally adding a volume to a vector (keep track of average volume and 'is decreasing') 
        if (samplesUntilEventFinish > 0) samplesUntilEventFinish -= numSamples; // cooldown period until another event can be generated

        // occassionally add a calculated 'average volume' to a vector (essentially a fixed length FIFO queue)
        if (timeUntilAddVolume <= 0.0f)
        {
            float absSum = 0.0f;
            for (int i = 0; i < bigBufferSize; i++)
            {
                absSum += fabs(bigBuffer[i]);
            }
            
            float endVolume = averageBigBufferVolumes[averageBigBufferVolumes.size() - 1];

            averageBigBufferVolumes.pop_back();
            averageBigBufferVolumes.insert(averageBigBufferVolumes.begin(), absSum / bigBufferSize);

            // update the mean of averageBigBufferVolumes vector (without looping over vector to recalculate)
            meanVolumeEstimate = meanVolumeEstimate + ((averageBigBufferVolumes[0] - endVolume) / averageBigBufferVolumes.size());
            timeUntilAddVolume += intervalBetweenAddingVolumes;
        }    
    }

    /*
    Clears the buffer of stored samples. Resets cool-down wait for another event to occur.
    Seems like a wise idea to call this when setting parameters which affect the audio processing.
    */
    void reset()
    {
        // also reset cooldown period between events
        samplesUntilEventFinish = numSamplesBetweenEvents;

        // now clear buffer
        for (int i = 0; i < bigBufferSize; i++)
        {
            bigBuffer[i] = 0.0f;
        }
    }


    bool detectHit()
    {
        if (samplesUntilEventFinish <= 0)
        {
            // if cooldown period completed, now try detect events: ------------------------------------

            float value = 0.0f;
            float beforeValue = 0.0f;
            for (int i = edgeDetectionStartIndex; i < triggerKernelEdgePosition; i++)
            {
                beforeValue += fabs(bigBuffer[i]);
            }

            float afterValue = 0.0f;
            for (int i = triggerKernelEdgePosition; i < bigBufferSize; i++)
            {
                afterValue += fabs(bigBuffer[i]);
            }

            float prior = priorEventLikelihood();

            // dont want to get dived by zero error
            if (beforeValue <= 0.000001f) beforeValue = 0.000001f;

            // check ratio of after volume to before volume, multiplied by any prior knowledge of if we're on a sub-beat
            // and hence expect rhythmic events more. Also if both values are just really small, disregard this
            if ((prior*(afterValue / beforeValue) > detectionThreshold) && ((afterValue + beforeValue) > 2.0)) 
            {
                // EVENT DETECTED:
                // ====================================================

                // reset cooldown until next event can  occur ---------
                samplesUntilEventFinish = numSamplesBetweenEvents;
               
                prevEventIntervals.pop_back();
                prevEventIntervals.insert(prevEventIntervals.begin(), currentTimeBetweenEvents);
                currentTimeBetweenEvents = 0.001f;
                
                eventOccurring = true;
                eventReleaseOccurring = false;
                return true;
            }

            // check for event release now

            // dont want to get dived by zero error
            if (afterValue <= 0.000001f) afterValue = 0.000001f;

            // as above, but check ratio of before/after for any release-events
            if (prior*(beforeValue / afterValue) > releaseDetectionThreshold && ((afterValue + beforeValue) > 2.0))
            {
                // reset cooldown until next event can  occur ---------
                samplesUntilEventFinish = numSamplesBetweenEvents;

                eventOccurring = false;
                eventReleaseOccurring = true;
                return false;
                // =========================================
                // dont reset currentTimeBetweenEvents here?
                // will generally just use this for a transition AND-ed with a MeanAmplitude transition
                // so shouldn't get super-frequent release events.
                // also I want event detetions to be able to occur instantly after a release detection
            }
        } 
        eventOccurring = false;
        eventReleaseOccurring = false;
        return false;
        //else 
        //{
            //samplesUntilEventFinish -= audioBufferSize;
        //    return false;
        //}
    }

    /*
    Blend in whether we actually care about the distance to nearest sub-beat in
    the beat detection (i.e. return uniform value of 1 if eventOnBeatBias = 0, or return
    distToNearestSubBeat() if eventOnBeatBias = 1).
    */
    float priorEventLikelihood()
    {
        return ((1.0f - distToNearestSubBeat()) * eventOnBeatBias) + (1.0f - eventOnBeatBias);
    }


    float distToNearestSubBeat()
    {
        float subBeatPosition = fmod(beatPosition, 1.0f) * numEventSubBeats;
        int prevSubBeat = (int)subBeatPosition;
        float distToPrevSubBeat = subBeatPosition -  prevSubBeat;
        float distToNextSubBeat = (prevSubBeat + 1) - subBeatPosition;

        // return the closest distance
        if (distToPrevSubBeat < distToNextSubBeat) return distToPrevSubBeat;
        else return distToNextSubBeat;
    }



    float getAverageVolume()
    {
        return meanVolumeEstimate;
    }

    /*
    Just return rhythmic event detection info from most recent detectHit() call.
    -> used in transition rules so that multiple things aren't repeatetly
    calling detectHit() for one input audio buffer.
    */
    bool getEventOccurring()
    {
        return eventOccurring;
    }

    /*
    Just return release-event info from most recent detectHit() call.
    -> used in transition rules so that multiple things aren't repeatetly
    calling detectHit() for one input audio buffer.
    */
    bool getEventReleaseOccurring()
    {
        return eventReleaseOccurring;
    }

private:
    float sampleRate;
    int audioBufferSize;
    
    static const int bigBufferSize = 8192;
    float bigBuffer[bigBufferSize];

    int edgeDetectionStartIndex;
    int triggerKernelEdgePosition;

    float detectionThreshold = 3.0;
    float eventDuration;
    float windowDuration;
    float edgePositionRatio = 0.5f;
    bool eventOccurring = false;

    float releaseDetectionThreshold = 3.0f;
    bool eventReleaseOccurring = false;

    float eventOnBeatBias;
    float beatPosition = 0; // <- gets updated by a StateHandler
    float numEventSubBeats; // <- the number of sub-beat division where
                            // we expect events to be more likely

    int windowSizeInSamples;
    int samplesUntilEventFinish;
    int numSamplesBetweenEvents;

    // density of events estimation
    float maxTimeIntervalConsidered = 16.0f; // seconds
    std::vector<float> prevEventIntervals = { 2.0f, 2.0f, 2.0f, 2.0f };
    int numPrevEventsConsidered = prevEventIntervals.size();
    float currentTimeBetweenEvents = 0.001f; // want no chance of any divide by zero error;


    std::vector<float> averageBigBufferVolumes = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float meanVolumeEstimate = 0.0f;
    float intervalBetweenAddingVolumes = 0.2f; // seconds
    float timeUntilAddVolume = intervalBetweenAddingVolumes;
    
};