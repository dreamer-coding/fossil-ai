/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#ifndef FOSSIL_JELLYFISH_ANOM_H
#define FOSSIL_JELLYFISH_ANOM_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Limits
// ======================================================

#define FOSSIL_AI_ANOM_MAX_HISTORY 1024
#define FOSSIL_AI_ANOM_THRESHOLD   3.0f // z-score threshold

// ======================================================
// Anomaly Types
// ======================================================

typedef enum {
    FOSSIL_AI_ANOM_NONE = 0,
    FOSSIL_AI_ANOM_STATISTICAL,
    FOSSIL_AI_ANOM_RECONSTRUCTION,
    FOSSIL_AI_ANOM_DEVIATION
} fossil_ai_anom_type_t;

// ======================================================
// Anomaly History Record
// ======================================================

typedef struct {
    float score;
    fossil_ai_anom_type_t type;
    int64_t timestamp;
} fossil_ai_anom_record_t;

// ======================================================
// Anomaly Detector
// ======================================================

typedef struct {
    fossil_ai_anom_record_t history[FOSSIL_AI_ANOM_MAX_HISTORY];
    size_t history_len;
} fossil_ai_anom_t;

// ======================================================
// Initialization
// ======================================================

/**
 * @brief Creates and initializes a new anomaly detection object for Fossil AI.
 *
 * This function allocates memory for a new instance of `fossil_ai_anom_t`, which
 * represents an anomaly detection context within the Fossil AI system. The returned
 * object is initialized and ready for use in anomaly detection tasks.
 *
 * The caller is responsible for managing the lifetime of the returned object,
 * including freeing its memory when it is no longer needed.
 *
 * @return Pointer to a newly created and initialized `fossil_ai_anom_t` object,
 *         or NULL if memory allocation fails.
 */
fossil_ai_anom_t *fossil_ai_anom_create(void);

/**
 * @brief Frees the memory associated with a Fossil AI anomaly detection object.
 *
 * This function releases the resources allocated for the given `fossil_ai_anom_t`
 * object. It is the caller's responsibility to ensure that the pointer passed to
 * this function is valid and was previously allocated by `fossil_ai_anom_create()`.
 *
 * After calling this function, the pointer should not be used unless it is
 * reinitialized.
 *
 * @param detector Pointer to the `fossil_ai_anom_t` object to be freed.
 */
void fossil_ai_anom_free(fossil_ai_anom_t *detector);

// ======================================================
// Detection Methods
// ======================================================

/**
 * @brief Performs statistical anomaly detection using z-score method.
 *
 * This function analyzes the provided data array to detect anomalies based on
 * statistical properties. It calculates the mean and standard deviation of the
 * data, computes the z-score for the last element, and determines if it exceeds
 * the specified threshold.
 *
 * @param detector Pointer to the `fossil_ai_anom_t` object used for detection.
 * @param data     Pointer to an array of float data points to analyze.
 * @param len      The number of elements in the data array.
 * @param threshold The z-score threshold above which an anomaly is detected.
 * @param score    Pointer to a float variable where the computed z-score will be stored.
 *
 * @return true if an anomaly is detected (z-score > threshold), false otherwise.
 */
bool fossil_ai_anom_detect_statistical(fossil_ai_anom_t *detector,
                                       const float *data,
                                       size_t len,
                                       float threshold,
                                       float *score);

/**
 * @brief Performs deviation-based anomaly detection.
 *
 * This function detects anomalies by measuring the deviation of the current data
 * point from the mean of previously recorded scores. If the deviation exceeds a
 * predefined threshold, an anomaly is flagged.
 * @param detector Pointer to the `fossil_ai_anom_t` object used for detection.
 * @param data     Pointer to an array of float data points to analyze.
 * @param len      The number of elements in the data array.
 * @param score    Pointer to a float variable where the computed deviation score will be stored.
 * @return true if an anomaly is detected (deviation > threshold), false otherwise.
 */
bool fossil_ai_anom_detect_deviation(fossil_ai_anom_t *detector,
                                     const float *data,
                                     size_t len,
                                     float *score);

/**
 * @brief Performs reconstruction-based anomaly detection using an autoencoder model.
 * This function uses the provided autoencoder model to reconstruct the input data
 * and calculates the mean squared error (MSE) between the original and reconstructed data.
 * If the MSE exceeds a predefined threshold, an anomaly is flagged.
 * @param detector    Pointer to the `fossil_ai_anom_t` object used for detection.
 * @param data        Pointer to an array of float data points to analyze.
 * @param len         The number of elements in the data array.
 * @param autoencoder Pointer to a `fossil_ai_jellyfish_model_t` representing the autoencoder model.
 * @param score       Pointer to a float variable where the computed reconstruction error score will be stored.
 * @return true if an anomaly is detected (reconstruction error > threshold), false otherwise.
 */
bool fossil_ai_anom_detect_reconstruction(fossil_ai_anom_t *detector,
                                          const float *data,
                                          size_t len,
                                          fossil_ai_jellyfish_model_t *autoencoder,
                                          float *score);

// ======================================================
// Persistence
// ======================================================

/**
 * @brief Saves the state of the anomaly detector to a file.
 *
 * This function serializes the internal state of the provided `fossil_ai_anom_t`
 * object and writes it to the specified file path. This allows for persistence
 * of the anomaly detection state across sessions.
 *
 * @param detector Pointer to the `fossil_ai_anom_t` object to be saved.
 * @param filepath Path to the file where the detector's state will be saved.
 *
 * @return true if the detector was successfully saved; false otherwise.
 */
bool fossil_ai_anom_save(fossil_ai_anom_t *detector, const char *filepath);

/**
 * @brief Loads the state of an anomaly detector from a file.
 *
 * This function reads the serialized state of a `fossil_ai_anom_t` object
 * from the specified file path and reconstructs the anomaly detector.
 *
 * @param filepath Path to the file from which the detector's state will be loaded.
 *
 * @return Pointer to a newly created `fossil_ai_anom_t` object with the loaded state,
 *         or NULL if loading fails.
 */
fossil_ai_anom_t *fossil_ai_anom_load(const char *filepath);

#ifdef __cplusplus
}

#include <vector>
#include <memory>

namespace fossil {

    namespace ai {

        /**
         * @brief C++ wrapper class for Fossil AI anomaly detection.
         *
         * This class provides a safe and convenient interface for managing anomaly detection
         * using the underlying C API. All resource management is handled automatically.
         * Method names use snake_case for consistency with the C API.
         */
        class Anomaly
        {
        public:
            /**
             * @brief Constructs a new anomaly detector instance.
             *
             * Allocates and initializes a new anomaly detector using fossil_ai_anom_create().
             * Throws std::runtime_error if allocation fails.
             */
            Anomaly()
                : detector_(fossil_ai_anom_create())
            {
                if (!detector_)
                    throw std::runtime_error("Failed to create anomaly detector");
            }

            /**
             * @brief Destructor. Frees the anomaly detector resources.
             */
            ~Anomaly()
            {
                if (detector_)
                    fossil_ai_anom_free(detector_);
            }

            /**
             * @brief Performs statistical anomaly detection using z-score method.
             *
             * @param data      Vector of float data points to analyze.
             * @param threshold Z-score threshold for anomaly detection.
             * @param score     Output variable for computed z-score.
             * @return true if anomaly detected, false otherwise.
             */
            bool detect_statistical(const std::vector<float>& data, float threshold, float& score)
            {
                return fossil_ai_anom_detect_statistical(
                    detector_,
                    data.data(),
                    data.size(),
                    threshold,
                    &score
                );
            }

            /**
             * @brief Performs deviation-based anomaly detection.
             *
             * @param data  Vector of float data points to analyze.
             * @param score Output variable for computed deviation score.
             * @return true if anomaly detected, false otherwise.
             */
            bool detect_deviation(const std::vector<float>& data, float& score)
            {
                return fossil_ai_anom_detect_deviation(
                    detector_,
                    data.data(),
                    data.size(),
                    &score
                );
            }

            /**
             * @brief Performs reconstruction-based anomaly detection using an autoencoder model.
             *
             * @param data        Vector of float data points to analyze.
             * @param autoencoder Pointer to autoencoder model (C API).
             * @param score       Output variable for computed reconstruction error score.
             * @return true if anomaly detected, false otherwise.
             */
            bool detect_reconstruction(const std::vector<float>& data, fossil_ai_jellyfish_model_t* autoencoder, float& score)
            {
                return fossil_ai_anom_detect_reconstruction(
                    detector_,
                    data.data(),
                    data.size(),
                    autoencoder,
                    &score
                );
            }

            /**
             * @brief Saves the anomaly detector state to a file.
             *
             * @param filepath Path to the file for saving state.
             * @return true if successful, false otherwise.
             */
            bool save(const std::string& filepath)
            {
                return fossil_ai_anom_save(detector_, filepath.c_str());
            }

            /**
             * @brief Loads anomaly detector state from a file.
             *
             * Replaces the current detector with the loaded state.
             * @param filepath Path to the file for loading state.
             * @return true if successful, false otherwise.
             */
            bool load(const std::string& filepath)
            {
                fossil_ai_anom_t* loaded = fossil_ai_anom_load(filepath.c_str());
                if (!loaded)
                    return false;
                if (detector_)
                    fossil_ai_anom_free(detector_);
                detector_ = loaded;
                return true;
            }

            /**
             * @brief Returns the underlying C anomaly detector pointer.
             */
            fossil_ai_anom_t* native_handle() const { return detector_; }

            // Non-copyable, movable
            anomaly(const anomaly&) = delete;
            anomaly& operator=(const anomaly&) = delete;
            anomaly(anomaly&& other) noexcept : detector_(other.detector_) { other.detector_ = nullptr; }
            anomaly& operator=(anomaly&& other) noexcept
            {
                if (this != &other)
                {
                    if (detector_)
                        fossil_ai_anom_free(detector_);
                    detector_ = other.detector_;
                    other.detector_ = nullptr;
                }
                return *this;
            }

        private:
            fossil_ai_anom_t* detector_;
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* AI_FRAMEWORK_H */
