// Timeout.cpp
#include "Timeout.h"
#include <Arduino.h>

// Constructor implementation
Timeout::Timeout(unsigned long durationInSeconds)
    : timeoutDuration(durationInSeconds * 1000), running(false) {}

// Start the timeout
void Timeout::start()
{
    startTime = millis(); // Record the current time
    running = true;       // Set the timer to running
}

// Check if the timeout has expired
bool Timeout::hasExpired()
{
    if (running && (millis() - startTime >= timeoutDuration))
    {
        running = false; // Stop the timeout once it has expired
        return true;     // Return true when timeout is reached
    }
    return false;
}

// Reset the timeout (if you need to restart it)
void Timeout::reset()
{
    running = false; // Stop the timer
}
