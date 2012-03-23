/*
 * ShaprnessMapProcessor.h
 *
 *  Created on: Feb 22, 2012
 *      Author: jasonch
 */

#ifndef SHAPRNESSMAPPROCESSOR_H_
#define SHAPRNESSMAPPROCESSOR_H_

#include <ImageStack/ImageStack.h>

class SharpnessMapProcessor {
private:
	static pair<float, float> calibratedCenter;
	static float magnificationCorrection;

public:

	// takes a list of [width x height] depth samples, returns a depth map of [targetWidth x targetHeight]
	static ImageStack::Image processSamples(int** list, int width, int height, int targetWidth, int targetHeight);
	static ImageStack::Image processSampleWithConfidence(int** list, int** values, int width, int height, int targetWidth, int targetHeight);
	// takes the depthmap and a reference image, uses bilateral filtering to smooth it
	static void processDepthMap(ImageStack::Image &depthmap, ImageStack::Image &reference);
	static void processDepthMap2(ImageStack::Image &depthmap, ImageStack::Image &reference,int size,double sigma);
};


#endif /* SHAPRNESSMAPPROCESSOR_H_ */
