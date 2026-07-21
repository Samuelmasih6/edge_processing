#ifndef CSV_READER_H
#define CSV_READER_H

#include "csi_features.h"

#include <stdbool.h>
#include <stdio.h>

FILE *csv_open(const char *filename);

void csv_close(FILE *fp);

bool csv_read_frame(
    FILE *fp,
    csi_frame_t *frame
);

#endif
