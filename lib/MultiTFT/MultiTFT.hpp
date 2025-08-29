#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

class MultiTFT : public TFT_eSPI {
public:
    MultiTFT(uint8_t csPin);

    void begin(uint8_t rotation = 1);

    // Public select/deselect for manual CS control
    void select();
    void deselect();

    // Catch-all passthrough for direct TFT_eSPI access if needed
    TFT_eSPI* raw();

private:
    uint8_t _csPin;
};
