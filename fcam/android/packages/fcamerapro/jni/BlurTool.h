#ifndef BLURTOOL_H_
#define BLURTOOL_H_

#include <FCam/Tegra.h>
#include <FCam/Frame.h>
#include <FCam/Base.h>
#include <ImageStack/ImageStack.h>
#include <android/log.h>
#include "Common.h"
#include <cmath>

#define SIGMA_SCALE 5 //TODO tune this
#define NUM_BLUR_LEVELS 4

static const float sigmas[] = {0.2, 0.4, 1.6, 6.4}; //TODO tune this


class BlurTool{

public:

	BlurTool(){
		doBlurImage = false;
	}

	FCam::Frame blurImage(const char* origImgPath, const char* depthMapPath, float focusDepth){

		LOG("DEPTH Blur image begin\n");

		//make copy of the original image, blur and save into the stack
		ImageStack::Image original = ImageStack::FileJPG::load(origImgPath);
		ImageStack::Image depthMap = ImageStack::FileJPG::load(depthMapPath);
		ImageStack::Image gauss1 = ImageStack::FileJPG::load(origImgPath);
		ImageStack::Image gauss2 = ImageStack::FileJPG::load(origImgPath);
		ImageStack::Image gauss3 = ImageStack::FileJPG::load(origImgPath);
		ImageStack::Image gauss4 = ImageStack::FileJPG::load(origImgPath);

		LOG("DEPTH Blur image loads passed\n");

		//TODO blur the image actually
		//ImageStack::JointBilateral::apply(original, depthMap, 4.0f, 4.0f, 0.0f, 0.4f);
		ImageStack::FastBlur::apply(gauss1, SIGMA_SCALE*sigmas[0], SIGMA_SCALE*sigmas[0], 0.0f);
		ImageStack::FastBlur::apply(gauss2, SIGMA_SCALE*sigmas[1], SIGMA_SCALE*sigmas[1], 0.0f);
		ImageStack::FastBlur::apply(gauss3, SIGMA_SCALE*sigmas[2], SIGMA_SCALE*sigmas[2], 0.0f);
		ImageStack::FastBlur::apply(gauss4, SIGMA_SCALE*sigmas[3], SIGMA_SCALE*sigmas[3], 0.0f);//TODO might have to square this?

		LOG("DEPTH Blur fast blur without problem\n");

		FCam::_Frame* f = new FCam::Tegra::_Frame;
		f->image = FCam::Image(original.width, original.height, FCam::RGB24);
		for (int y = 0; y < original.height; y++) {
			for (int x = 0; x < original.width; x++) {
				float focus_diff = abs(focusDepth - depthMap(x,y)[0]);
				focus_diff = focus_diff / depthMap(x,y)[0];

				float w1=0.0f; float w2=0.0f;
				ImageStack::Image *blur1, *blur2;

				if (x == 0 && y == 0) LOG("DEPTH focus diff is: %f\n", focus_diff);

				if (focus_diff < sigmas[0]){
					w2 = focus_diff / sigmas[0];
					w1 = 1 - w2;
					blur1 = &original;
					blur2 = &gauss1;
					//((unsigned char*)f->image(x, y))[c] = w1 * (unsigned char)(255 * original(x, y)[c] + 0.5f)
					//								    + w2 * (unsigned char)(255 * gauss1(x, y)[c] + 0.5f);
				}else if(focus_diff < sigmas[1]){
					w2 = (focus_diff - sigmas[0])/(sigmas[1] - sigmas[0]);
					w1 = 1 - w2;
					blur1 = &gauss1;
					blur2 = &gauss2;
					//((unsigned char*)f->image(x, y))[c] = w1 * (unsigned char)(255 * gauss1(x, y)[c] + 0.5f)
					//								    + w2 * (unsigned char)(255 * gauss2(x, y)[c] + 0.5f);
				}else if(focus_diff < sigmas[2]){
					w2 = (focus_diff - sigmas[1])/(sigmas[2] - sigmas[1]);
					w1 = 1 - w2;
					blur1 = &gauss2;
					blur2 = &gauss3;
					//((unsigned char*)f->image(x, y))[c] = w1 * (unsigned char)(255 * gauss2(x, y)[c] + 0.5f)
					//								    + w2 * (unsigned char)(255 * gauss3(x, y)[c] + 0.5f);
				}else if(focus_diff < sigmas[3]){
					w2 = (focus_diff - sigmas[2])/(sigmas[3] - sigmas[2]);
					w1 = 1 - w2;
					blur1 = &gauss3;
					blur2 = &gauss4;
					//((unsigned char*)f->image(x, y))[c] = w1 * (unsigned char)(255 * gauss3(x, y)[c] + 0.5f)
					//								    + w2 * (unsigned char)(255 * gauss4(x, y)[c] + 0.5f);
				}else{
					w1 = 1.0f;
					w2 = 0.0f;
					blur1 = &gauss4;
					blur2 = &gauss4; //doesn't matter what this is;
					//((unsigned char*)f->image(x, y))[c] = (unsigned char)(255 * gauss4(x, y)[c] + 0.5f)
				}

				if (x == 0 && y == 0) LOG("DEPTH weights are: %f, %f\n", w1, w2);

				for (int c = 0; c < 3; c++) {
					((unsigned char*)f->image(x, y))[c] = w1 * (unsigned char)(255 * (*blur1)(x, y)[c] + 0.5f)
														+ w2 * (unsigned char)(255 * (*blur2)(x, y)[c] + 0.5f);
				}
			}
		}

		LOG("DEPTH Blur image end\n");

		return FCam::Frame(f);
	}

	bool doBlurImage;

private:
	//empty
};


#endif /* BLURTOOL_H_ */
