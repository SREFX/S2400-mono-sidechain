#include "MonoSC.hpp"

// This macro opens the DISTRHO namespace
START_NAMESPACE_DISTRHO

MonoSC::MonoSC()
    : Plugin(DISTRHO_PLUGIN_NUM_PARAMS, 0, 0), 
      fThreshold(0.5f),
      fRatio(4.0f),
      fAttack(10.0f),
      fRelease(100.0f),
      fPanL(0.0f),
      fPanR(1.0f),
      fDryL(1.0f),
      fEnvelope(0.0f)
{
    activate(); // Force coefficient calculation
}

void MonoSC::initParameter(uint32_t index, Parameter& parameter) {
    parameter.hints = kParameterIsAutomatable;
    
    switch (index) {
        case 0:
            parameter.name = "Threshold";
            parameter.symbol = "threshold";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.5f;
            break;

        case 1:
            parameter.name = "Ratio";
            parameter.symbol = "ratio";
            parameter.ranges.min = 1.0f;
            parameter.ranges.max = 30.0f;
            parameter.ranges.def = 4.0f;
            break;

        case 2:
            parameter.name = "Attack (ms)";
            parameter.symbol = "attack";
            parameter.ranges.min = 1.0f;
            parameter.ranges.max = 100.0f;
            parameter.ranges.def = 10.0f;
            break;

        case 3:
            parameter.name = "Release (ms)";
            parameter.symbol = "release";
            parameter.ranges.min = 1.0f;
            parameter.ranges.max = 400.0f;
            parameter.ranges.def = 100.0f;
            break;

        case 4:
            parameter.name = "Pan L (Src)";
            parameter.symbol = "pan_l";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.5f;
            break;

        case 5:
            parameter.name = "Pan R (Dest)";
            parameter.symbol = "pan_r";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.5f;
            break;

        case 6:
            parameter.name = "Source Vol";
            parameter.symbol = "src_vol";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 1.0f;
            break;
        
        case 7:
            parameter.name = "SC Gain";
            parameter.symbol = "sc_gain";
            parameter.ranges.min = 1.0f;
            parameter.ranges.max = 12.0f;
            parameter.ranges.def = 1.0f;
            break;
    }
}

float MonoSC::getParameterValue(uint32_t index) const {
    switch (index) {
        case 0: return fThreshold;
        case 1: return fRatio;
        case 2: return fAttack;
        case 3: return fRelease;
        case 4: return fPanL;
        case 5: return fPanR;
        case 6: return fDryL;
        case 7: return fScaGain;
        default: return 0.0f;
    }
}

void MonoSC::setParameterValue(uint32_t index, float value) {
    double sampleRate = getSampleRate();
    switch (index) {
        case 0:
            // Square the value to make range more sensistive at the bottom
            fThreshold = value * value; break;
        case 1: fRatio = value; break;
        case 2:
            fAttack = value;
            fAttackCoef = 1.0f - std::exp(-1.0f / (fAttack * 0.001f * sampleRate));
            break;
        case 3:
            fRelease = value;
            fReleaseCoef = 1.0f - std::exp (-1.0f / (fRelease * 0.001f * sampleRate));
            break;
        case 4: fPanL = value; break;
        case 5: fPanR = value; break;
        case 6: fDryL = value; break;
        case 7: fScaGain = value; break;
    }
}

void MonoSC::activate() {
    fEnvelope = 0.0f;
    // Retrigger coefficient calculation
    setParameterValue(2, fAttack);
    setParameterValue(3, fRelease);
}

void MonoSC::run(const float** inputs, float** outputs, uint32_t frames) {
    const float* inL = inputs[0]; // Trigger
    const float* inR = inputs[1]; // Signal to be ducked
    float* outL = outputs[0];
    float* outR = outputs[1];

    for (uint32_t i = 0; i < frames; ++i) {

        // Boost and clip the tirgger
        // Multiply the SC and clip at 1.0
        float trigger = std::abs(inL[i]) * fScaGain;
        if (trigger > 1.0f) trigger = 1.0f;

        // Enve follower on Left channel
        float coef = (trigger > fEnvelope) ? fAttackCoef : fReleaseCoef;
        fEnvelope = fEnvelope + coef * (trigger - fEnvelope);

        // Compression logic
        float gainReduction = 1.0f;

        if (fEnvelope > fThreshold && fThreshold < 1.0f) {
            // scale the amountOver value
            float amountOver = (fEnvelope - fThreshold) / (1.0f - fThreshold);
            if (amountOver > 1.0f) amountOver = 1.0f;

            // Use ratio as ducking intensity
            float duckDepth = (fRatio - 1.0f) / 19.0f;

            // Final gain
            gainReduction = 1.0f - (amountOver * duckDepth);
        }

        // Clamp gain
        gainReduction = std::max(0.0f, std::min(1.0f, gainReduction));

        // Apply compression to right channel
        float compressedR = inR[i] * gainReduction;
        float originalL = inL[i] * fDryL; // to keep the source

        // Panning logic
        // PanL: 0.0 hard left, 1.0 = hard right
        outL[i] = (originalL * (1.0f - fPanL)) + (compressedR * (1.0f - fPanR));
        outR[i] = (originalL * fPanL) + (compressedR * fPanR);
    }
}

/* Plugin Library Entry Point */
Plugin* createPlugin() {
    return new MonoSC();
}

// This macro closes the namespace
END_NAMESPACE_DISTRHO