// Author: Robert Polk
// Copyright (c) 2024 BLINK. All rights reserved.
// Last Modified: 11/24/2024

//================================================================================================//

/*
 * This program creates a BLE server to transmit data from the IMU to the BLE client on the
 * mechanism's main controller. To configure this program to interface with the internal eyeball
 * hardware, read through the following sections and set the variables accordingly. Each section
 * configures a different portion of the program and outlines configuration variables and program
 * variables. Only edit the configuration variables. THe sections are in the following order:
 *      Logging
 *      BLE Server
 *      IMU
 */

//================================================================================================//

// #include the necessary header files - Do not edit
#include <Arduino.h>
#include <ArduinoLog.h>
#include <NimBLEDevice.h>
#include "C:\Users\rober\blink\lib\I2Cdev\I2Cdev.h"
#include "C:\Users\rober\blink\lib\MPU6050\MPU6050_6Axis_MotionApps20.h"

/*
 * Logging
 *
 * This section determines the level of logging within the program. The messages are written to
 * the Serial monitor. The following log levels are available:
 *      0 - LOG_LEVEL_SILENT     no output
 *      1 - LOG_LEVEL_FATAL      fatal errors
 *      2 - LOG_LEVEL_ERROR      all errors
 *      3 - LOG_LEVEL_WARNING    errors, and warnings
 *      4 - LOG_LEVEL_NOTICE     errors, warnings and notices
 *      5 - LOG_LEVEL_TRACE      errors, warnings, notices & traces
 *      6 - LOG_LEVEL_VERBOSE    all
 * Uncomment one of the lines below to select the desired logging level. To completely remove
 * logging, go into the ArduinoLog.h file and uncomment line 38 to define DISABLE_LOGGING.
 *
 * Set the baud rate for serial communication. This should match the value in the platformio.ini
 * file.
 *
 */

// Configuration Variables
//constexpr uint8_t LOG_LEVEL = LOG_LEVEL_SILENT;   // The level of messages to show. Uncomment
// constexpr uint8_t LOG_LEVEL = LOG_LEVEL_FATAL;   // the desired level
// constexpr uint8_t LOG_LEVEL = LOG_LEVEL_ERROR;
// constexpr uint8_t LOG_LEVEL = LOG_LEVEL_WARNING;
//constexpr uint8_t LOG_LEVEL = LOG_LEVEL_NOTICE;
constexpr uint8_t LOG_LEVEL = LOG_LEVEL_TRACE;
//constexpr uint8_t LOG_LEVEL = LOG_LEVEL_VERBOSE;
constexpr uint32_t BAUD_RATE = 115200;  // The baud rate for serial communication

/*
 * BLE Server
 *
 * This section configures the BLE Server by setting the UUIDs and device name. The UUIDs need to
 * match those set in controller/main.cpp in order for the server to connect properly. New
 * UUIDs can be generated at https://www.uuidgenerator.net/.
 */

// Configuration Variables
const std::string SERVICE_UUID =
        "da2aa210-e2ab-4d96-8d94-8536ec5a2728"; // The UUID for the service the client should
// connect to
const std::string IMU_CHARACTERISTIC_UUID =
        "72b9a4be-85fe-4cd5-ae42-f32414542c5a"; // The UUID for the IMU characteristic
const std::string DEVICE_NAME = ""; // The name of the device that the client is on

// Program Variables
NimBLEServer *server = nullptr; // Ptr to the server
NimBLECharacteristic *IMUCharacteristic = nullptr;  // Ptr to teh IMU characteristic
bool connected = false; // If the server is currently connected to a client
bool prevConnected = false; // Previous state of connected

/*
 * IMU
 *
 * This section configures the IMU by setting the interrupt pin and variable offsets. Offset
 * values can be obtained from the IMU_Zero program found in the examples folder of the library
 */

// Configuration Variables
const uint8_t INTERRUPT_PIN = 18;   // GPIO pin connected to the INT pin on the IMU
int16_t xAccelOffset = -5537;   // Offset values
int16_t yAccelOffset = 917;
int16_t zAccelOffset = 595;
int16_t xGyroOffset = -103;
int16_t yGyroOffset = 9;
int16_t zGyroOffset = 34;

// Program Variables
MPU6050 mpu;    // MPU instance
bool DMPInit = false;   // If the DMP initialization was successful
volatile bool interrupt = false; // If the IMU interrupt pin has gone high
uint8_t interruptStatus;    // Holds the interrupt status byte from the IMU
uint8_t DMPStatus;  // The result of each DMP operation (!0 = error)
uint16_t packetSize;    // Expected DMP packet size (default is 42 bytes)
uint16_t fifoCount; // Sum of all bytes currently in the FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
Quaternion quaternion;  // Quaternion container [w,x,y,z]
uint8_t quaternionData[16]; // Buffer to hold the 4 quaternion floats [wxyz]

//================================================================================================//

/**
 * A struct to define what to do for server callbacks
 */
struct ServerCallbacks final : public NimBLEServerCallbacks {
    /**
     * Called for connection events. Provides logging messages
     *
     * @param connectedServer - The server that had the connection event
     * @param connInfo - The connection info
     */
    void onConnect(NimBLEServer *connectedServer, NimBLEConnInfo &connInfo) {
        Log.trace("Client Address: ");
        Log.traceln(connInfo.getAddress().toString().c_str());
        Log.infoln("Connected to a client");
    }

    /**
     * Called for disconnection events. Provides logging messages and starts advertising
     *
     * @param disconnectedServer - The server that had the disconnection event
     * @param connInfo - The disconnection info
     * @param reason - The reason for disconnect
     */
    void onDisconnect(NimBLEServer *disconnectedServer, NimBLEConnInfo &connInfo, int reason) {
        Log.trace("Client Address: ");
        Log.traceln(connInfo.getAddress().toString().c_str());
        Log.warningln("Client disconnected");
        Log.infoln("Starting advertising");
        NimBLEDevice::startAdvertising();
    }
};

/**
 * A struct to define what to do for characteristic callbacks
 */
struct CharacteristicCallbacks final : public NimBLECharacteristicCallbacks {
    //todo do i want this?
};

/**
 * A struct to define what to do for descriptor callbacks
 */
struct DescriptorCallbacks final : public NimBLEDescriptorCallbacks {
    //todo do i want this?
};

//================================================================================================//

// Callback instances
static ServerCallbacks serverCallback;
static CharacteristicCallbacks characteristicCallback;
static DescriptorCallbacks descriptorCallback;

/**
 * Restart the ESP32
 */
void restart() {
    Log.fatalln("Fatal error occurred. Restarting the ESP32");
    ESP.restart();
}

/**
 * Establish a BLE server and begin advertising. It creates the device, server, service,
 * characteristic, and descriptor. It then starts teh service and begins advertising
 *
 * 2902 descriptors are created automatically within the library for characteristics that have
 * notify or indicate
 */
void setupBLEServer() {
    // Initialize BLE Device
    NimBLEDevice::init(DEVICE_NAME);
    Log.traceln("BLE device created");

    // Create the server
    server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallback);
    Log.traceln("Server created");

    // Create service and
    NimBLEService *eyeballService = server->createService(SERVICE_UUID);
    Log.traceln("Service created");

    // Create characteristics
    IMUCharacteristic = eyeballService->createCharacteristic(IMU_CHARACTERISTIC_UUID,
                                                             NIMBLE_PROPERTY::READ |
                                                             NIMBLE_PROPERTY::NOTIFY);
    IMUCharacteristic->setCallbacks(&characteristicCallback);
    Log.traceln("IMU Characteristic created");

    // Create other characteristics here

    // Start the service
    Log.traceln("Starting the eyeball service");
    eyeballService->start();

    // Set up and start advertising
    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(false);
    Log.traceln("Starting advertising");
    advertising->start();

    Log.infoln("BLE Server setup successful");
}

void setupIMU() {}

void packageQuaternionData() {
    Log.traceln("packageQuaternionData - Begin");

    mpu.dmpGetQuaternion(&quaternion, fifoBuffer);
    memcpy(&quaternionData[0], &quaternion.w, sizeof(float));
    memcpy(&quaternionData[4], &quaternion.x, sizeof(float));
    memcpy(&quaternionData[8], &quaternion.y, sizeof(float));
    memcpy(&quaternionData[12], &quaternion.z, sizeof(float));

    Log.verboseln("\tQuat:\t%d\t%d\t%d\t%d", quaternion.w, quaternion.x, quaternion.y, quaternion
            .z);
}

void setup() {
    // Establish serial and logging
    Serial.begin(BAUD_RATE);
    Log.begin(LOG_LEVEL, &Serial, true);
    Log.infoln("Serial and logging initialized");

    // Set up the server
    try {
        setupBLEServer();
    } catch (const std::exception &ex) {
        Log.errorln("Failed to setup BLEServer - %s", ex.what());
        restart();
    } catch (...) {
        Log.errorln("Failed to setup BLEServer - Unknown Error");
        restart();
    }

    // Set up the IMU
    try {
        setupIMU();
    } catch (const std::exception &ex) {
        Log.errorln("Failed to setup IMU - %s", ex.what());
        restart();
    } catch (...) {
        Log.errorln("Failed to setup IMU - Unknown Error");
        restart();
    }
}

/**
 * Main program loop to manage getting quaternion data from the DMP and transmitting it to the
 * client. If the client is connected, it gets the latest quaternion packet, packages it, and
 * then notifies the client. It handles reestablishing connections and disconnections
 */
void loop() {
    // Notify the client that the IMU data has changed
    if (connected) {
        if (!DMPInit) {
            Log.errorln("DMP not initialized successfully");
            restart();
        }

        // Get the latest packet and transmit it
        if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
            packageQuaternionData();
            IMUCharacteristic->setValue(quaternionData, sizeof(quaternionData));
            IMUCharacteristic->notify();
            delay(10); //todo optimize this
        }
    }

    // For disconnecting
    if (!connected && prevConnected) {
        delay(500); // Allow BLE Stack a chance to get things ready
        server->startAdvertising();
        prevConnected = connected;
    }

    // For connecting
    if (connected && !prevConnected) {
        prevConnected = connected;
    }
}