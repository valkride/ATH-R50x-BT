/**
 * Advanced Audio Processing Library Implementation
 */

#include "AudioProcessing.h"

// Constructor
AudioProcessor::AudioProcessor() {
    inputBuffer = nullptr;
    outputBuffer = nullptr;
    overlapBuffer = nullptr;
    windowBuffer = nullptr;
    fftReal = nullptr;
    fftImag = nullptr;
    magnitude = nullptr;
    phase = nullptr;
    noiseSpectrum = nullptr;
    signalSpectrum = nullptr;
    wienerFilter = nullptr;
    
    vadState = 0;
    vadCounter = 0;
    hangoverCounter = 0;
    noiseFloor = 0.001f;
    noiseEstimationMode = true;
    noiseFrameCount = 0;
    bufferIndex = 0;
    bufferReady = false;
    
    // Initialize history buffers
    for (int i = 0; i < VAD_HANGOVER_FRAMES; i++) {
        energyHistory[i] = 0.0f;
        zcrHistory[i] = 0.0f;
    }
}

// Destructor
AudioProcessor::~AudioProcessor() {
    end();
}

// Initialize audio processor
bool AudioProcessor::begin() {
    initializeBuffers();
    
    // Create Hann window
    for (int i = 0; i < FFT_SIZE; i++) {
        windowBuffer[i] = 0.5f * (1.0f - cos(2.0f * M_PI * i / (FFT_SIZE - 1)));
    }
    
    // Initialize noise spectrum to small values
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        noiseSpectrum[i] = 0.001f;
        wienerFilter[i] = 1.0f;
    }
    
    Serial.println("Audio processor initialized");
    return true;
}

// Cleanup
void AudioProcessor::end() {
    freeBuffers();
}

// Initialize audio buffers
void AudioProcessor::initializeBuffers() {
    inputBuffer = (float*)ps_malloc(AUDIO_FRAME_SIZE * sizeof(float));
    outputBuffer = (float*)ps_malloc(AUDIO_FRAME_SIZE * sizeof(float));
    overlapBuffer = (float*)ps_malloc(OVERLAP_SIZE * sizeof(float));
    windowBuffer = (float*)ps_malloc(FFT_SIZE * sizeof(float));
    
    fftReal = (float*)ps_malloc(FFT_SIZE * sizeof(float));
    fftImag = (float*)ps_malloc(FFT_SIZE * sizeof(float));
    magnitude = (float*)ps_malloc(FFT_SIZE / 2 * sizeof(float));
    phase = (float*)ps_malloc(FFT_SIZE / 2 * sizeof(float));
    
    noiseSpectrum = (float*)ps_malloc(FFT_SIZE / 2 * sizeof(float));
    signalSpectrum = (float*)ps_malloc(FFT_SIZE / 2 * sizeof(float));
    wienerFilter = (float*)ps_malloc(FFT_SIZE / 2 * sizeof(float));
    
    // Initialize overlap buffer
    if (overlapBuffer) {
        memset(overlapBuffer, 0, OVERLAP_SIZE * sizeof(float));
    }
}

// Free audio buffers
void AudioProcessor::freeBuffers() {
    if (inputBuffer) { free(inputBuffer); inputBuffer = nullptr; }
    if (outputBuffer) { free(outputBuffer); outputBuffer = nullptr; }
    if (overlapBuffer) { free(overlapBuffer); overlapBuffer = nullptr; }
    if (windowBuffer) { free(windowBuffer); windowBuffer = nullptr; }
    if (fftReal) { free(fftReal); fftReal = nullptr; }
    if (fftImag) { free(fftImag); fftImag = nullptr; }
    if (magnitude) { free(magnitude); magnitude = nullptr; }
    if (phase) { free(phase); phase = nullptr; }
    if (noiseSpectrum) { free(noiseSpectrum); noiseSpectrum = nullptr; }
    if (signalSpectrum) { free(signalSpectrum); signalSpectrum = nullptr; }
    if (wienerFilter) { free(wienerFilter); wienerFilter = nullptr; }
}

// Main processing function
bool AudioProcessor::processFrame(int16_t* input, int16_t* output, uint16_t frameSize) {
    if (!inputBuffer || !outputBuffer) {
        return false;
    }
    
    // Convert input to float
    for (int i = 0; i < frameSize; i++) {
        inputBuffer[i] = (float)input[i] / 32768.0f;
    }
    
    // Voice Activity Detection
    float energy = calculateEnergy(inputBuffer, frameSize);
    float zcr = calculateZCR(inputBuffer, frameSize);
    
    // Update VAD history
    for (int i = VAD_HANGOVER_FRAMES - 1; i > 0; i--) {
        energyHistory[i] = energyHistory[i-1];
        zcrHistory[i] = zcrHistory[i-1];
    }
    energyHistory[0] = energy;
    zcrHistory[0] = zcr;
    
    // VAD decision
    bool currentVAD = (energy > VAD_ENERGY_THRESHOLD) && (zcr > VAD_ZCR_THRESHOLD);
    
    if (currentVAD) {
        vadCounter++;
        hangoverCounter = VAD_HANGOVER_FRAMES;
        if (vadCounter >= VAD_TRIGGER_FRAMES) {
            vadState = 1;
            noiseEstimationMode = false;
        }
    } else {
        vadCounter = 0;
        if (hangoverCounter > 0) {
            hangoverCounter--;
        } else {
            vadState = 0;
        }
    }
    
    // Noise suppression processing
    if (frameSize == FFT_SIZE) {
        // Copy input to FFT buffer
        memcpy(fftReal, inputBuffer, frameSize * sizeof(float));
        memset(fftImag, 0, frameSize * sizeof(float));
        
        // Apply window
        applyWindow(fftReal, frameSize);
        
        // Forward FFT
        fft(fftReal, fftImag, frameSize);
        
        // Compute magnitude and phase
        computeMagnitudePhase(fftReal, fftImag, magnitude, phase, frameSize / 2);
        
        // Update noise spectrum during silence
        if (vadState == 0 && noiseEstimationMode) {
            updateNoiseSpectrum(magnitude);
        }
        
        // Compute Wiener filter
        computeWienerFilter(magnitude, noiseSpectrum, wienerFilter);
        
        // Apply filter
        for (int i = 0; i < frameSize / 2; i++) {
            magnitude[i] *= wienerFilter[i];
        }
        
        // Reconstruct complex spectrum
        computeRealImag(magnitude, phase, fftReal, fftImag, frameSize / 2);
        
        // Inverse FFT
        ifft(fftReal, fftImag, frameSize);
        
        // Apply window again
        applyWindow(fftReal, frameSize);
        
        // Overlap-add
        for (int i = 0; i < OVERLAP_SIZE; i++) {
            outputBuffer[i] = fftReal[i] + overlapBuffer[i];
        }
        for (int i = OVERLAP_SIZE; i < frameSize; i++) {
            outputBuffer[i] = fftReal[i];
        }
        
        // Save overlap
        memcpy(overlapBuffer, &fftReal[frameSize - OVERLAP_SIZE], OVERLAP_SIZE * sizeof(float));
    } else {
        // Simple processing for non-FFT frame sizes
        memcpy(outputBuffer, inputBuffer, frameSize * sizeof(float));
    }
    
    // Convert output back to int16
    for (int i = 0; i < frameSize; i++) {
        float sample = outputBuffer[i] * 32768.0f;
        output[i] = (int16_t)constrain(sample, -32768, 32767);
    }
    
    return true;
}

// Apply window function
void AudioProcessor::applyWindow(float* buffer, uint16_t size) {
    for (int i = 0; i < size; i++) {
        buffer[i] *= windowBuffer[i];
    }
}

// Simple FFT implementation (Cooley-Tukey)
void AudioProcessor::fft(float* real, float* imag, uint16_t size) {
    // Bit-reversal permutation
    uint16_t j = 0;
    for (uint16_t i = 1; i < size; i++) {
        uint16_t bit = size >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        
        if (i < j) {
            float temp = real[i];
            real[i] = real[j];
            real[j] = temp;
            
            temp = imag[i];
            imag[i] = imag[j];
            imag[j] = temp;
        }
    }
    
    // Cooley-Tukey FFT
    for (uint16_t len = 2; len <= size; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        float wlen_r = cos(angle);
        float wlen_i = sin(angle);
        
        for (uint16_t i = 0; i < size; i += len) {
            float w_r = 1.0f;
            float w_i = 0.0f;
            
            for (uint16_t j = 0; j < len / 2; j++) {
                uint16_t u = i + j;
                uint16_t v = i + j + len / 2;
                
                float u_r = real[u];
                float u_i = imag[u];
                float v_r = real[v];
                float v_i = imag[v];
                
                real[u] = u_r + v_r;
                imag[u] = u_i + v_i;
                real[v] = (u_r - v_r) * w_r - (u_i - v_i) * w_i;
                imag[v] = (u_r - v_r) * w_i + (u_i - v_i) * w_r;
                
                float w_r_new = w_r * wlen_r - w_i * wlen_i;
                float w_i_new = w_r * wlen_i + w_i * wlen_r;
                w_r = w_r_new;
                w_i = w_i_new;
            }
        }
    }
}

// Inverse FFT
void AudioProcessor::ifft(float* real, float* imag, uint16_t size) {
    // Conjugate
    for (int i = 0; i < size; i++) {
        imag[i] = -imag[i];
    }
    
    // Forward FFT
    fft(real, imag, size);
    
    // Conjugate and scale
    for (int i = 0; i < size; i++) {
        real[i] /= size;
        imag[i] = -imag[i] / size;
    }
}

// Compute magnitude and phase
void AudioProcessor::computeMagnitudePhase(float* real, float* imag, float* mag, float* phase, uint16_t size) {
    for (int i = 0; i < size; i++) {
        mag[i] = sqrt(real[i] * real[i] + imag[i] * imag[i]);
        phase[i] = atan2(imag[i], real[i]);
    }
}

// Compute real and imaginary from magnitude and phase
void AudioProcessor::computeRealImag(float* mag, float* phase, float* real, float* imag, uint16_t size) {
    for (int i = 0; i < size; i++) {
        real[i] = mag[i] * cos(phase[i]);
        imag[i] = mag[i] * sin(phase[i]);
    }
    
    // Mirror for full spectrum
    for (int i = size; i < 2 * size; i++) {
        real[i] = real[2 * size - i];
        imag[i] = -imag[2 * size - i];
    }
}

// Update noise spectrum
void AudioProcessor::updateNoiseSpectrum(float* spectrum) {
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        noiseSpectrum[i] = NOISE_FLOOR_ALPHA * noiseSpectrum[i] + 
                           (1.0f - NOISE_FLOOR_ALPHA) * spectrum[i];
    }
    noiseFrameCount++;
}

// Compute Wiener filter
void AudioProcessor::computeWienerFilter(float* signal, float* noise, float* filter) {
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float snr = signal[i] / (noise[i] + 1e-10f);
        filter[i] = snr / (1.0f + snr);
        
        // Apply spectral floor
        filter[i] = fmax(filter[i], SPECTRAL_FLOOR);
    }
}

// Calculate energy
float AudioProcessor::calculateEnergy(float* buffer, uint16_t size) {
    float energy = 0.0f;
    for (int i = 0; i < size; i++) {
        energy += buffer[i] * buffer[i];
    }
    return sqrt(energy / size);
}

// Calculate Zero Crossing Rate
float AudioProcessor::calculateZCR(float* buffer, uint16_t size) {
    float zcr = 0.0f;
    for (int i = 1; i < size; i++) {
        if ((buffer[i] > 0 && buffer[i-1] < 0) || (buffer[i] < 0 && buffer[i-1] > 0)) {
            zcr += 1.0f;
        }
    }
    return zcr / size;
}

// Public interface functions
bool AudioProcessor::isVoiceActive() {
    return vadState == 1;
}

float AudioProcessor::getVoiceActivityLevel() {
    return energyHistory[0];
}

void AudioProcessor::resetVAD() {
    vadState = 0;
    vadCounter = 0;
    hangoverCounter = 0;
    for (int i = 0; i < VAD_HANGOVER_FRAMES; i++) {
        energyHistory[i] = 0.0f;
        zcrHistory[i] = 0.0f;
    }
}

void AudioProcessor::setVADThreshold(float energyThreshold, float zcrThreshold) {
    // These would need to be stored as member variables
    // For now, they're defined as constants
}

void AudioProcessor::enableNoiseSupression(bool enable) {
    noiseEstimationMode = enable;
}

void AudioProcessor::resetNoiseEstimation() {
    noiseEstimationMode = true;
    noiseFrameCount = 0;
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        noiseSpectrum[i] = 0.001f;
    }
}

float AudioProcessor::getNoiseFloor() {
    return noiseFloor;
}

// Audio Utilities Implementation
float AudioUtils::dbToLinear(float db) {
    return pow(10.0f, db / 20.0f);
}

float AudioUtils::linearToDb(float linear) {
    return 20.0f * log10(linear + 1e-10f);
}

float AudioUtils::rms(float* buffer, uint16_t size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += buffer[i] * buffer[i];
    }
    return sqrt(sum / size);
}

void AudioUtils::normalize(float* buffer, uint16_t size, float targetRMS) {
    float currentRMS = rms(buffer, size);
    if (currentRMS > 0) {
        float gain = targetRMS / currentRMS;
        applyGain(buffer, size, gain);
    }
}

void AudioUtils::applyGain(float* buffer, uint16_t size, float gain) {
    for (int i = 0; i < size; i++) {
        buffer[i] *= gain;
    }
}

// Audio Effects Implementation
AudioEffects::AudioEffects() {
    agcEnabled = false;
    equalizerEnabled = false;
    compressorEnabled = false;
    limiterEnabled = false;
    
    agcTarget = 0.5f;
    agcGain = 1.0f;
    agcAttack = 0.001f;
    agcRelease = 0.01f;
    
    // Initialize EQ gains to unity
    for (int i = 0; i < NUM_FILTERS; i++) {
        eqGains[i] = 1.0f;
    }
}

void AudioEffects::begin() {
    // Initialize EQ coefficients
    for (int i = 0; i < NUM_FILTERS; i++) {
        // Initialize as unity gain filters
        eqCoeffs[i][0] = 1.0f;  // b0
        eqCoeffs[i][1] = 0.0f;  // b1
        eqCoeffs[i][2] = 0.0f;  // b2
        eqCoeffs[i][3] = 0.0f;  // a1
        eqCoeffs[i][4] = 0.0f;  // a2
        
        // Initialize states
        for (int j = 0; j < 4; j++) {
            eqStates[i][j] = 0.0f;
        }
    }
}

void AudioEffects::processFrame(float* input, float* output, uint16_t size) {
    // Copy input to output
    memcpy(output, input, size * sizeof(float));
    
    // Apply effects in order
    if (agcEnabled) {
        processAGC(output, size);
    }
    
    if (equalizerEnabled) {
        processEqualizer(output, size);
    }
    
    if (compressorEnabled) {
        processCompressor(output, size);
    }
    
    if (limiterEnabled) {
        processLimiter(output, size);
    }
}

void AudioEffects::processAGC(float* buffer, uint16_t size) {
    float rms = AudioUtils::rms(buffer, size);
    
    if (rms > 0) {
        float error = agcTarget - rms;
        float alpha = (error > 0) ? agcAttack : agcRelease;
        agcGain += alpha * error;
        agcGain = constrain(agcGain, 0.1f, 10.0f);
        
        AudioUtils::applyGain(buffer, size, agcGain);
    }
}

void AudioEffects::processEqualizer(float* buffer, uint16_t size) {
    // Simple EQ implementation using biquad filters
    for (int i = 0; i < size; i++) {
        float sample = buffer[i];
        
        // Apply each EQ band
        for (int band = 0; band < NUM_FILTERS; band++) {
            sample = biquadFilter(sample, eqCoeffs[band], eqStates[band]);
        }
        
        buffer[i] = sample;
    }
}

float AudioEffects::biquadFilter(float input, float* coeffs, float* state) {
    float output = coeffs[0] * input + coeffs[1] * state[0] + coeffs[2] * state[1] 
                   - coeffs[3] * state[2] - coeffs[4] * state[3];
    
    // Update states
    state[1] = state[0];
    state[0] = input;
    state[3] = state[2];
    state[2] = output;
    
    return output;
}
