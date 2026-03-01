#include <M5Cardputer.h>

#include "dialogue_manager.h"

void DialogueManager::begin() {
    display.begin();
    network.begin();
    display.showIdle();
}

void DialogueManager::loop() {
    if (state == SessionState::Idle) {
        if (M5.BtnA.wasPressed()) {
            enterPromptMode();
        }
    } else if (state == SessionState::Prompt) {
        handlePromptInput();
    } else if (state == SessionState::Responding || state == SessionState::Error) {
        if (M5.BtnA.wasPressed()) {
            enterPromptMode();
        }
    }
}

void DialogueManager::enterPromptMode() {
    promptInput.clear();
    state = SessionState::Prompt;
    display.showPromptMode(promptInput.buffer(), promptInput.currentCandidate());
    if (!network.notifyModeStatus(true)) {
        display.showStatusLine("モード通知に失敗しました");
    }
}

void DialogueManager::sendActivePrompt() {
    if (!promptInput.hasContent()) {
        display.showStatusLine("入力文字がありません");
        return;
    }

    state = SessionState::Sending;
    display.showSending();
    if (!network.notifyModeStatus(false)) {
        display.showStatusLine("モード終了通知に失敗しました");
    }
    const PromptResponse response = network.postPrompt(promptInput.buffer());
    if (!response.success) {
        state = SessionState::Error;
        display.showError(response.error);
        return;
    }

    state = SessionState::Responding;
    promptInput.clear();
    display.showResponse(response.text);
}

void DialogueManager::handlePromptInput() {
    bool updated = false;

    if (M5.BtnA.wasPressed()) {
        promptInput.nextCandidate();
        updated = true;
    }

    if (M5.BtnB.wasPressed()) {
        if (M5.BtnA.isPressed()) {
            promptInput.backspace();
        } else {
            promptInput.commitCandidate();
        }
        updated = true;
    }

    if (M5.BtnC.wasPressed()) {
        sendActivePrompt();
        return;
    }

    if (updated) {
        display.showPromptMode(promptInput.buffer(), promptInput.currentCandidate());
    }
}
