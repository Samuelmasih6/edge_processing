#include "csi_features.h"

#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846
/* ==================================
 * IQ Processing
 * ================================== */

float extract_amplitude(float i_val, float q_val)
{
    return sqrtf(
        (float)i_val * (float)i_val +
        (float)q_val * (float)q_val
    );
}

float extract_phase(float i_val, float q_val)
{
    return atan2f(
        (float)q_val,
        (float)i_val
    );
}

float unwrap_phase(float previous, float current)
{
    float diff = current - previous;

    if (diff > M_PI)
        diff -= 2.0f * M_PI;
    else if (diff < -M_PI)
        diff += 2.0f * M_PI;

    return previous + diff;
}

/* ==================================
 * Welford Statistics
 * ================================== */

void welford_reset(welford_t *w)
{
    w->mean = 0.0;
    w->m2 = 0.0;
    w->count = 0;
}

void welford_update(welford_t *w, double x)
{
    w->count++;

    double delta = x - w->mean;

    w->mean += delta / (double)w->count;

    double delta2 = x - w->mean;

    w->m2 += delta * delta2;
}

double welford_variance(const welford_t *w)
{
    if (w->count < 2)
        return 0.0;

    return w->m2 / (double)(w->count - 1);
}

/* ==================================
 * Basic Statistics
 * ================================== */

float compute_mean(
    const float data[],
    uint16_t length
)
{
    if (length == 0)
        return 0.0f;

    float sum = 0.0f;

    for (uint16_t i = 0; i < length; i++)
        sum += data[i];

    return sum / (float)length;
}

float compute_std(
    const float data[],
    uint16_t length
)
{
    if (length < 2)
        return 0.0f;

    float mean = compute_mean(data, length);

    float sum_sq = 0.0f;

    for (uint16_t i = 0; i < length; i++)
    {
        float diff = data[i] - mean;
        sum_sq += diff * diff;
    }

    return sqrtf(sum_sq / (float)(length - 1));
}

/* ==================================
 * Motion Energy
 * ================================== */
float compute_motion_energy(
    const float history[],
    uint16_t length
)
{
    if (length < 2)
        return 0.0f;

    float mean = compute_mean(history, length);

    float variance = 0.0f;

    for (uint16_t i = 0; i < length; i++)
    {
        float diff = history[i] - mean;
        variance += diff * diff;
    }

    variance /= (float)(length - 1);

    return variance;
}

float compute_topk_motion_energy(
    const csi_frame_t window[],
    uint16_t window_size,
    const uint16_t top_k_indices[]
)
{
    float total_energy = 0.0f;

    for(uint16_t k = 0; k < TOP_K; k++)
    {
        float history[WINDOW_SIZE];

        uint16_t sc =
            top_k_indices[k];

        for(uint16_t pkt = 0;
            pkt < window_size;
            pkt++)
        {
            history[pkt] =
                window[pkt].phase[sc];
        }

        total_energy +=
            compute_motion_energy(
                history,
                window_size
            );
    }

    return total_energy / TOP_K;
}

/* ==================================
 * Top-K Variance Selection
 * ================================== */

void select_top_k(
    const double variances[],
    uint16_t n_subcarriers,
    uint16_t k,
    uint16_t top_k_indices[]
)
{
    if (k > n_subcarriers)
        k = n_subcarriers;

    bool used[MAX_SUBCARRIERS];

    memset(used, 0, sizeof(used));

    for (uint16_t rank = 0; rank < k; rank++)
    {
        double best_var = -1.0;
        uint16_t best_idx = 0;

        for (uint16_t sc = 0;
             sc < n_subcarriers;
             sc++)
        {
            if (used[sc])
                continue;

            if (variances[sc] > best_var)
            {
                best_var = variances[sc];
                best_idx = sc;
            }
        }

        top_k_indices[rank] = best_idx;
        used[best_idx] = true;
    }
}

//implement delta phase feature
void compute_delta_phase_features(
    const float phase_history[],
    uint16_t length,
    float *mean_delta,
    float *std_delta,
    float *max_delta
)
{
    if(length < 2)
    {
        *mean_delta = 0;
        *std_delta = 0;
        *max_delta = 0;
        return;
    }

    float deltas[WINDOW_SIZE];

    uint16_t n = 0;

    for(uint16_t i = 1; i < length; i++)
    {
        deltas[n++] =
            fabsf(
                phase_history[i] -
                phase_history[i-1]
            );
    }

    *mean_delta =
        compute_mean(deltas, n);

    *std_delta =
        compute_std(deltas, n);

    *max_delta = 0;

    for(uint16_t i = 0; i < n; i++)
    {
        if(deltas[i] > *max_delta)
            *max_delta = deltas[i];
    }
}



/* ==================================
 * Feature Extraction
 * ================================== */

void extract_features(
    const csi_frame_t window[],
    uint16_t window_size,
    feature_vector_t *features
)
{
    memset(features, 0, sizeof(feature_vector_t));

    if (window_size == 0)
        return;

    double variances[MAX_SUBCARRIERS];

    for (uint16_t sc = 0;
         sc < MAX_SUBCARRIERS;
         sc++)
    {
        welford_t w;
        welford_reset(&w);

        for (uint16_t pkt = 0;
             pkt < window_size;
             pkt++)
        {
            welford_update(
                &w,
                window[pkt].phase[sc]
            );
        }

        variances[sc] =
            welford_variance(&w);
    }
    uint16_t top_k_indices[TOP_K];

	select_top_k(
	    variances,
	    MAX_SUBCARRIERS,
	    TOP_K,
	    top_k_indices
	);

	float topk_mean = 0.0f;
	float topk_max = 0.0f;

	for(uint16_t k = 0; k < TOP_K; k++)
	{
	    float v =
	        (float)variances[
	            top_k_indices[k]
	        ];

	    topk_mean += v;

	    if(v > topk_max)
	        topk_max = v;
	}

	topk_mean /= TOP_K;

	features->topk_variance_mean = topk_mean;

	features->topk_variance_max = topk_max;

    double variance_sum = 0.0;
    double variance_max = 0.0;

    for (uint16_t sc = 0;
         sc < MAX_SUBCARRIERS;
         sc++)
    {
        variance_sum += variances[sc];

        if (variances[sc] > variance_max)
            variance_max = variances[sc];
    }

    features->variance_mean =
        (float)(variance_sum /
                MAX_SUBCARRIERS);

    features->variance_max =
        (float)variance_max;

    float phase_history[WINDOW_SIZE];

	for (uint16_t pkt = 0;
	     pkt < window_size;
	     pkt++)
	{
    	float avg_phase = 0.0f;

	    for (uint16_t k = 0;
	         k < TOP_K;
	         k++)
	    {
	        avg_phase +=
	            window[pkt].phase[
	                top_k_indices[k]
	            ];
	    }

	    avg_phase /= TOP_K;

	    phase_history[pkt] =
	        avg_phase;
	}


    features->motion_energy =
    		compute_topk_motion_energy(
	        window,
	        window_size,
	        top_k_indices
	     );

    compute_delta_phase_features(
	    phase_history,
	    window_size,
	    &features->delta_phase_mean,
	    &features->delta_phase_std,
	    &features->delta_phase_max
    );

    float amplitudes[
        WINDOW_SIZE *
        MAX_SUBCARRIERS
    ];


    float phases[
        WINDOW_SIZE *
        MAX_SUBCARRIERS
    ];

    uint32_t idx = 0;

    for (uint16_t pkt = 0;
         pkt < window_size;
         pkt++)
    {
        for (uint16_t sc = 0;
             sc < MAX_SUBCARRIERS;
             sc++)
        {
            amplitudes[idx] =
                window[pkt].amplitude[sc];

            phases[idx] =
                window[pkt].phase[sc];

            idx++;
        }
    }

    features->amplitude_mean =
        compute_mean(
            amplitudes,
            idx
        );

    features->amplitude_std =
        compute_std(
            amplitudes,
            idx
        );

    features->rms_amplitude =
    	compute_rms(
        	amplitudes,
         	idx
     	);

    features->phase_mean =
        compute_mean(
            phases,
            idx
        );

    features->phase_std =
        compute_std(
            phases,
            idx
        );
}

// RMS
float compute_rms(
    const float data[],
    uint16_t length
)
{
    if(length == 0)
        return 0.0f;

    float sum = 0.0f;

    for(uint16_t i = 0; i < length; i++)
    {
        sum += data[i] * data[i];
    }

    return sqrtf(sum / length);
}
