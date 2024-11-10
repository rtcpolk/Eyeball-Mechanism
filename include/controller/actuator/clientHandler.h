// Author: Robert Polk
// Copyright (c) 2024 BLINK. All rights reserved.
// Last Modified: 11/07/24

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <memory>
#include "BLEDevice.h"
#include "ArduinoLog.h"

struct ClientCallbacks final : public BLEClientCallbacks {
    void onConnect(BLEClient*) override;

    void onDisconnect(BLEClient*) override;
};

/**
 * This is a struct that implements the program's AdvertisedDeviceCallback method. It inherits
 * from the BLE Library's abstract BLEAdvertisedDeviceCallbacks class
 */
struct AdvertisedDeviceCallbacks final : public BLEAdvertisedDeviceCallbacks {
    /**
     * This method defines what to do when a new device is found during a scan. After
     * scanner->start is called, the program checks for devices and this method is called
     * for every device that is discovered. It checks if the device has the service and
     * characteristic UUIDs that the client is looking for
     */
    void onResult(BLEAdvertisedDevice advertisedDevice) override;
};

/**
 * @brief A singleton class to handle everything related to the BLEClient
 */
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
     * @param SERVICE_UUID - The UUID of the service to connect to
     * @param IMU_CHARACTERISTIC_UUID - The UUID of the IMU characteristic
     * @param CLIENT_NAME - The name for the BLE device
     */
    static bool initialize(const std::string&, const std::string&, const
    std::string&);

    /**
     * @brief Get the ClientHandler instance
     * @return Ptr to the ClientHandler instance
     */
    static ClientHandler* instance();

    /**
     * @brief Set the value of connected
     * @param newConnected - The new value to set connected to
     */
    void setConnected(const bool&);

    /**
     * @brief Set the value of server
     * @param newServer - The new value to set server to
     */
    void setServer(BLEAdvertisedDevice*);

     /**
      * @brief Set the value of attemptConnect
      * @param newAttemptConnect - The new value to set attemptConnect to
      */
      void setAttemptConnect(const bool&);

      /**
       * @brief Set the value of attemptScan
       * @param newInitiateScan - The new value to set attemptScan to
       */
      void setInitiateScan(const bool&);

      /**
       * @brief Get the serviceUUID object
       * @return The serviceUUID object
       */
      BLEUUID getServiceUUID() const;

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
    BLEAdvertisedDevice* server; // Ptr to the remote server
    BLERemoteCharacteristic *IMUCharacteristic; // Ptr to the IMU's characteristic

    // Class Constants
    static constexpr int SCAN_INTERVAL = 40; // The interval at which scans are initiated, in ms
    static constexpr int SCAN_WINDOW = 30; // How long each individual scan is, in ms
    static constexpr int SCAN_DURATION = 15; // How long a scan event will run, in s
};

#endif // CLIENTHANDLER_H