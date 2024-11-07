// Author: Robert Polk
// Copyright (c) 2024 BLINK. All rights reserved.
// Last Modified: 

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "BLEDevice.h"

class ClientHandler {
public:
    // Delete constructor, copy-constructor, and assignment-op
    ClientHandler() = delete;
    ClientHandler(const ClientHandler &) = delete;
    ClientHandler &operator=(const ClientHandler &) = delete;

    // Destructor
    ~ClientHandler();

    /**
     * @brief Initialize the client by creating a BLE device and setting the scanning parameters
     *
     * @param SERVICE_UUID - The UUID of the service to connect to
    * @param IMU_CHARACTERISTIC_UUID - The UUID of the IMU characteristic
     * @param CLIENT_NAME - The name for the BLE device
     */
    static ClientHandler *initialize(const std::string&, const std::string&, const
    std::string&);

    /**
     * Get the ClientHandler instance
     * @return Ptr to the ClientHandler instance
     */
    static ClientHandler* instance();


    void loop();

private:
    /**
    * @brief Private constructor for the singleton pattern
     *
    * @param SERVICE_UUID - The UUID of the service to connect to
    * @param IMU_CHARACTERISTIC_UUID - The UUID of the IMU characteristic
     * @param CLIENT_NAME - The name for the BLE device
    */
    ClientHandler(const std::string&, const std::string&, const std::string&);


    bool connectToServer();


    static void notifyCallback(BLERemoteCharacteristic *, uint8_t *, size_t, bool);

    // Member Variables
    static ClientHandler *inst; // Singleton ptr
    BLEUUID serviceUUID;           // The service UUID to connect to
    BLEUUID imuCharacteristicUUID; // The IMU's characteristic UUID
    bool attemptConnect; // If the client should attempt to connect to a server
    bool connected;      // If the client is currently connected
    bool initiateScan; // If a new scan should be initiated after a disconnection
    BLEAdvertisedDevice *server;                // Ptr to the remote server
    BLERemoteCharacteristic *IMUCharacteristic; // Ptr to the IMU's characteristic

    // Class Constants
    static constexpr int SCAN_INTERVAL = 40; // The interval at which scans are initiated, in ms
    static constexpr int SCAN_WINDOW = 30; // How long each individual scan is, in ms
    static constexpr int SCAN_DURATION = 15; // How long a scan event will run, in s
};

struct ClientCallback final : public BLEClientCallbacks {
    void onConnect(BLEClient*) override;

    void onDisconnect(BLEClient*) override;
};

struct AdvertisedDeviceCallback final : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice) override;
};

#endif // CLIENTHANDLER_H