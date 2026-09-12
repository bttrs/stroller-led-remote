#include <Arduino.h>
#include <NimBLEDevice.h>

#include <cstring>

#include "bluetooth.h"
#include "led.h"

namespace
{
constexpr char strollerName[] = "Led Stroller";
constexpr char commandServiceUuid[] = "8bc01404-b072-413b-881e-6ca27b3f630d";
constexpr char commandCharacteristicUuid[] =
    "c7df1272-99f7-4059-8a35-ca03831f2897";
constexpr char statusCharacteristicUuid[] =
    "d6a27e19-4382-4f8d-a6f8-c2eb57a91476";
constexpr char palettePatternStatus[] = "palette";
constexpr char notPalettePatternStatus[] = "not_palette";
constexpr char nextPatternStatusPrefix[] = "received:next_pattern:";
constexpr unsigned long commandIntervalMs = 15;
constexpr uint8_t commandQueueSize = 16;

NimBLEClient *client = nullptr;
NimBLERemoteCharacteristic *commandCharacteristic = nullptr;
NimBLERemoteCharacteristic *statusCharacteristic = nullptr;
bool connectionAttemptInProgress = false;
bool commandChannelReady = false;
bool palettePatternActive = false;
unsigned long nextCommandTransmissionAt = 0;

struct QueuedCommand
{
    const char *value;
};

QueuedCommand commandQueue[commandQueueSize];
uint8_t commandQueueHead = 0;
uint8_t commandQueueTail = 0;
uint8_t commandQueueCount = 0;

void startScan();

void clearCommandQueue()
{
    commandQueueHead = 0;
    commandQueueTail = 0;
    commandQueueCount = 0;
}

bool hasActiveConnection()
{
    return commandChannelReady && client != nullptr && client->isConnected();
}

bool parsePalettePatternStatus(
    const uint8_t *value,
    size_t length,
    bool &isPalettePattern)
{
    const size_t paletteStatusLength = sizeof(palettePatternStatus) - 1;
    const size_t notPaletteStatusLength = sizeof(notPalettePatternStatus) - 1;

    if (length == paletteStatusLength &&
        std::memcmp(value, palettePatternStatus, length) == 0)
    {
        isPalettePattern = true;
        return true;
    }

    if (length == notPaletteStatusLength &&
        std::memcmp(value, notPalettePatternStatus, length) == 0)
    {
        isPalettePattern = false;
        return true;
    }

    const size_t nextPatternPrefixLength = sizeof(nextPatternStatusPrefix) - 1;
    if (length <= nextPatternPrefixLength ||
        std::memcmp(value, nextPatternStatusPrefix, nextPatternPrefixLength) !=
            0)
    {
        return false;
    }

    return parsePalettePatternStatus(
        value + nextPatternPrefixLength,
        length - nextPatternPrefixLength,
        isPalettePattern);
}

void updatePalettePatternStatus(const uint8_t *value, size_t length)
{
    bool isPalettePattern;
    if (!parsePalettePatternStatus(value, length, isPalettePattern))
    {
        return;
    }

    if (palettePatternActive == isPalettePattern)
    {
        return;
    }

    palettePatternActive = isPalettePattern;
    Serial.printf(
        "Bluetooth pattern is %spalette-driven\n",
        palettePatternActive ? "" : "not ");
}

void statusCharacteristicCallback(
    NimBLERemoteCharacteristic *,
    uint8_t *value,
    size_t length,
    bool)
{
    Led::acknowledge();
    updatePalettePatternStatus(value, length);
}

bool isStrollerAdvertisement(const NimBLEAdvertisedDevice *advertisedDevice)
{
    const bool hasStrollerName =
        advertisedDevice->haveName() &&
        advertisedDevice->getName() == strollerName;
    const bool hasCommandService =
        advertisedDevice->isAdvertisingService(NimBLEUUID(commandServiceUuid));

    // The legacy BLE server may not fit both its name and 128-bit service UUID
    // in one advertising packet. The GATT lookup below validates the service.
    return hasStrollerName || hasCommandService;
}

class ClientCallbacks : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient *) override
    {
        connectionAttemptInProgress = false;
    }

    void onConnectFail(NimBLEClient *, int) override
    {
        connectionAttemptInProgress = false;
        startScan();
    }

    void onDisconnect(NimBLEClient *, int) override
    {
        commandCharacteristic = nullptr;
        statusCharacteristic = nullptr;
        commandChannelReady = false;
        palettePatternActive = false;
        connectionAttemptInProgress = false;
        clearCommandQueue();
        startScan();
    }
} clientCallbacks;

class ScanCallbacks : public NimBLEScanCallbacks
{
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
    {
        if (connectionAttemptInProgress ||
            !isStrollerAdvertisement(advertisedDevice))
        {
            return;
        }

        Serial.printf(
            "Bluetooth connecting to %s\n",
            advertisedDevice->getAddress().toString().c_str());
        NimBLEScan *scan = NimBLEDevice::getScan();
        scan->stop();

        client = NimBLEDevice::getDisconnectedClient();
        if (client == nullptr)
        {
            client = NimBLEDevice::createClient(advertisedDevice->getAddress());
        }

        if (client == nullptr)
        {
            Serial.println("Bluetooth client allocation failed");
            startScan();
            return;
        }

        client->setClientCallbacks(&clientCallbacks, false);
        connectionAttemptInProgress = true;
        if (!client->connect(advertisedDevice, true, true))
        {
            connectionAttemptInProgress = false;
            Serial.println("Bluetooth connection failed");
            startScan();
        }
    }

    void onScanEnd(const NimBLEScanResults &, int) override
    {
        if (!connectionAttemptInProgress &&
            (client == nullptr || !client->isConnected()))
        {
            startScan();
        }
    }
} scanCallbacks;

void startScan()
{
    NimBLEScan *scan = NimBLEDevice::getScan();
    if (!scan->isScanning())
    {
        scan->start(0);
    }
}

bool prepareCommandChannel()
{
    NimBLERemoteService *commandService =
        client->getService(commandServiceUuid);
    if (commandService == nullptr)
    {
        Serial.println("Bluetooth command service not found");
        return false;
    }

    commandCharacteristic =
        commandService->getCharacteristic(commandCharacteristicUuid);
    if (commandCharacteristic == nullptr || !commandCharacteristic->canWrite())
    {
        Serial.println("Bluetooth command characteristic is unavailable");
        commandCharacteristic = nullptr;
        return false;
    }

    statusCharacteristic =
        commandService->getCharacteristic(statusCharacteristicUuid);
    if (statusCharacteristic == nullptr || !statusCharacteristic->canRead() ||
        !statusCharacteristic->canNotify())
    {
        Serial.println("Bluetooth status characteristic is unavailable");
        statusCharacteristic = nullptr;
        return false;
    }

    if (!statusCharacteristic->subscribe(true, statusCharacteristicCallback))
    {
        Serial.println("Bluetooth status notifications could not be enabled");
        statusCharacteristic = nullptr;
        return false;
    }

    const std::string status = statusCharacteristic->readValue();
    updatePalettePatternStatus(
        reinterpret_cast<const uint8_t *>(status.data()),
        status.size());

    commandChannelReady = true;
    Serial.println("Bluetooth connected to Led Stroller");
    return true;
}

bool commandTransmissionIsDue(unsigned long now)
{
    return static_cast<long>(now - nextCommandTransmissionAt) >= 0;
}

bool queueCommand(const char *command)
{
    if (!hasActiveConnection())
    {
        Serial.printf(
            "Bluetooth command %s dropped: no active connection\n", command);
        return false;
    }

    if (commandQueueCount == commandQueueSize)
    {
        Serial.printf("Bluetooth command %s dropped: queue is full\n", command);
        return false;
    }

    commandQueue[commandQueueTail] = {command};
    commandQueueTail = (commandQueueTail + 1) % commandQueueSize;
    ++commandQueueCount;
    return true;
}

void transmitNextCommand(unsigned long now)
{
    if (!hasActiveConnection())
    {
        clearCommandQueue();
        return;
    }

    if (commandQueueCount == 0 || !commandTransmissionIsDue(now))
    {
        return;
    }

    const QueuedCommand &command = commandQueue[commandQueueHead];
    if (!commandCharacteristic->writeValue(command.value, false))
    {
        Serial.printf("Bluetooth command %s could not be sent\n", command.value);
        nextCommandTransmissionAt = now + commandIntervalMs;
        return;
    }

    Serial.printf("Bluetooth command sent: %s\n", command.value);
    commandQueueHead = (commandQueueHead + 1) % commandQueueSize;
    --commandQueueCount;
    nextCommandTransmissionAt = now + commandIntervalMs;
}
} // namespace

void Bluetooth::initialize()
{
    NimBLEDevice::init("");

    NimBLEScan *scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(&scanCallbacks);
    scan->setInterval(45);
    scan->setWindow(45);
    scan->setActiveScan(true);
    startScan();
}

void Bluetooth::update()
{
    if (client != nullptr && client->isConnected() && !commandChannelReady)
    {
        if (!prepareCommandChannel())
        {
            clearCommandQueue();
            client->disconnect();
            return;
        }
    }

    transmitNextCommand(millis());
}

bool Bluetooth::isConnected()
{
    return hasActiveConnection();
}

bool Bluetooth::isPalettePatternActive()
{
    return palettePatternActive;
}

bool Bluetooth::blinkerLeft()
{
    return queueCommand("L");
}

bool Bluetooth::blinkerRight()
{
    return queueCommand("R");
}

bool Bluetooth::hazardLights()
{
    return queueCommand("W");
}

bool Bluetooth::toggleAutoPattern()
{
    return queueCommand("auto_pattern");
}

bool Bluetooth::toggleAutoPalette()
{
    return queueCommand("auto_palette");
}

bool Bluetooth::nextPattern()
{
    return queueCommand("next_pattern");
}

bool Bluetooth::nextPalette()
{
    return queueCommand("next_palette");
}

bool Bluetooth::speedUp()
{
    return queueCommand("speed_up");
}

bool Bluetooth::speedDown()
{
    return queueCommand("speed_down");
}

bool Bluetooth::brightnessUp()
{
    return queueCommand("brightness_up");
}

bool Bluetooth::brightnessDown()
{
    return queueCommand("brightness_down");
}

bool Bluetooth::extra1()
{
    return queueCommand("extra_1");
}

bool Bluetooth::extra2()
{
    return queueCommand("extra_2");
}

bool Bluetooth::turnOff()
{
    return queueCommand("off");
}

bool Bluetooth::toggleCarMode()
{
    return queueCommand("car");
}
