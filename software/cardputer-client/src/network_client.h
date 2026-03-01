#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "config.h"

struct PromptResponse {
    bool success = false;
    String text;
    String error;
};

class NetworkClient {
public:
    NetworkClient();

    void begin();
    bool ensureConnected();
    bool connected() const;

    PromptResponse postPrompt(const String& text);
    bool notifyModeStatus(bool ready);

private:
    bool connectWifi();
    unsigned long lastConnectAttempt;
};
