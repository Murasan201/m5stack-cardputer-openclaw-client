import pathlib
import sys

TEST_DIR = pathlib.Path(__file__).resolve().parent
if str(TEST_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_DIR))

import pytest

from state_machine import SessionState, StateMachine, StateMachineError


def test_default_state_is_idle() -> None:
    machine = StateMachine()
    assert machine.state == SessionState.IDLE


def test_valid_transitions() -> None:
    machine = StateMachine()
    machine.transition(SessionState.PROMPT)
    assert machine.state == SessionState.PROMPT
    machine.transition(SessionState.SENDING)
    assert machine.state == SessionState.SENDING
    machine.transition(SessionState.RESPONDING)
    assert machine.state == SessionState.RESPONDING
    machine.transition(SessionState.PROMPT)
    assert machine.state == SessionState.PROMPT


def test_invalid_transition_raises_error() -> None:
    machine = StateMachine()
    with pytest.raises(StateMachineError):
        machine.transition(SessionState.RESPONDING)


def test_allow_sequence() -> None:
    machine = StateMachine()
    assert machine.allow_sequence([SessionState.PROMPT, SessionState.SENDING, SessionState.RESPONDING, SessionState.IDLE])
    assert not machine.allow_sequence([SessionState.SENDING, SessionState.RESPONDING])
