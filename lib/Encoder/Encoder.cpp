#include "Encoder.h"

// Quadrature transition table (Gray code).
// index = (prev<<2) | curr ; value = -1, 0, +1 per transition
static const int8_t QTAB[16] = {
    0, -1, +1, 0,
    +1, 0, 0, -1,
    -1, 0, 0, +1,
    0, +1, -1, 0};

void Encoder::begin()
{
    if (usePullups)
    {
        pinMode(A, INPUT_PULLUP);
        pinMode(B, INPUT_PULLUP);
        pinMode(SW, INPUT_PULLUP);
    }
    else
    {
        pinMode(A, INPUT);
        pinMode(B, INPUT);
        pinMode(SW, INPUT);
    }

    // Initialize quadrature previous state
    prev = (digitalRead(A) << 1) | digitalRead(B);

    // Initialize button states
    swRaw = digitalRead(SW); // HIGH = released (pull-up)
    swStable = swRaw;
    swStableSince = millis();
    swLastEdgeMs = 0;

    // Attach interrupts with instance pointer
    attachInterruptArg(digitalPinToInterrupt(A), isrAB_trampoline, this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(B), isrAB_trampoline, this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(SW), isrSW_trampoline, this, CHANGE);
}

void IRAM_ATTR Encoder::isrAB_trampoline(void *arg)
{
    reinterpret_cast<Encoder *>(arg)->isrAB();
}

void IRAM_ATTR Encoder::isrAB()
{
    // Read both quickly
    uint8_t a = digitalRead(A);
    uint8_t b = digitalRead(B);
    uint8_t curr = (a << 1) | b;

    uint8_t idx = (prev << 2) | curr;
    int8_t delta = QTAB[idx];
    if (delta)
        quarterCount += delta;
    prev = curr;
}

void IRAM_ATTR Encoder::isrSW_trampoline(void *arg)
{
    reinterpret_cast<Encoder *>(arg)->isrSW();
}

void IRAM_ATTR Encoder::isrSW()
{
    // Raw (bouncy) level; timestamp the latest edge
    swRaw = digitalRead(SW);
    swLastEdgeMs = millis();
}

void Encoder::update()
{
    // Debounce the switch outside ISR
    bool raw;
    uint32_t edgeTs;

    noInterrupts();
    raw = swRaw;
    edgeTs = swLastEdgeMs;
    interrupts();

    uint32_t now = millis();

    if (raw != swStable)
    {
        if ((uint32_t)(now - edgeTs) >= debounceMs)
        {
            swStable = raw;
            swStableSince = now;

            // INPUT_PULLUP logic: LOW = pressed
            if (swStable == LOW)
            {
                if (pressCB)
                    pressCB();
            }
            else
            {
                if (releaseCB)
                    releaseCB();
            }
        }
    }
}

int32_t Encoder::getQuarterCount()
{
    noInterrupts();
    int32_t q = quarterCount;
    interrupts();
    return q;
}

int32_t Encoder::getDetentCount()
{
    noInterrupts();
    int32_t q = quarterCount;
    interrupts();
    return (detentDiv ? (q / detentDiv) : q);
}

int32_t Encoder::getScaledDetentCount()
{
    // Get normalized detent count (1 per physical click)
    int32_t detents = getDetentCount();

    // Special case: rotations = -1 means each click = 1 unit
    if (rotationsFor255 < 0.0f)
    {
        return detents;
    }

    // Scale based on rotations parameter
    // Formula: sensitivity_per_click = 255 / (rotationsFor255 * DETENTS_PER_ROTATION)
    // For example: rotations=1.0 → 255/(1.0*20) = 12.75 per click
    //              rotations=0.5 → 255/(0.5*20) = 25.5 per click
    float sensitivityPerClick = 255.0f / (rotationsFor255 * DETENTS_PER_ROTATION);

    return (int32_t)(detents * sensitivityPerClick);
}

void Encoder::reset()
{
    noInterrupts();
    quarterCount = 0;
    interrupts();
}
