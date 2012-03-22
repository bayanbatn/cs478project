#ifndef __CALIBRATION_MATRICES_H
#define __CALIBRATION_MATRICES_H

static float tforms[9][3][3] = {
{
    {0.9683, -0.0030, 10.8977},
    {0.0030, 0.9683, 7.8648},
         {0, 0, 1.0000}
},


{
    {0.9683, -0.0030, 10.8977},
    {0.0030, 0.9683, 7.8648},
         {0, 0, 1.0000}
},


{
    {0.9703, -0.0020, 10.1059},
    {0.0020, 0.9703, 7.8317},
         {0, 0, 1.0000}
},


{
    {0.9716, -0.0051, 10.2418},
    {0.0051, 0.9716, 6.6916},
         {0, 0, 1.0000}
},


{
    {0.9830, -0.0025, 5.9934},
    {0.0025, 0.9830, 4.0544},
         {0, 0, 1.0000}
},


{
    {0.9830, -0.0025, 5.9934},
    {0.0025, 0.9830, 4.0544},
         {0, 0, 1.0000}
},


{
    {0.9861, -0.0036, 5.2348},
    {0.0036, 0.9861, 3.2295},
         {0, 0, 1.0000}
},


{
    {0.9897, -0.0040, 4.3568},
    {0.0040, 0.9897, 2.4270},
         {0, 0, 1.0000}
},


{
    {0.9939, -0.0021, 2.4530},
    {0.0021, 0.9939, 1.1994},
         {0, 0, 1.0000}
}
};

static void matrixVectorMult(float mat[3][3], float vec[3], float result[3]) {
	for (int i = 0; i < 3; i++) {
		result[i] = 0;
		for (int j = 0; j < 3; j++) {
			result[i] += mat[i][j]*vec[j];
		}
	}
}

static void calibrateCoords(int& x, int& y, int depthInd) {
	// everything is calibrated to the closest focus,
	// so the transformation for that depth is identity
	if (depthInd == NUM_INTERVALS -1) return;

	float vec[3] = {x, y, 1.f};
	float result[3];

	matrixVectorMult(tforms[depthInd], vec, result);

	x = (int)(result[0]+0.5f);
	y = (int)(result[1]+0.5f);
	//LOG("calibrated %d (x, y) = (%d, %d)", depthInd, x, y);
}


#endif
