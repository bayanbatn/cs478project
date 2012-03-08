/*
 * AllFocusImageMerge.h
 *
 *  Created on: Mar 5, 2012
 *      Author: jasonch
 */

#ifndef ALLFOCUSIMAGEMERGE_H_
#define ALLFOCUSIMAGEMERGE_H_


#include <ImageStack/ImageStack.h>


class AllFocusImageMerge {
public:
	static ImageStack::Image process(ImageStack::Image *images, int numImages);

private:

};


#endif /* ALLFOCUSIMAGEMERGE_H_ */
