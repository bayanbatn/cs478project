#ifndef BLURTOOL_H_
#define BLURTOOL_H_

#include <FCam/Tegra.h>
#include <FCam/Frame.h>
#include <FCam/Base.h>
#include <ImageStack/ImageStack.h>
#include <android/log.h>
#include "Common.h"
#include <cmath>

//#define SIGMA_SCALE 0.9 //TODO tune this
#define NUM_BLUR_LEVELS 4
#define CLTS_const 1.3; //10mm

static const float sigmas[] = {0.2, 0.4, 1.6, 6.4}; //TODO tune this


class BlurTool{

public:

	BlurTool(){
		doBlurImage = false;
	}

	//helper
	void printImagePath(ImageStack::Image gauss1, ImageStack::Image gauss2, ImageStack::Image final){
		std::ostringstream oss;

		//text += oss.str();
		oss << "DEPTH Printing first blur image! \n";
		for (int xx = 100; xx < 110; xx++)
		{
			for (int yy = 100; yy < 110; yy++)
			{
				int c = gauss1(xx, yy)[0];
				oss << c << " ";
			}
			oss << "\n";
		}
		//std::String print = oss.str();
		LOG("%s", oss.str());
		oss.flush();

		oss << "DEPTH Printing second blur image! \n";
		for (int xx = 100; xx < 110; xx++)
		{
			for (int yy = 100; yy < 110; yy++)
			{
				int c = gauss2(xx, yy)[0];
				oss << c << " ";
			}
			oss << "\n";
		}
		LOG("%s", oss.str());
		oss.flush();

		oss << "DEPTH Printing final result image! \n";
		for (int xx = 100; xx < 110; xx++)
		{
			for (int yy = 100; yy < 110; yy++)
			{
				int c = final(xx, yy)[0];
				oss << c << " ";
			}
			oss << "\n";
		}
		LOG("%s", oss.str());
		oss.flush();

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
		ImageStack::FastBlur::apply(gauss1, sigmas[0], sigmas[0], 0.0f);
		ImageStack::FastBlur::apply(gauss2, sigmas[1], sigmas[1], 0.0f);
		ImageStack::FastBlur::apply(gauss3, sigmas[2], sigmas[2], 0.0f);
		ImageStack::FastBlur::apply(gauss4, sigmas[3], sigmas[3], 0.0f);

		LOG("DEPTH Blur fast blur without problem\n");
		float inv_focus = 1/focusDepth;

		FCam::_Frame* f = new FCam::Tegra::_Frame;
		f->image = FCam::Image(original.width, original.height, FCam::RGB24);
		for (int y = 0; y < original.height; y++) {
			for (int x = 0; x < original.width; x++) {
				//unsigned char t = depthMap(x,y)[0];
				float true_depth = depthMap(x,y)[0];
				if (true_depth < 0.08f) true_depth = 0.1f;
				float blur_radius = fabs(1.3f * (1/true_depth - inv_focus));

				float w1=0.0f; float w2=0.0f;
				ImageStack::Image *blur1, *blur2;

				if (x <= 10 && y <= 10) LOG("DEPTH blur radius is: %f, tmp: %f, inv_focus: %f\n", blur_radius, true_depth, inv_focus);//if (x == 0 && y == 0)

				if (blur_radius < sigmas[0]){
					w2 = blur_radius / sigmas[0];
					w1 = 1 - w2;
					blur1 = &original;
					blur2 = &gauss1;
				}else if(blur_radius < sigmas[1]){
					w2 = (blur_radius - sigmas[0])/(sigmas[1] - sigmas[0]);
					w1 = 1 - w2;
					blur1 = &gauss1;
					blur2 = &gauss2;
				}else if(blur_radius < sigmas[2]){
					w2 = (blur_radius - sigmas[1])/(sigmas[2] - sigmas[1]);
					w1 = 1 - w2;
					blur1 = &gauss2;
					blur2 = &gauss3;
				}else if(blur_radius < sigmas[3]){
					w2 = (blur_radius - sigmas[2])/(sigmas[3] - sigmas[2]);
					w1 = 1 - w2;
					blur1 = &gauss3;
					blur2 = &gauss4;
				}else{
					w1 = 1.0f;
					w2 = 0.0f;
					blur1 = &gauss4;
					blur2 = &gauss4; //doesn't matter what this is;
				}

				if (x <= 10 && y <= 10) LOG("DEPTH weights are: %f, %f\n", w1, w2);//if (x == 0 && y == 0)

				for (int c = 0; c < 3; c++) {
					((unsigned char*)f->image(x, y))[c] = (unsigned char) (w1 * (unsigned char)(255 * (*blur1)(x, y)[c] + 0.5f)
																		 + w2 * (unsigned char)(255 * (*blur2)(x, y)[c] + 0.5f));
					if (x <= 10 && y <= 10) LOG("DEPTH blurred pixels are %f, %f, %d\n", (*blur1)(x, y)[c], (*blur2)(x, y)[c], ((unsigned char*)f->image(x, y))[c]);//if (x == 0 && y == 0)
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
