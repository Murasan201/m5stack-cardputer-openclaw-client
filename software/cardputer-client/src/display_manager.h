#pragma once

#include <Arduino.h>

class DisplayManager {
public:
    void begin();

    void showIdle();
    void showPromptMode(const String& buffer, const String& candidate);
    void showSending();
    void showResponse(const String& response);
    void showError(const String& message);
    void showStatusLine(const String& text);

private:
    void clearRegion(uint16_t y, uint16_t height);
};
