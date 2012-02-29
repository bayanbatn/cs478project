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
	static void calibrate() {}

	static void adjustSharpnessMap(float** list) {}

	static ImageStack::Image process(int** list, int width, int height, int targetWidth, int targetHeight)
	{
		// single frame, single channel
		ImageStack::Image target(targetWidth, targetHeight, 1, 1);
		int w, h;
	    for (int tw = 0; tw < targetWidth; tw++ ) {
	      for (int th = 0; th < targetHeight; th++) {
	        w = floor(width * tw / targetWidth);
	        h = floor(height* th / targetHeight);

	        // list is given in ROW-COLUMN order, which is equivalent to HEIGHT-WIDTH
	        *(target(tw,th)) = 0.0f;//list[h][w];
	      }
	    }
	    return target;
	}

};


#endif /* SHAPRNESSMAPPROCESSOR_H_ */
