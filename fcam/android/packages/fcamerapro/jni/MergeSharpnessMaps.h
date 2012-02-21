/*
 * MergeSharpnessMaps.h
 *
 *  Created on: Feb 20, 2012
 *      Author: jasonch
 */

#ifndef MERGESHARPNESSMAPS_H_
#define MERGESHARPNESSMAPS_H_

#include <vector.h>
#include <ImageStack/ImageStack.h>

// forward declaration
struct coord3D;

class MergeSharpnessMaps  {
public:
	/* Takes the list of (x,y,z) coordinate samples,
	 * populate an Image of size targetWidth x targetHeight
	 * with the z-coordinate copied over each entire patch
	 */
	static ImageStack::Image processSharpnessMap(const float** list,
			int width, int height,
			int targetWidth, int targetHeight)
	{
		ImageStack::Image target(targetWidth, targetHeight, 1, 1);

		int w = 0, h = 0;
		for (int tw = 0; tw < targetWidth; tw++ ) {
			for (int th = 0; th < targetHeight; th++) {
				w = tw * width / targetWidth;
				h = th * height/ targetHeight;
				*(target(tw, th)) = list[w][h];
			}
		}
	}
};


#endif /* MERGESHARPNESSMAPS_H_ */
