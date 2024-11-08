// Author: Robert Polk
// Copyright (c) 2024 BLINK. All rights reserved.
// Last Modified: 11/08/2024


#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class SerialLogger {
public:
    // Delete everything as this class does not need an instance
    SerialLogger() = delete;

    SerialLogger(const SerialLogger &) = delete;

    SerialLogger &operator=(const SerialLogger &) = delete;

    ~SerialLogger() = delete;

    /**
     * @brief Log things at the data level
     * @param message - The log message to output
     * @param tag - A tag for who is calling the log event
     */
    #ifdef LOGGING_DATA
        static void data(const char *, const char *);
    #else
        static void data(const char *, const char *) {}
    #endif // LOGGING_DATA

    /**
     * @brief Log things at the debug level
     * @param message - The log message to output
     * @param tag - A tag for who is calling the log event
     */
    #ifdef LOGGING_DEBUG
        static void debug(const char *, const char *);
    #else
        static void debug(const char *, const char *) {}
    #endif // LOGGING_DEBUG

    /**
     * @brief Log things at the info level
     * @param message - The log message to output
     * @param tag - A tag for who is calling the log event
     */
    #ifdef LOGGING_INFO
        static void info(const char *, const char *);
    #else
        static void info(const char *, const char *) {}
    #endif // LOGGING_INFO

    /**
     * @brief Log things at the warning level
     * @param message - The log message to output
     * @param tag - A tag for who is calling the log event
     */
    #ifdef LOGGING_WARNING
        static void warning(const char *, const char *);
    #else
        static void warning(const char *, const char *) {}
    #endif // LOGGING_WARNING

    /**
     * @brief Log things at the error level
     * @param message - The log message to output
     * @param tag - A tag for who is calling the log event
     */
    #ifdef LOGGING_ERROR
        static void error(const char *, const char *);
    #else
        static void error(const char *, const char *) {}
    #endif // LOGGING_ERROR

private:
    /**
     * @brief Log things at the data level
     * @param logLevel - The log level of the log event
     * @param message - The log message to output
     * @param tag - A tag for who is calling the log event
     */
    static void printLog(const char *, const char *, const char *);
};

#endif // LOGGER_H