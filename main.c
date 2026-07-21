#include "csv_reader.h"
#include "csi_features.h"

#include <stdio.h>
#include <string.h>

#define STEP_SIZE 1


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
	    printf(
	        "Usage: %s input.csv\n",
	        argv[0]
	    );
	    return 1;
	}

	int window_id = 0;

    FILE *fp = csv_open(argv[1]);

    if (!fp)
    {
        printf("Cannot open file\n");
        return 1;
    }

    FILE *out = fopen("features.csv", "w");

	if (!out)
	{
	    printf("Cannot create features.csv\n");
	    return 1;
	}

	fprintf(
	    out,
	    "window,label,motion_energy,var_mean,var_max,"
	    "topk_variance_mean,topk_variance_max,amp_std,phase_std,"
	    "delta_mean,delta_std,delta_max,"
	    "rms\n"
	);

    char header[8192];
    fgets(header, sizeof(header), fp);

    csi_frame_t window[WINDOW_SIZE];

    uint16_t count = 0;

    csi_frame_t frame;

	while(csv_read_frame(fp, &frame))
	{
	    window[count++] = frame;
		feature_vector_t features;
	    if(count < WINDOW_SIZE)
	        continue;

	    extract_features(
	        window,
	        WINDOW_SIZE,
	        &features
	    );




        printf(
		    "Label=%s "
		    "Motion=%.4f "
		    "VarMean=%.4f "
		    "VarMax=%.4f "
			"topk_variance_mean=%.3f"
			"topk_variance_max=%.3f "
		    "AmpStd=%.4f "
		    "PhaseStd=%.4f "
			"DeltaMean=%.3f "
			"DeltaStd=%.3f "
			"DeltaMax=%.3f "
			"RMS=%.3f\n",
		    window[WINDOW_SIZE/2].label,
		    features.motion_energy,
		    features.variance_mean,
		    features.variance_max,
			features.topk_variance_max,
			features.topk_variance_mean,
		    features.amplitude_std,
		    features.phase_std,
			features.delta_phase_mean,
			features.delta_phase_std,
			features.delta_phase_max,
			features.rms_amplitude
		);

		fprintf(
		    out,
		    "%d,"
			"%s,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f,"
		    "%.4f\n",

		    window_id++,
			window[WINDOW_SIZE/2].label,
		    features.motion_energy,
		    features.variance_mean,
		    features.variance_max,
			features.topk_variance_mean,
			features.topk_variance_max,

		    features.amplitude_std,
		    features.phase_std,

		    features.delta_phase_mean,
		    features.delta_phase_std,
		    features.delta_phase_max,

		    features.rms_amplitude
		);

		memmove(
		    &window[0],
		    &window[STEP_SIZE],
		    (WINDOW_SIZE - STEP_SIZE)
		    * sizeof(csi_frame_t)
		);
		count = WINDOW_SIZE - STEP_SIZE;
        }


    fclose(out);
    csv_close(fp);

    return 0;
}
