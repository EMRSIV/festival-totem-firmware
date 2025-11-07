#pragma once
#include <Arduino.h>
#include <functional>

class Encoder
{
public:
    using Callback = std::function<void(void)>;

    Encoder(uint8_t pinA, uint8_t pinB, uint8_t pinSW,
            bool useInternalPullups = true,
            uint8_t detentTransitions = 4, // 4 or 2 for most EC11s (quarters per click)
            uint16_t debounceMs = 10,      // switch debounce (ms)
            float rotations = 1.0f)        // rotations needed to go from 0 to 255 (1.0 = one full rotation, -1 = 1 unit per click)
        : A(pinA), B(pinB), SW(pinSW),
          usePullups(useInternalPullups),
          detentDiv(detentTransitions),
          debounceMs(debounceMs),
          rotationsFor255(rotations)
    {
    }

    void begin();

    // Call frequently in loop() — debounces button and fires callbacks
    void update();

    // Counts
    int32_t getQuarterCount();      // +/-1 per quadrature transition
    int32_t getDetentCount();       // quarterCount / detentDiv (normalized clicks)
    int32_t getScaledDetentCount(); // detentCount scaled by rotations parameter
    void reset();                   // reset movement count

    // Config
    void setDetentTransitions(uint8_t div) { detentDiv = div ? div : 4; }
    void setDebounce(uint16_t ms) { debounceMs = ms; }
    void setRotations(float r) { rotationsFor255 = r; }
    float getRotations() const { return rotationsFor255; }

    // Button events
    void onPress(Callback cb) { pressCB = cb; }
    void onRelease(Callback cb) { releaseCB = cb; }

    bool getButtonStateRaw() const
    {
        return digitalRead(SW); // Returns HIGH when released, LOW when pressed
    }

private:
    // Pins
    const uint8_t A, B, SW;
    const bool usePullups;

    // Config
    volatile uint8_t detentDiv;
    volatile uint16_t debounceMs;
    float rotationsFor255;                              // Number of full rotations to change from 0 to 255
    static constexpr uint8_t DETENTS_PER_ROTATION = 20; // EC11 standard

    // Quadrature state
    static void IRAM_ATTR isrAB_trampoline(void *arg);
    void IRAM_ATTR isrAB();
    volatile int32_t quarterCount = 0;
    volatile uint8_t prev = 0;

    // Button state
    static void IRAM_ATTR isrSW_trampoline(void *arg);
    void IRAM_ATTR isrSW();
    volatile bool swRaw = true; // true = released (INPUT_PULLUP)
    volatile uint32_t swLastEdgeMs = 0;

    // Debounced state
    bool swStable = true; // true = released
    uint32_t swStableSince = 0;

    // Callbacks
    Callback pressCB = nullptr;
    Callback releaseCB = nullptr;
};
