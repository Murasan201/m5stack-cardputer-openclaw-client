"""State machine helpers for Cardputer ↔ OpenClaw dialogues."""
from enum import Enum, auto
from typing import Dict, Set, Iterable

class SessionState(Enum):
    IDLE = auto()
    PROMPT = auto()
    SENDING = auto()
    RESPONDING = auto()
    ERROR = auto()


class StateMachineError(Exception):
    pass


class StateMachine:
    transitions: Dict[SessionState, Set[SessionState]] = {
        SessionState.IDLE: {SessionState.PROMPT, SessionState.IDLE},
        SessionState.PROMPT: {SessionState.SENDING, SessionState.IDLE, SessionState.ERROR},
        SessionState.SENDING: {SessionState.RESPONDING, SessionState.ERROR, SessionState.IDLE},
        SessionState.RESPONDING: {SessionState.PROMPT, SessionState.IDLE, SessionState.ERROR},
        SessionState.ERROR: {SessionState.IDLE, SessionState.PROMPT},
    }

    def __init__(self) -> None:
        self.state = SessionState.IDLE

    def can_transition(self, next_state: SessionState) -> bool:
        allowed = self.transitions.get(self.state, set())
        return next_state in allowed

    def transition(self, next_state: SessionState) -> None:
        if not self.can_transition(next_state):
            raise StateMachineError(f"Cannot move {self.state.name} -> {next_state.name}")
        self.state = next_state

    def reset(self) -> None:
        self.state = SessionState.IDLE

    def allow_sequence(self, sequence: Iterable[SessionState]) -> bool:
        current = self.state
        for state in sequence:
            allowed = self.transitions.get(current, set())
            if state not in allowed:
                return False
            current = state
        return True
