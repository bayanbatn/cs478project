/*
 * ImageInsertion.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: tonychunwei
 */

#include "ImageInsertion.h"
#include "focusUtil.h"

static int getDepthIndex(float depthValue) {
	for (int i = 0; i < NUM_INTERVALS; i++) {
		if (depthValue >= 1.f / discreteDioptres[i])
			return i;
	}
	// default to the closest focus distance (?)
	return NUM_INTERVALS - 1;
}



ImageInsertion::ImageInsertion() {
	// TODO Auto-generated constructor stub

}

ImageInsertion::~ImageInsertion() {
	// TODO Auto-generated destructor stub
}

ImageStack::Image ImageInsertion::process(ImageStack::Image *images1, ImageStack::Image *images2){
    //int diff[images1[0].width * images1[0].height];
    //int mask[images1[0].width * images1[0].height];
	ImageStack::Image allfocus(images1[0].width, images1[0].height, 1, 3);
		for (int y = 0; y < images1[0].height; y++) {
			for (int x = 0; x < images1[0].width; x++) {
				int depthIndex1 = getDepthIndex(images1[1](x,y)[0]);
				int depthIndex2 = getDepthIndex(images2[1](x,y)[0]);


				if (depthIndex1>=depthIndex2){
					for (int c = 0; c < images1[0].channels; c++) {
						allfocus(x,y)[c] = images1[0](x,y)[c];
					}

				}
				else if(depthIndex1<depthIndex2){
					for (int c = 0; c < images2[0].channels; c++) {
						allfocus(x,y)[c] = images2[0](x,y)[c];
					}
				}







				/*
				else{
					for (int c = 0; c < images2[0].channels; c++) {

						 // method1 : drawback:ghost image

						allfocus(x,y)[c] = (images1[0](x,y)[c]+images2[0](x,y)[c])/2;


						 //  method2



					}
				}
				*/



			}
		}
		return allfocus;
};
