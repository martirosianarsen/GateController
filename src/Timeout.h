// Timeout.h
#ifndef TIMEOUT_H
#define TIMEOUT_H

class Timeout
{
private:
    unsigned long timeoutDuration; // The timeout duration in milliseconds
    unsigned long startTime;       // When the timeout started
    bool running;                  // Flag to indicate if the timeout is running

public:
    // Constructor to initialize the timeout duration in seconds
    Timeout(unsigned long durationInSeconds);

    // Start the timeout
    void start();

    // Check if the timeout has expired
    bool hasExpired();

    // Reset the timeout
    void reset();
};

#endif // TIMEOUT_H
