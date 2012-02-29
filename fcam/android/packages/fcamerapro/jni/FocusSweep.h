
#ifndef FOCUSSWEEP_H_
#define FOCUSSWEEP_H_

#include <FCam/Tegra/AutoFocus.h>
#include <FCam/Tegra/Lens.h>
#include <FCam/Base.h>
#include <android/log.h>
#include <vector>
#include "FocusUtil.h"
#include "Common.h"

//Number of discrete focal lengths we sweep through
#define RECT_EDGE_LEN 			20
#define IMAGE_WIDTH 			640
#define IMAGE_HEIGHT 			480
#define FILTER_SIZE 			5
#define NUM_NIGHT_IMAGES 		2
//Face Detect
#define FD_MAX_FRAMES			3
//Focus states
#define SWEEP_FIN_PHASE			2
#define SWEEP_PHASE 			1
#define WAIT_PHASE 				0
//Max number of areas to focus
#define NUM_RECTS_X 			24
#define NUM_RECTS_Y 			16

typedef unsigned char uchar;

struct point3d
{
	int x;
	int y;
	float z;

	point3d(int x_pixel, int y_pixel, float z_depth)
	{
		x = x_pixel;
		y = y_pixel;
		z = z_depth;
	}
}typedef point3d;

class FocusSweep{// : public FCam::Tegra::AutoFocus {
public:

       FocusSweep(FCam::Tegra::Lens *l, FCam::Rect r = FCam::Rect());

       void startSweep();

       void update(const FCam::Frame &f);

       float** getDepthSamples();
       // State - ranges from face detection (2), focus sweep (1), and waiting (0)
       int state;
       //std::vector<point3d> samples;
       float** samples;

private:

       /* Private method */
       int computeImageContrast(FCam::Image &image, int rectIdx);

       void setRects();
       void updateRects();

       void logRectDump();
       void logDepthsDump();

       void drawRectangles(const FCam::Frame &frame);

       FCam::Tegra::Lens* lens;
       FCam::Rect rect;
       /* [CS478]
        * Declare any state variables you might need here.
        */

       //Regions where we're keeping track of the contrast
       std::vector<FCam::Rect> rects;
       //Stores the best contrast value and best focal length for a given reach - shares indexes with rects
       std::vector<FocusContrast> rectsFC;
       //interval count - counts from 0 to 22, where each index is associated with a focal length value
       int itvlCount;
       //for face detection phase - counts from 0 to 2. For each frame, store the regions where faces were detected
       int fdFrameCount;
       //float **samples;

};

#endif /* FOCUSSWEEP_H_ */
