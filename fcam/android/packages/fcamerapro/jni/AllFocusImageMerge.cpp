/*
 * AllFocusImageMerge.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: jasonch
 */

#include "AllFocusImageMerge.h"
#include "focusUtil.h"

static int getDepthIndex(float depthValue) {
	for (int i = 0; i < NUM_INTERVALS; i++) {
		if (depthValue >= 1.f / discreteDioptres[i])
			return i;
	}
	// default to the closest focus distance (?)
	return NUM_INTERVALS - 1;
}

// note this method assumes every image in the array is off the size
// and the last image is the depth map
ImageStack::Image AllFocusImageMerge::process(ImageStack::Image *images, int numImages) {
	ImageStack::Image allfocus(images[0].width, images[0].height, 1, 3);
	for (int y = 0; y < images[0].height; y++) {
		for (int x = 0; x < images[0].width; x++) {
			int depthIndex = getDepthIndex(images[numImages-1](x,y)[0]);
			for (int c = 0; c < images[depthIndex].channels; c++) {
				allfocus(x,y)[c] = images[depthIndex](x,y)[c];
			}
		}
	}
	return allfocus;
}


