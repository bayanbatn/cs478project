/*
 * DepthMapSmooth.cpp
 *
 *  Created on: 2012/2/21
 *      Author: tonychunwei
 */

#include "DepthMapSmooth.h"


DepthMapSmooth::DepthMapSmooth(int size,Image depthMap, Image refImage, float colorSigma):wndSize(size),depthMap(depthMap),refImage(refImage),colorSigma(colorSigma) {
	// TODO Auto-generated constructor stub

}

Image GetDepthMap(){
	ImageStack::JointBilateral::apply(depthMap,refImage, wndSize, wndSize, 1.0, colorSigma,ImageStack::GaussTransform::AUTO);
	return depthMap;
}

void SaveDepthMap(){

	static AsyncImageWriter *writer;
	ImageSet *is = writer->newImageSet(); // writer is a global instance of AsyncImageWriter already defined
	FCam::_Frame* f = new FCam::Tegra::_Frame;

	f->image = FCam::Image(depthMap.width, depthMap.height, FCam::RGB24);

	for (int y = 0; y < depthMap.height; y++) {
		for (int x = 0; x < depthMap.width; x++) {
			for (int c = 0; c < 3; c++) {
				((unsigned char*)f->image(x, y))[c] = (unsigned char)(255 * depthMap(x, y)[c] + 0.5f);
			}
		}
	}

	FileFormatDescriptor* fmt=new FileFormatDescriptor(FileFormatDescriptor::EFormatJPEG,80); // FORMAT, QUALITY
	is->add( *fmt, FCam::Frame(f));
	writer->push(is);
};


DepthMapSmooth::~DepthMapSmooth() {
	// TODO Auto-generated destructor stub
}

