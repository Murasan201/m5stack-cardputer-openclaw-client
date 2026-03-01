#include <M5Cardputer.h>

#include "dialogue_manager.h"

DialogueManager dialogue;

void setup() {
    M5Cardputer.begin(true);
    dialogue.begin();
}

void loop() {
    M5Cardputer.update();
    dialogue.loop();
    delay(20);
}
