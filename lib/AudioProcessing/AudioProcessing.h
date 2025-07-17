/**
 * Advanced Audio Processing Library
 * 
 * This library provides advanced audio processing functions including:
 * - Voice Activity Detection (VAD)
 * - Noise Suppression
 * - Spectral Subtraction
 * - Adaptive Filtering
 * - Real-time Audio Effects
 * 
 * Optimized for ESP32-C3 with real-time performance
 * 
 * Author: ESP32-C3 Audio Team
 * Date: July 2025
 */

#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Audio processing constants
#define AUDIO_FRAME_SIZE        512
#define AUDIO_SAMPLE_RATE       16000
#define FFT_SIZE                512
#define OVERLAP_SIZE            256
#define NUM_FILTERS             32
#define VAD_FRAME_SIZE          160  // 10ms at 16kHz
#define NOISE_REDUCTION_FRAMES  10

// VAD parameters
#define VAD_ENERGY_THRESHOLD    0.02f
#define VAD_ZCR_THRESHOLD       0.1f
#define VAD_HANGOVER_FRAMES     5
#define VAD_TRIGGER_FRAMES      3

// Noise suppression parameters
#define NOISE_FLOOR_ALPHA       0.95f
#define WIENER_FILTER_ALPHA     0.98f
#define SPECTRAL_FLOOR          0.1f
#define OVER_SUBTRACTION_FACTOR 2.0f

class AudioProcessor {
private:
    // Audio buffers
    float* inputBuffer;
    float* outputBuffer;
    float* overlapBuffer;
    float* windowBuffer;
    
    // FFT buffers
    float* fftReal;
    float* fftImag;
    float* magnitude;
    float* phase;
    
    // Noise estimation
    float* noiseSpectrum;
    float* signalSpectrum;
    float* wienerFilter;
    
    // VAD state
    float energyHistory[VAD_HANGOVER_FRAMES];
    float zcrHistory[VAD_HANGOVER_FRAMES];
    uint8_t vadState;
    uint8_t vadCounter;
    uint8_t hangoverCounter;
    
    // Noise suppression state
    float noiseFloor;
    bool noiseEstimationMode;
    uint16_t noiseFrameCount;
    
    // Buffer management
    uint16_t bufferIndex;
    bool bufferReady;
    
    // Private methods
    void initializeBuffers();
    void freeBuffers();
    void applyWindow(float* buffer, uint16_t size);
    void fft(float* real, float* imag, uint16_t size);
    void ifft(float* real, float* imag, uint16_t size);
    void computeMagnitudePhase(float* real, float* imag, float* mag, float* phase, uint16_t size);
    void computeRealImag(float* mag, float* phase, float* real, float* imag, uint16_t size);
    void updateNoiseSpectrum(float* spectrum);
    void computeWienerFilter(float* signal, float* noise, float* filter);
    float calculateEnergy(float* buffer, uint16_t size);
    float calculateZCR(float* buffer, uint16_t size);
    
public:
    AudioProcessor();
    ~AudioProcessor();
    
    // Initialization
    bool begin();
    void end();
    
    // Main processing function
    bool processFrame(int16_t* input, int16_t* output, uint16_t frameSize);
    
    // VAD functions
    bool isVoiceActive();
    float getVoiceActivityLevel();
    void resetVAD();
    void setVADThreshold(float energyThreshold, float zcrThreshold);
    
    // Noise suppression
    void enableNoiseSupression(bool enable);
    void setNoiseSuppressionLevel(float level);
    void adaptNoiseFloor(bool enable);
    void resetNoiseEstimation();
    
    // Audio effects
    void enableAGC(bool enable);
    void setAGCTarget(float targetLevel);
    void enableEqualizer(bool enable);
    void setEqualizerGain(uint8_t band, float gain);
    
    // Diagnostics
    float getNoiseFloor();
    float getSNR();
    uint16_t getProcessingLoad();
    
    // Configuration
    void setFrameSize(uint16_t size);
    void setSampleRate(uint32_t rate);
    void setOverlapFactor(float factor);
};

// Utility functions
class AudioUtils {
public:
    static float dbToLinear(float db);
    static float linearToDb(float linear);
    static float rms(float* buffer, uint16_t size);
    static void normalize(float* buffer, uint16_t size, float targetRMS);
    static void applyGain(float* buffer, uint16_t size, float gain);
    static void mixBuffers(float* buffer1, float* buffer2, float* output, uint16_t size, float mix);
    static void highPassFilter(float* input, float* output, uint16_t size, float cutoff, float sampleRate);
    static void lowPassFilter(float* input, float* output, uint16_t size, float cutoff, float sampleRate);
    static void bandPassFilter(float* input, float* output, uint16_t size, float lowCutoff, float highCutoff, float sampleRate);
};

// Real-time audio effects
class AudioEffects {
private:
    // Effect states
    bool agcEnabled;
    bool equalizerEnabled;
    bool compressorEnabled;
    bool limiterEnabled;
    
    // AGC parameters
    float agcTarget;
    float agcGain;
    float agcAttack;
    float agcRelease;
    
    // Equalizer parameters
    float eqGains[NUM_FILTERS];
    float eqCoeffs[NUM_FILTERS][5];  // Biquad coefficients
    float eqStates[NUM_FILTERS][4];  // Biquad states
    
    // Compressor parameters
    float compThreshold;
    float compRatio;
    float compAttack;
    float compRelease;
    float compGain;
    
public:
    AudioEffects();
    
    void begin();
    void processFrame(float* input, float* output, uint16_t size);
    
    // AGC control
    void enableAGC(bool enable);
    void setAGCParameters(float target, float attack, float release);
    
    // Equalizer control
    void enableEqualizer(bool enable);
    void setEQGain(uint8_t band, float gain);
    void setEQPreset(uint8_t preset);
    
    // Compressor control
    void enableCompressor(bool enable);
    void setCompressorParameters(float threshold, float ratio, float attack, float release);
    
    // Limiter control
    void enableLimiter(bool enable);
    void setLimiterThreshold(float threshold);
    
private:
    void processAGC(float* buffer, uint16_t size);
    void processEqualizer(float* buffer, uint16_t size);
    void processCompressor(float* buffer, uint16_t size);
    void processLimiter(float* buffer, uint16_t size);
    float biquadFilter(float input, float* coeffs, float* state);
    void calculateEQCoeffs(float frequency, float gain, float q, float* coeffs);
};

#endif // AUDIO_PROCESSING_H
