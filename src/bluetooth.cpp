#include <Arduino.h>
#include <NimBLEDevice.h>

#include "bluetooth.h"

#ifndef BLUETOOTH_TARGET_ADDRESS
#define BLUETOOTH_TARGET_ADDRESS "AA:BB:CC:DD:EE:FF"
#endif

#ifndef BLUETOOTH_TARGET_ADDRESS_TYPE
#define BLUETOOTH_TARGET_ADDRESS_TYPE BLE_ADDR_PUBLIC
#endif

#ifndef BLUETOOTH_COMMAND_SERVICE_UUID
#define BLUETOOTH_COMMAND_SERVICE_UUID "FF00"
#endif

#ifndef BLUETOOTH_COMMAND_CHARACTERISTIC_UUID
#define BLUETOOTH_COMMAND_CHARACTERISTIC_UUID "FF01"
#endif

#ifndef BLUETOOTH_ACKNOWLEDGEMENT_CHARACTERISTIC_UUID
#define BLUETOOTH_ACKNOWLEDGEMENT_CHARACTERISTIC_UUID \
    BLUETOOTH_COMMAND_CHARACTERISTIC_UUID
#endif

namespace
{
constexpr unsigned long reconnectIntervalMs = 5000;
constexpr unsigned long connectionTimeoutMs = 5000;

NimBLEClient *client = nullptr;
NimBLERemoteCharacteristic *commandCharacteristic = nullptr;
NimBLERemoteCharacteristic *acknowledgementCharacteristic = nullptr;
unsigned long nextConnectionAttemptAt = 0;

bool retryIsDue(unsigned long now)
{
    return static_cast<long>(now - nextConnectionAttemptAt) >= 0;
}

void logAcknowledgement(
    NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool)
{
    Serial.printf(
        "Bluetooth acknowledgement: %.*s\n", static_cast<int>(length), data);
}

bool prepareCommandChannel()
{
    NimBLERemoteService *commandService =
        client->getService(BLUETOOTH_COMMAND_SERVICE_UUID);
    if (commandService == nullptr)
    {
        Serial.printf(
            "Bluetooth command service %s not found\n",
            BLUETOOTH_COMMAND_SERVICE_UUID);
        return false;
    }

    commandCharacteristic = commandService->getCharacteristic(
        BLUETOOTH_COMMAND_CHARACTERISTIC_UUID);
    if (commandCharacteristic == nullptr || !commandCharacteristic->canWrite())
    {
        Serial.printf(
            "Bluetooth command characteristic %s is unavailable\n",
            BLUETOOTH_COMMAND_CHARACTERISTIC_UUID);
        commandCharacteristic = nullptr;
        return false;
    }

    acknowledgementCharacteristic = commandService->getCharacteristic(
        BLUETOOTH_ACKNOWLEDGEMENT_CHARACTERISTIC_UUID);
    if (acknowledgementCharacteristic == nullptr)
    {
        Serial.printf(
            "Bluetooth acknowledgement characteristic %s not found\n",
            BLUETOOTH_ACKNOWLEDGEMENT_CHARACTERISTIC_UUID);
        return false;
    }

    const bool notifications = acknowledgementCharacteristic->canNotify();
    if (!notifications && !acknowledgementCharacteristic->canIndicate())
    {
        Serial.printf(
            "Bluetooth acknowledgement characteristic %s cannot notify\n",
            BLUETOOTH_ACKNOWLEDGEMENT_CHARACTERISTIC_UUID);
        return false;
    }

    if (!acknowledgementCharacteristic->subscribe(
            notifications, logAcknowledgement, true))
    {
        Serial.printf("Bluetooth acknowledgement subscription failed\n");
        return false;
    }

    return true;
}

void connectToTarget()
{
    commandCharacteristic = nullptr;
    acknowledgementCharacteristic = nullptr;

    if (client->connect(false))
    {
        Serial.printf("Bluetooth connected to %s\n", BLUETOOTH_TARGET_ADDRESS);
        if (!prepareCommandChannel())
        {
            client->disconnect();
        }
        return;
    }

    Serial.printf("Bluetooth connection to %s failed\n", BLUETOOTH_TARGET_ADDRESS);
}

bool sendCommand(const char *command)
{
    if (commandCharacteristic == nullptr || !client->isConnected())
    {
        Serial.printf("Bluetooth command %s ignored: target is unavailable\n", command);
        return false;
    }

    if (!commandCharacteristic->writeValue(command, 0, true))
    {
        Serial.printf("Bluetooth command %s was not acknowledged\n", command);
        return false;
    }

    Serial.printf("Bluetooth command sent: %s\n", command);
    return true;
}
} // namespace

void Bluetooth::initialize()
{
    NimBLEDevice::init("");

    const NimBLEAddress targetAddress(
        std::string(BLUETOOTH_TARGET_ADDRESS), BLUETOOTH_TARGET_ADDRESS_TYPE);
    client = NimBLEDevice::createClient(targetAddress);
    client->setConnectTimeout(connectionTimeoutMs);
    nextConnectionAttemptAt = millis();
}

void Bluetooth::update()
{
    if (client == nullptr || client->isConnected())
    {
        return;
    }

    const unsigned long now = millis();
    if (!retryIsDue(now))
    {
        return;
    }

    nextConnectionAttemptAt = now + reconnectIntervalMs;
    connectToTarget();
}

bool Bluetooth::isConnected()
{
    return client != nullptr && client->isConnected();
}

bool Bluetooth::increaseBrightness()
{
    return sendCommand("brightness-increase");
}

bool Bluetooth::decreaseBrightness()
{
    return sendCommand("brightness-decrease");
}

bool Bluetooth::increaseSpeed()
{
    return sendCommand("speed-increase");
}

bool Bluetooth::decreaseSpeed()
{
    return sendCommand("speed-decrease");
}

bool Bluetooth::blinkerLeft()
{
    return sendCommand("blinker-left");
}

bool Bluetooth::blinkerRight()
{
    return sendCommand("blinker-right");
}

bool Bluetooth::nextPattern()
{
    return sendCommand("next-pattern");
}

bool Bluetooth::nextPalette()
{
    return sendCommand("next-palette");
}

bool Bluetooth::nextMode()
{
    return sendCommand("next-mode");
}
