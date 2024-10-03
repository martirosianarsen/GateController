#include "gate_control.h"

unsigned long lastActionTime = 0;
bool actionInProgress = false;

void GateControl::init()
{
    // Initialize the GPIO pins for the gate
    pinMode(openPin, OUTPUT);
    pinMode(pausePin, OUTPUT);
    pinMode(closePin, OUTPUT);

    // Set all pins to HIGH to stop all actions initially
    stopAllActions();

    // Begin the NVS preferences
    preferences.begin(namespaceKey, false);

    // Load the last saved state from NVS, defaulting to PAUSE if none is found
    current_state = (GateState)preferences.getUInt("gate_state", GATE_PAUSE);

    // Apply the loaded state to the gate
    setState(current_state);
}

void GateControl::setState(GateState state)
{
    current_state = state;

    // Save the current state to NVS
    preferences.putUInt("gate_state", (uint8_t)current_state);

    // Stop all gate actions before performing the new action
    stopAllActions();

    // Set the appropriate pin LOW to start the action
    switch (current_state)
    {
    case GATE_OPEN:
        digitalWrite(openPin, LOW); // Trigger opening
        break;
    case GATE_PAUSE:
        digitalWrite(pausePin, LOW); // Trigger pausing
        break;
    case GATE_CLOSE:
        digitalWrite(closePin, LOW); // Trigger closing
        break;
    }

    // Record the time when the action started
    lastActionTime = millis();
    actionInProgress = true; // Mark that an action is in progress
}

void GateControl::update()
{
    // Check if 200ms have passed since the action started
    if (actionInProgress && millis() - lastActionTime >= 400)
    {
        // Set all pins back to HIGH to stop the action
        stopAllActions();
        actionInProgress = false; // Action is complete
    }
}

GateState GateControl::getState()
{
    return current_state;
}

String GateControl::getStateText()
{
    // Convert the enum state to a readable string
    switch (current_state)
    {
    case GATE_OPEN:
        return "Open";
    case GATE_PAUSE:
        return "Paused";
    case GATE_CLOSE:
        return "Closed";
    default:
        return "Unknown";
    }
}

void GateControl::stopAllActions()
{
    // Set all pins to HIGH to stop all actions
    digitalWrite(openPin, HIGH);
    digitalWrite(pausePin, HIGH);
    digitalWrite(closePin, HIGH);
}
