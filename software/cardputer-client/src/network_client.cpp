#include "network_client.h"

#include <ArduinoJson.h>

NetworkClient::NetworkClient()
    : lastConnectAttempt(0) {}

void NetworkClient::begin() {
    WiFi.mode(WIFI_STA);
    connectWifi();
}

bool NetworkClient::connected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkClient::ensureConnected() {
    if (connected()) {
        return true;
    }
    if ((millis() - lastConnectAttempt) < 1000) {
        return false;
    }
    return connectWifi();
}

bool NetworkClient::connectWifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastConnectAttempt = millis();
    unsigned long start = lastConnectAttempt;
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
        delay(200);
    }
    return connected();
}

PromptResponse NetworkClient::postPrompt(const String& text) {
    PromptResponse response;
    if (!ensureConnected()) {
        response.error = "Wi-Fi が接続されていません";
        return response;
    }

    HTTPClient http;
    http.setConnectTimeout(PROMPT_RESPONSE_TIMEOUT_MS);
    http.setTimeout(PROMPT_RESPONSE_TIMEOUT_MS);
    http.begin(OPENCLAW_PROMPT_URL);
    http.addHeader("Content-Type", "application/json");
    if (strlen(OPENCLAW_AUTH_TOKEN) > 0) {
        http.addHeader("Authorization", OPENCLAW_AUTH_TOKEN);
    }

    StaticJsonDocument<256> payload;
    payload["prompt"] = text;
    payload["language"] = "ja-JP";
    payload["source"] = "cardputer";
    String body;
    serializeJson(payload, body);

    const int code = http.POST(body);
    if (code == HTTP_CODE_OK) {
        String reply = http.getString();
        StaticJsonDocument<512> parsed;
        auto err = deserializeJson(parsed, reply);
        if (!err && parsed.containsKey("response")) {
            response.success = true;
            response.text = parsed["response"].as<String>();
        } else {
            response.success = true;
            response.text = reply;
        }
    } else {
        response.error = http.errorToString(code);
    }
    http.end();
    return response;
}

bool NetworkClient::notifyModeStatus(bool ready) {
    if (!ensureConnected()) {
        return false;
    }

    HTTPClient http;
    http.setConnectTimeout(5000);
    http.begin(OPENCLAW_MODE_URL);
    http.addHeader("Content-Type", "application/json");
    if (strlen(OPENCLAW_AUTH_TOKEN) > 0) {
        http.addHeader("Authorization", OPENCLAW_AUTH_TOKEN);
    }

    StaticJsonDocument<192> payload;
    payload["mode"] = "prompt";
    payload["ready"] = ready;
    payload["source"] = "cardputer";
    String body;
    serializeJson(payload, body);

    const int code = http.POST(body);
    http.end();
    return (code >= 200 && code < 300);
}
