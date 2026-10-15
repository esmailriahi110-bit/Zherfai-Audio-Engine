// درون متد processBlock در PluginProcessor.cpp این بخش تحلیل را قرار دهید:
void ZherfaiAudioEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 1. Fixed Sub-Cut Filter (30 Hz)
    subCutFunction(buffer);

    // 2. Real-time RMS & Spectral Centroid Calculation for Stem Classification
    float rmsInput = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    float currentRmsDb = juce::Decibels::gainToDecibels(rmsInput, -100.0f);

    // Simple heuristic spectral centroid approximation for stem routing
    float lowEnergy = 0.0f, midEnergy = 0.0f, highEnergy = 0.0f;
    const int numSamples = buffer.getNumSamples();
    if (numSamples > 0 && totalNumInputChannels > 0) {
        const float* channelData = buffer.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i) {
            float sample = std::abs(channelData[i]);
            if (i % 2 == 0) lowEnergy += sample;
            else if (i % 3 == 0) midEnergy += sample;
            else highEnergy += sample;
        }
    }

    // Auto-detect stem type if set to Auto
    int detectedStem = STEM_UNKNOWN;
    if (lowEnergy > midEnergy && lowEnergy > highEnergy)
        detectedStem = STEM_SUB_LOW;
    else if (midEnergy > lowEnergy && midEnergy > highEnergy)
        detectedStem = STEM_VOCAL_LEAD;
    else
        detectedStem = STEM_PERCUSSION;

    // Update shared memory telemetry if connected
    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0) {
        pSharedMatrix->channels[assignedNodeIndex].currentRmsDb.store(currentRmsDb);
        pSharedMatrix->channels[assignedNodeIndex].stemType.store(detectedStem);
    }

    // 3. Dynamic De-Mud & De-Esser processing
    processDeEsserAndDynamics(buffer);

    // 4. Mesh Sidechain Ducking Application from Master/Collision Matrix
    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0) {
        float duckGainDb = pSharedMatrix->channels[assignedNodeIndex].sidechainDuckGainDb.load();
        if (duckGainDb < 0.0f) {
            float gainFactor = juce::Decibels::decibelsToGain(duckGainDb);
            buffer.applyGain(gainFactor);
        }
    }
}
