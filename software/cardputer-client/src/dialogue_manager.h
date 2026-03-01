#pragma once

#include <Arduino.h>

#include "display_manager.h"
#include "network_client.h"
#include "prompt_input.h"

enum class SessionState {
    Idle,
    Prompt,
    Sending,
    Responding,
    Error
};

class DialogueManager {
public:
    void begin();
    void loop();

private:
    void enterPromptMode();
    void exitToIdle();
    void handlePromptInput();
    void sendActivePrompt();

    SessionState state = SessionState::Idle;
    DisplayManager display;
    NetworkClient network;
    PromptInput promptInput;
};
