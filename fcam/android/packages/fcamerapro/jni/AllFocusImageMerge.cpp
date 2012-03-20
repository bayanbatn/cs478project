/*
 * AllFocusImageMerge.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: jasonch
 */

#include "AllFocusImageMerge.h"
#include "focusUtil.h"
#include "Common.h"
#include "CalibrationMatrices.h"

static int getDepthIndex(float depthValue) {
	for (int i = 0; i < NUM_INTERVALS; i++) {
		if (depthValue >= 1.f / discreteDioptres[i])
			return i;
	}
	// default to the closest focus distance (?)
	return NUM_INTERVALS - 1;
}

// using optical flow to align all images to the first image
static void alignImages(ImageStack::Image *images, int num) {
	LOG("aligning images");
	for (int i = 1; i < num - 5; i++) {
		images[i] = ImageStack::OpticalFlow::apply(images[0], images[i]);
	}
	LOG("done aligning images");
}


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

// note this method assumes every image in the array is of the same size
// and the last image is the depth map
// numImages should probably always equal NUM_INTERVALS+1
ImageStack::Image AllFocusImageMerge::process(ImageStack::Image *images, int numImages) {
	// optical flow is way too slow to compute on the tablet, even once
	// alignImages(images, numImages - 1);

	ImageStack::Image allfocus(images[0].width, images[0].height, 1, 3);
	for (int y = 0; y < images[0].height; y++) {
		for (int x = 0; x < images[0].width; x++) {
			int depthIndex = getDepthIndex(images[numImages-1](x,y)[0]);

			int realx = x, realy = y;
			calibrateCoords(realx,realy, depthIndex);

			for (int c = 0; c < images[depthIndex].channels; c++) {
				allfocus(x,y)[c] = images[depthIndex](realx,realy)[c];
			}
		}
	}
	return allfocus;
}


