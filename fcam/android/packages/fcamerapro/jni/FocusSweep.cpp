
#include "FocusSweep.h"
#include <android///LOG.h>
#include <new>

FocusSweep::FocusSweep(FCam::Tegra::Lens *l, FCam::Rect r){
	lens = l;
	rect = r;
	/* [CS478]
	 * Do any initialization you need.
	 */
	//LOG("DEPTH In sweep constructo1r\n");
	state = WAIT_PHASE;
	setRects();
}

void FocusSweep::setRects()//take focal length later?
{
	//LOG("DEPTH In set rects\n");
	int x_step_size = IMAGE_WIDTH / (NUM_RECTS_X + 1);
	int y_step_size = IMAGE_HEIGHT / (NUM_RECTS_Y + 1);

	for (int i = 1; i <= NUM_RECTS_X; i++)
	{
		for (int j = 1; j <= NUM_RECTS_Y; j++)
		{
			int x = x_step_size * i;
			int y = y_step_size * j;
			int rect_x = x - RECT_EDGE_LEN/2;
			int rect_y = y - RECT_EDGE_LEN/2;

			FCam::Rect r = FCam::Rect(rect_x, rect_y, RECT_EDGE_LEN, RECT_EDGE_LEN);
			rects.push_back(r);

			point3d p = point3d(x, y, 0.0f);
			samples.push_back(p);
		}
	}
}

void FocusSweep::updateRects(){};

/* Begin focus phase */
void FocusSweep::startSweep() {
	if (!lens) return;
	if (state != SWEEP_PHASE) return;

	//LOG("DEPTH In start sweep1\n");
	itvlCount = 0;
	lens->setFocus(discreteDioptres[itvlCount]);
	itvlCount++;
	//LOG("DEPTH In start sweep2\n");

	/*if (samples == NULL)
	{
		float* samples[NUM_RECTS_Y];
		//LOG("DEPTH In sweep constructor2\n");
		for (int i = 0; i<16; i++)
			samples[i] = new float[NUM_RECTS_X];
		//LOG("DEPTH In sweep constructor3\n");
	}*/
}

/* High Freq Pass filter - averages a region of pixels and takes the difference
 * between the center of the pixel and the average. Not efficiieeeeent!!
 */
int FocusSweep::computeImageContrast(FCam::Image &image, int rectIdx)
{
	////LOG("MYFOCUS compute contrast begin\n======================\n");

	unsigned int sum = 0;
	int totalValue = 0;
	int filterHalfSize = FILTER_SIZE / 2;
	int filterArea = FILTER_SIZE * FILTER_SIZE;
	int highX = rects[rectIdx].x + rects[rectIdx].width - filterHalfSize;
	int highY = rects[rectIdx].y + rects[rectIdx].height - filterHalfSize;
	for(int x = rects[rectIdx].x + filterHalfSize; x < highX; x++)
	{
		for(int y = rects[rectIdx].y + filterHalfSize; y < highY; y++)
		{
			sum = 0;
			for (int i = -filterHalfSize; i <= filterHalfSize; i++)
				for (int j = -filterHalfSize; j <= filterHalfSize; j++)
					sum += *image(x+i, y+j);

			sum /= filterArea;
			int temp = *image(x, y) - sum;

			totalValue += temp * temp;
		}
	}
	////LOG("MYFOCUS total value: %d\n", totalValue);
	////LOG("MYFOCUS compute contrast end\n======================\n");
	return totalValue;
}

/* [CS478]
 * This method is supposed to be called in order to inform
 * the autofocus engine of a new viewfinder frame.
 * You probably want to compute how sharp it is, and possibly
 * use the information to plan where to position the lens next.
 */
void FocusSweep::update(const FCam::Frame &f) {

	FCam::Image image = f.image();
	if (!image.valid()){
		////LOG("MYFOCUS Invalid image\n");
	}

	float expectedFocus = discreteDioptres[itvlCount - 1];
	////LOG("MYFOCUS The expected focus setting: %f\n", expectedFocus);
	float actualFocus = (float) f["lens.focus"];
	////LOG("MYFOCUS The average focus setting during the frame: %f\n", actualFocus);

	//If the lens focus request didn't go through, try again
	if ((actualFocus > expectedFocus + 0.003f) || (actualFocus < expectedFocus - 0.003f))
	{
		////LOG("MYFOCUS Trying lens focus request again\n");
		lens->setFocus(expectedFocus);
		drawRectangles(f);
		return;
	}

	for (int i = 0; i < rects.size(); i++)
	{
		int totalContrast = computeImageContrast(image, i);
		if (rectsFC[i].bestContrast < totalContrast)
		{
			rectsFC[i].bestContrast = totalContrast;
			rectsFC[i].bestFocus = itvlCount;
		}
		else if (itvlCount - 1 == 1)
		{
			//Catch outliers for the first checkpoint
			if((rectsFC[i].bestContrast * 0.6f) > totalContrast && rectsFC[i].bestFocus == 0)
			{
				rectsFC[i].bestContrast = -1;
				rectsFC[i].bestFocus = -1;
			}
		}
	}

	if (itvlCount != NUM_INTERVALS){
		lens->setFocus(discreteDioptres[itvlCount]);
		itvlCount++;
		drawRectangles(f);
		return;
	}

	state = SWEEP_FIN_PHASE;
	drawRectangles(f);
}

/* Must flip the state of focussweep to WAIT_PHASE after calling this function */
std::vector<point3d> FocusSweep::getDepthSamples()
{
	for (int i = 0; i < rects.size(); i++)
	{
		int y_ind = i % NUM_RECTS_Y;
		int x_ind = i / NUM_RECTS_Y;
		//samples[y_ind][x_ind] = 1.0f / discreteDioptres[rectsFC[i].bestFocus];
		samples[i].z = 1.0f / discreteDioptres[rectsFC[i].bestFocus];
		////LOG("MYFOCUS  x: %d\n", rect.x);
	}
	state = WAIT_PHASE;
	return samples;
}

void FocusSweep::drawRectangles(const FCam::Frame &frame)
{
	for (unsigned int i = 0; i < rects.size(); i++) {
		FCam::Rect r = rects[i];
		for (int x = 0; x < r.width; x++) {
			frame.image()(r.x + x, r.y)[0] = 254u;
			frame.image()(r.x + x, r.y + r.height)[0] = 254u;
		}
		for (int y = 0; y < r.height; y++) {
			frame.image()(r.x, r.y + y)[0] = 254u;
			frame.image()(r.x + r.width, r.y + y)[0] = 254u;
		}
	}
}

void FocusSweep::logRectDump()
{
	////LOG("MYFOCUS //LOG RECT DUMP BEGIN\n======================\n");
	////LOG("MYFOCUS rect x: %d\n", rect.x);
	////LOG("MYFOCUS rect y: %d\n", rect.y);
	////LOG("MYFOCUS rect width: %d\n", rect.width);
	////LOG("MYFOCUS rect height: %d\n", rect.height);
	////LOG("MYFOCUS LOG RECT DUMP END\n======================\n");
}

