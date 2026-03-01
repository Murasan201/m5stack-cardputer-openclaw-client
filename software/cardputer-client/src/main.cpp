#include <M5Cardputer.h>

#include "dialogue_manager.h"

DialogueManager dialogue;

void setup() {
    M5.begin();
    dialogue.begin();
}

void loop() {
    M5.update();
    dialogue.loop();
    delay(20);
}
