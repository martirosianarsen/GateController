#ifndef GATE_CONTROL_H
#define GATE_CONTROL_H

#include <Arduino.h>
#include <Preferences.h>

// Enum representing the three states of the gate
enum GateState
{
    GATE_OPEN,
    GATE_PAUSE,
    GATE_CLOSE
};

class GateControl
{
private:
    GateState current_state;                 // The current state of the gate
    Preferences preferences;                 // NVS storage object
    const char *namespaceKey = "gate_prefs"; // NVS namespace for storage

    // GPIO pins for controlling the gate
    const uint8_t openPin = 16;
    const uint8_t pausePin = 17;
    const uint8_t closePin = 18;

    unsigned long lastActionTime; // To store the start time of the action
    bool actionInProgress;        // To track if an action is in progress

    // Helper function to stop all gate actions (set all pins to HIGH)
    void stopAllActions();

public:
    // Initializes the gate control (GPIO setup and state restoration)
    void init();

    // Sets the gate to the desired state (OPEN, PAUSE, CLOSE)
    void setState(GateState state);

    // Update function to check if the action time (200ms) has passed
    void update();

    // Retrieves the current gate state
    GateState getState();

    // Returns the current gate state as a human-readable string
    String getStateText();
};

#endif
