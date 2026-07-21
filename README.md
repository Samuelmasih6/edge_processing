# CSI Feature Extraction Pipeline

A lightweight C-based feature extraction pipeline for processing Wi-Fi Channel State Information (CSI) collected from ESP32 devices.

The project converts raw CSI packets into meaningful statistical features that can later be used for machine learning tasks such as human presence detection and activity recognition.

---

# Overview

Wi-Fi signals constantly interact with the surrounding environment. When a person walks, stands, or leaves a room, these signals change slightly.

Instead of using cameras or wearable sensors, this project analyzes those tiny changes in Wi-Fi signals and converts them into numerical features.

These features can later be used by machine learning algorithms to recognize human presence and activities.

---

# Project Workflow

```
                    Raw CSI CSV
                         │
                         ▼
               Read One Packet at a Time
                         │
                         ▼
              Convert IQ → Amplitude & Phase
                         │
                         ▼
                Sliding Window Buffer
                (Window = 100 packets)
                         │
                         ▼
              Feature Extraction Engine
                         │
                         ▼
                  Feature Dataset CSV
                         │
                         ▼
        Machine Learning / Classification
```

---

# Input Dataset

Each row represents one CSI packet.

Example:

```
label,I1,Q1,I2,Q2,...,I64,Q64
```

where

- Label = Activity label
- I = In-phase component
- Q = Quadrature component

Each packet contains CSI values from 64 Wi-Fi subcarriers.

---

# Streaming Processing

Instead of loading the complete dataset into memory, the pipeline processes CSI as a stream.

```
Packet arrives
      │
      ▼
Added to sliding buffer
      │
      ▼
Window becomes full
      │
      ▼
Extract Features
      │
      ▼
Slide Window
```

Current configuration:

```
WINDOW_SIZE = 100 packets
STEP_SIZE   = 20 packets
```

Windows generated:

```
1-100
21-120
41-140
61-160
...
```

This closely simulates real-time CSI processing.

---

# Feature Extraction

The following features are extracted from every sliding window.

## Motion Energy

Measures how much the CSI phase changes over time.

Higher values generally indicate more movement.

---

## Top-K Variance Mean

Computes the average variance of the most dynamic subcarriers.

Helps identify motion-sensitive Wi-Fi channels.

---

## Top-K Variance Max

Largest variance among the selected subcarriers.

Captures the strongest signal fluctuation.

---

## Variance Mean

Average variance across all subcarriers.

Represents overall CSI stability.

---

## Variance Max

Maximum variance observed across all subcarriers.

---

## Delta Phase Mean

Average phase difference between consecutive packets.

---

## Delta Phase Standard Deviation

Measures consistency of phase changes.

---

## Delta Phase Maximum

Largest phase jump within the window.

---

## Amplitude Standard Deviation

Variation in signal amplitude.

---

## Phase Standard Deviation

Variation in signal phase.

---

# Repository Structure

```
.
├── main.c
├── csv_reader.c
├── csv_reader.h
├── csi_features.c
├── csi_features.h
├── esp_iq.csv
├── features.csv
└── README.md
```

---

# Module Description

## main.c

Entry point of the application.

Responsible for:

- Reading CSI packets
- Maintaining sliding window
- Calling feature extraction
- Exporting features

---

## csv_reader.c

Reads CSI packets from CSV.

Responsibilities:

- Read one packet at a time
- Parse IQ values
- Convert IQ into amplitude and phase
- Handle missing CSI values
- Simulate packet streaming

---

## csi_features.c

Core feature extraction module.

Implements:

- Motion Energy
- Variance Features
- Top-K Selection
- Delta Phase Features
- Statistical Features

---

## csi_features.h

Common data structures and configuration.

Contains:

- CSI frame definition
- Feature vector definition
- Window size
- Top-K configuration

---

# Build

Compile using GCC:

```bash
gcc main.c csv_reader.c csi_features.c -o main -lm
```

---

# Run

```bash
./main esp_iq.csv
```

Output:

```
features.csv
```

---

# Output Format

Each row represents one sliding window.

Example:

```
window,
label,
motion_energy,
topk_variance_mean,
topk_variance_max,
variance_mean,
variance_max,
delta_mean,
delta_std,
delta_max,
amplitude_std,
phase_std
```

---

# Current Status

Completed

- CSI packet parser
- IQ to Amplitude/Phase conversion
- Sliding window processing
- Streaming pipeline
- Feature extraction
- CSV export

In Progress

- Feature visualization
- Machine learning classification
- Presence detection evaluation

Future Work

- Advanced CSI denoising
- Phase sanitization
- Frequency-domain features
- Activity recognition
- Pose estimation
- Real-time ESP32 integration

---

# Applications

This project can serve as the preprocessing stage for:

- Human Presence Detection
- Occupancy Detection
- Human Activity Recognition
- Smart Homes
- Indoor Sensing
- Device-Free Monitoring

---

# Technologies Used

- C
- ESP32
- Wi-Fi Channel State Information (CSI)
- Signal Processing
- Sliding Window Analysis
- Feature Engineering
- GCC



