/*
 * DepthMapSmooth.h
 *
 *  Created on: 2012/2/21
 *      Author: tonychunwei
 */
#include "AsyncImageWriter.h"
#include <ImageStack/ImageStack.h>


#ifndef DEPTHMAPSMOOTH_H_
#define DEPTHMAPSMOOTH_H_

class DepthMapSmooth {
private:
	int wndSize;
	Image depthMap;
	Image refImage;
	float colorSigma;

public:
	DepthMapSmooth(int size,Image depthMap, Image refImage, float colorSigma){};
	Image GetDepthMap();
	void SaveDepthMap();
	virtual ~DepthMapSmooth();
};

#endif /* DEPTHMAPSMOOTH_H_ */
