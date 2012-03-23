/*
 * SharpnessMapProcessor.cpp
 *
 *  Created on: Mar 7, 2012
 *      Author: jasonch
 */
#include "SharpnessMapProcessor.h"
#include "FocusUtil.h"


static float interpolateDepth(float** list, int width, int height, int w, int h) {

	float convo[9] = {1,2,1,2,4,2,1,2,1};

	float total = 0;
	int count = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int row = h + i;
			int col = w + j;

			if (row < 0 || row >= height || col < 0 || col >= width) continue;
			total += convo[3*(i+1) + j+1] * list[row][col];
			count += 1;
		}
	}
	if (count == 0) return 0;
	return total/count;
}

static int findModeDepth(int** list, int width, int height, int w, int h) {
	int count[NUM_INTERVALS] = {0};

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int row = h + i;
			int col = w + j;

			if (row < 0 || row >= height || col < 0 || col >= width) continue;
			count[ list[row][col] ]++;
		}
	}


	int maxCount = count[0], maxInd = 0;
	for (int i = 1; i < NUM_INTERVALS; i++) {
		if (maxCount < count[i]) {
			maxCount = count[i];
			maxInd = i;
		}
	}
	return maxInd;


}


static int findConfidenceDepth(int** list, int** sharpnessvalue, int width, int height, int w, int h) {
	int sum[NUM_INTERVALS] = {0};
	int count[NUM_INTERVALS] = {0};
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int row = h + i;
			int col = w + j;

			if (row < 0 || row >= height || col < 0 || col >= width) continue;
			sum[ list[row][col] ]+= sharpnessvalue[row][col];
			count[ list[row][col] ]++;
		}
	}


	int maxSum = sum[0], maxInd = 0;
		for (int i = 1; i < NUM_INTERVALS; i++) {
			if (maxSum < sum[i]) {
				maxSum = sum[i];
				maxInd = i;
			}
		}
/*

	int maxSum = sum[0]*count[0], maxInd = 0;
	for (int i = 1; i < NUM_INTERVALS; i++) {
		if (maxSum < sum[i]*count[i]) {
			maxSum = sum[i]*count[i];
			maxInd = i;
		}
	}
*/
	return maxInd;
}


static void modeSmoothing(ImageStack::Image &target, int **depthList, int width, int height) {
	int w, h;
	for (int tw = 0; tw < target.width; tw++ ) {
		for (int th = 0; th < target.height; th++) {
			w = floor(width * tw / target.width);
			h = floor(height* th / target.height);

			if (depthList[h][w] == -1) {
				depthList[h][w] = findModeDepth(depthList, width, height, w, h);
			}
			// list is given in ROW-COLUMN order, which is equivalent to HEIGHT-WIDTH
			*(target(tw,th)) = 1./ discreteDioptres[depthList[h][w]];
		}
	}
}

static void confidenceSmoothing(ImageStack::Image &target, int **depthList,int **valueList, int width, int height) {
	int w, h;
	for (int tw = 0; tw < target.width; tw++ ) {
		for (int th = 0; th < target.height; th++) {
			w = floor(width * tw / target.width);
			h = floor(height* th / target.height);

			if (depthList[h][w] == -1) {
				depthList[h][w] = findConfidenceDepth(depthList, valueList, width, height, w, h);
			}
			// list is given in ROW-COLUMN order, which is equivalent to HEIGHT-WIDTH
			*(target(tw,th)) = 1./ discreteDioptres[depthList[h][w]];
		}
	}
}




ImageStack::Image SharpnessMapProcessor::processSamples(int** list, int width, int height, int targetWidth, int targetHeight)
{
	// single frame, single channel (black & white)
	ImageStack::Image target(targetWidth, targetHeight, 1, 1);
	modeSmoothing(target, list, width, height);

	return target;
}

ImageStack::Image SharpnessMapProcessor::processSampleWithConfidence(int** list, int** values, int width, int height, int targetWidth, int targetHeight)
{
	// single frame, single channel (black & white)
	ImageStack::Image target(targetWidth, targetHeight, 1, 1);
	confidenceSmoothing(target, list, values,width, height);
	return target;
}





void SharpnessMapProcessor::processDepthMap(ImageStack::Image &depthmap, ImageStack::Image &reference) {
	// first sharpen the reference image to enhance the edges
	//reference = ImageStack::BilateralSharpen::apply(reference, .2, .7, 5);
	// use the reference image to apply bilateral filter
	ImageStack::JointBilateral::apply(depthmap, reference, 50, 50, 1, 0.05);
}

