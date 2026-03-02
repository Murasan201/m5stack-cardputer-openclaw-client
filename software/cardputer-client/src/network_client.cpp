#include "network_client.h"

#include <M5Cardputer.h>
#include <ArduinoJson.h>

NetworkClient::NetworkClient()
    : lastConnectAttempt(0) {}

void NetworkClient::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.print("Wi-Fi 接続中...");
    M5.Lcd.setCursor(10, 55);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.print("SSID: ");
    M5.Lcd.println(WIFI_SSID);

    if (connectWifi()) {
        M5.Lcd.setCursor(10, 80);
        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Lcd.print("接続成功: ");
        M5.Lcd.println(WiFi.localIP().toString());
        delay(1500);
    } else {
        M5.Lcd.setCursor(10, 80);
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.println("接続失敗 - 後で再試行します");
        delay(2000);
    }
}

bool NetworkClient::connected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkClient::ensureConnected() {
    if (connected()) {
        return true;
    }
    if ((millis() - lastConnectAttempt) < 3000) {
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
        response.error = "HTTP " + String(code) + ": " + http.errorToString(code);
    }
    http.end();
    return response;
}

bool NetworkClient::notifyModeStatus(bool ready) {
    // Mode notification not supported by HTTP bridge; silently succeed
    (void)ready;
    return true;
}
