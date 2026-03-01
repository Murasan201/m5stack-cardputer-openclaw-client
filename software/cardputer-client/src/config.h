#pragma once

#include <Arduino.h>

#ifndef WIFI_SSID
#define WIFI_SSID "your-wifi-ssid"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your-wifi-password"
#endif

static const char OPENCLAW_PROMPT_URL[] = "http://openclaw.local/api/v1/prompt";
static const char OPENCLAW_MODE_URL[] = "http://openclaw.local/api/v1/mode";
static const char OPENCLAW_AUTH_TOKEN[] = ""; // e.g. "Bearer abc123" or leave empty for unauthenticated

static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 12000;
static const unsigned long PROMPT_RESPONSE_TIMEOUT_MS = 12000;
static const unsigned long DISPLAY_REFRESH_MS = 100;
