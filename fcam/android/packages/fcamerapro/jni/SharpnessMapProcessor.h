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

	static ImageStack::Image process(float** list, int width, int height, int targetWidth, int targetHeight)
	{
		// single frame, single channel
		ImageStack::Image target(targetWidth, targetHeight, 1, 1);
		int w, h;
	    for (int tw = 0; tw < targetWidth; tw++ ) {
	      for (int th = 0; th < targetHeight; th++) {
	        w = floor(width * tw / targetWidth);
	        h = floor(height* th / targetHeight);

	        if (list[h][w] == -1) {
	        	list[h][w] = interpolateDepth(list, width, height, w, h);
	        }
	        // list is given in ROW-COLUMN order, which is equivalent to HEIGHT-WIDTH
	        *(target(tw,th)) = list[h][w];
	      }
	    }
	    return target;
	}

};


#endif /* SHAPRNESSMAPPROCESSOR_H_ */
