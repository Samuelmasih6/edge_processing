#ifndef CSI_FEATURES_H
#define CSI_FEATURES_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_SUBCARRIERS 64
#define WINDOW_SIZE     100
#define TOP_K           20

/* ================================
 * CSI Frame
 * ================================ */
typedef struct {
    float amplitude[MAX_SUBCARRIERS];
    float phase[MAX_SUBCARRIERS];
    char  label[32];
} csi_frame_t;

/* ================================
 * Welford Running Statistics
 * ================================ */
typedef struct {
    double mean;
    double m2;
    uint32_t count;
} welford_t;

/* ================================
 * Extracted Feature Vector
 * ================================ */
typedef struct {
    float motion_energy;

    float variance_mean;
    float variance_max;

    float amplitude_mean;
    float amplitude_std;

    float phase_mean;
    float phase_std;

    float delta_phase_mean;
    float delta_phase_std;
    float delta_phase_max;

    float rms_amplitude;
    float topk_variance_mean;
    float topk_variance_max;
} feature_vector_t;

/* ================================
 * IQ Processing
 * ================================ */

/* Convert I/Q pair to amplitude */
float extract_amplitude(float i_val, float q_val);

/* Convert I/Q pair to phase */
float extract_phase(float i_val, float q_val);

/* Phase unwrapping */
float unwrap_phase(float previous, float current);

/* ================================
 * Welford Statistics
 * ================================ */

void welford_reset(welford_t *w);

void welford_update(welford_t *w, double x);

double welford_variance(const welford_t *w);

/* ================================
 * Feature Extraction
 * ================================ */

/* Compute motion energy from phase history */
float compute_motion_energy(
    const float history[],
    uint16_t length
);

/* Mean of array */
float compute_mean(
    const float data[],
    uint16_t length
);

/* Standard deviation */
float compute_std(
    const float data[],
    uint16_t length
);

/* Select top-k subcarriers by variance */
void select_top_k(
    const double variances[],
    uint16_t n_subcarriers,
    uint16_t k,
    uint16_t top_k_indices[]
);

/* Extract complete feature vector */
void extract_features(
    const csi_frame_t window[],
    uint16_t window_size,
    feature_vector_t *features
);

/*compute RMS */
float compute_rms(
    const float data[],
    uint16_t length
);

/*delta phase */
void compute_delta_phase_features(
    const float phase_history[],
    uint16_t length,
    float *mean_delta,
    float *std_delta,
    float *max_delta
);

#endif /* CSI_FEATURES_H */
