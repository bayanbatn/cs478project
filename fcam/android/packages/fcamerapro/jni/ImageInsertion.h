/*
 * ImageInsertion.h
 *
 *  Created on: Mar 20, 2012
 *      Author: tonychunwei
 */

#ifndef IMAGEINSERTION_H_
#define IMAGEINSERTION_H_

#include <ImageStack/ImageStack.h>

class ImageInsertion {
public:
	ImageInsertion();
	virtual ~ImageInsertion();
	static ImageStack::Image process(ImageStack::Image *images1, ImageStack::Image *images2);
};

#endif /* IMAGEINSERTION_H_ */
