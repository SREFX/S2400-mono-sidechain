#ifndef MONO_SC_HPP_INCLUDED
#define MONO_SC_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

// Open the room
START_NAMESPACE_DISTRHO

class MonoSC : public Plugin {
public:
    MonoSC();

protected:
    /* Information */
    const char* getLabel() const override { return "MonoSC"; }
    const char* getMaker() const override { return "Fxxxxx"; }
    const char* getLicense() const override { return "MIT"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return d_cconst('m', 'S', 'C', '1'); }

    /* Init */
    void initParameter(uint32_t index, Parameter& parameter) override;

    /* Internal State */
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    /* Process */
    void activate() override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;
    

private:
    // Parameters
    float fThreshold, fRatio, fAttack, fRelease;
    float fPanL, fPanR, fDryL, fScaGain;

    // Internal state
    float fEnvelope;
    float fAttackCoef;
    float fReleaseCoef;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MonoSC)
};

// Close the room
END_NAMESPACE_DISTRHO

#endif