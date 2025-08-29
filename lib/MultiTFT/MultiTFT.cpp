#include "MultiTFT.hpp"
#include <SPI.h>

MultiTFT::MultiTFT(uint8_t csPin) 
    : TFT_eSPI(), _csPin(csPin) {}

void MultiTFT::begin(uint8_t rotation) {
    pinMode(_csPin, OUTPUT);
    select();
    init();
    setRotation(rotation);
    deselect();
}

void MultiTFT::select() {
    digitalWrite(_csPin, LOW);
    delayMicroseconds(10);
}

void MultiTFT::deselect() {
    delay(1);
    digitalWrite(_csPin, HIGH);
}

TFT_eSPI* MultiTFT::raw() {
    return static_cast<TFT_eSPI*>(this);
}