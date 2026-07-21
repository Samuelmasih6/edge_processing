#include "csv_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 8192

FILE *csv_open(const char *filename)
{
    return fopen(filename, "r");
}

void csv_close(FILE *fp)
{
    if (fp)
        fclose(fp);
}

bool csv_read_frame(
    FILE *fp,
    csi_frame_t *frame)
{
    if (!fp || !frame)
        return false;

    char line[LINE_BUFFER_SIZE];

    if (!fgets(line, sizeof(line), fp))
        return false;

    static int line_number = 0;
    line_number++;

    line[strcspn(line, "\r\n")] = '\0';

    char *token = strtok(line, ",");

    if (!token)
    {
        printf(
            "Skipping line %d (missing label)\n",
            line_number);
        return csv_read_frame(fp, frame);
    }

    strncpy(
        frame->label,
        token,
        sizeof(frame->label) - 1);

    frame->label[
        sizeof(frame->label) - 1] = '\0';

    int sc = 0;

    for (sc = 0;
         sc < MAX_SUBCARRIERS;
         sc++)
    {
        token = strtok(NULL, ",");

        if (!token || strlen(token) == 0)
        {
            break;
        }

        float i_val = atof(token);

        token = strtok(NULL, ",");

        if (!token || strlen(token) == 0)
        {
            break;
        }

        float q_val = atof(token);

        frame->amplitude[sc] =
            extract_amplitude(
                (int8_t)i_val,
                (int8_t)q_val);

        frame->phase[sc] =
            extract_phase(
                (int8_t)i_val,
                (int8_t)q_val);
    }

    for (; sc < MAX_SUBCARRIERS; sc++)
    {
        frame->amplitude[sc] = 0.0f;
        frame->phase[sc] = 0.0f;
    }

    return true;
}
