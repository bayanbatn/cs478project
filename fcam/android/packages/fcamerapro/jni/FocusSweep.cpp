
#include "FocusSweep.h"
#include <android/LOG.h>
#include "AsyncImageWriter.h"

FocusSweep::FocusSweep(FCam::Tegra::Lens *l, FCam::Rect r){
	lens = l;
	rect = r;

	LOG("DEPTH FOCUSSWEEP \n");
	state = WAIT_PHASE;
	setRects();
	samples = new int*[NUM_RECTS_Y];
	samples2 = new int*[NUM_RECTS_Y];
	for (int y_ind = 0; y_ind < NUM_RECTS_Y; y_ind++)
	{
		samples[y_ind] = new int[NUM_RECTS_X];
		samples2[y_ind] = new int[NUM_RECTS_X];
	}

	is = NULL;

}

FocusSweep::~FocusSweep() {
	delete[] samples;
	delete[] samples2;
}

void FocusSweep::setRects()
{
	LOG("DEPTH RECTS In set rects\n");
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

			rectsFC.push_back(FocusContrast(-1, -1));
		}
	}
}

void FocusSweep::updateRects(){};

void FocusSweep::setImageSet(ImageSet* set) {
	is = set;
}
ImageSet* FocusSweep::getImageSet() {
	return is;
}

/* Begin focus phase */
void FocusSweep::startSweep() {
	if (!lens) return;
	if (state != SWEEP_PHASE) return;

	itvlCount = 0;
	lens->setFocus(discreteDioptres[itvlCount]);
	itvlCount++;
}

/* High Freq Pass filter - averages a region of pixels and takes the difference
 * between the center of the pixel and the average. Not efficiieeeeent!!
 */
int FocusSweep::computeImageContrast(FCam::Image &image, int rectIdx)
{
	LOG("DEPTH compute contrast begin\n======================\n");

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

			//sum /= filterArea;
			int temp = (*image(x, y) * filterArea) - sum;

			if (temp < 0)
				totalValue -= temp;
			else
				totalValue += temp;
		}
	}
	LOG("DEPTH total value: %d\n", totalValue);
	LOG("DEPTH compute contrast end\n======================\n");
	return totalValue;
}

/* High Freq Pass filter - averages a region of pixels and takes the difference
 * between the center of the pixel and the average. Not efficiieeeeent!!
 */
int FocusSweep::computeSobelContrast(FCam::Image &image, int rectIdx)
{
	LOG("DEPTH compute contrast begin\n======================\n");

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
			//for (int i = -filterHalfSize; i <= filterHalfSize; i++)
				//for (int j = -filterHalfSize; j <= filterHalfSize; j++)
					//sum += *image(x+i, y+j);
			int Gx = *image(x+1, y+1) + *image(x+1, y-1) + (*image(x+1, y) << 1) - *image(x-1, y+1) - *image(x-1, y-1) - (*image(x-1, y) << 1);
			int Gy = *image(x+1, y-1) + *image(x-1, y-1) + (*image(x, y-1) << 1) - *image(x-1, y+1) - *image(x+1, y-1) - (*image(x, y+1) << 1);

			//sum /= filterArea;
			int temp = Gx*Gx + Gy*Gy;

			totalValue += temp;
		}
	}
	LOG("DEPTH total value: %d\n", totalValue);
	LOG("DEPTH compute contrast end\n======================\n");
	return totalValue;
}

/* UPDATE
 */
bool FocusSweep::update(const FCam::Frame &f) {
	FCam::Image image = f.image();
	if (!image.valid()){
		LOG("DEPTH UPDATE Invalid image\n");
		return false;
	}

	float expectedFocus = discreteDioptres[itvlCount - 1];
	LOG("DEPTH UPDATE The expected focus setting: %f\n", expectedFocus);
	float actualFocus = (float) f["lens.focus"];
	LOG("DEPTH UPDATE The average focus setting during the frame: %f\n", actualFocus);

	//If the lens focus request didn't go through, try again
	if ((actualFocus > expectedFocus + 0.003f) || (actualFocus < expectedFocus - 0.003f))
	{
		LOG("DEPTH UPDATE Trying lens focus request again\n");
		lens->setFocus(expectedFocus);
		//drawRectangles(f);
		return false;
	}


	for (int i = 0; i < rects.size(); i++)
	{
		int totalContrast = computeSobelContrast(image, i);
		if (rectsFC[i].bestContrast < totalContrast && CONTRAST_THRESHOLD < totalContrast)
		{
			LOG("DEPTH UPDATE Store contrast\n");
			rectsFC[i].bestContrast = totalContrast;
			rectsFC[i].bestFocus = itvlCount - 1;
		}
		else if (itvlCount - 1 == 0)
		{
			LOG("DEPTH UPDATE Catch outliers for the first checkpoint\n");
			if((rectsFC[i].bestContrast * 0.6f) > totalContrast && rectsFC[i].bestFocus == 0)
			{
				if (CONTRAST_THRESHOLD < totalContrast){
					rectsFC[i].bestContrast = totalContrast; //-1
					rectsFC[i].bestFocus = itvlCount - 1; //-1
				} else {
					rectsFC[i].bestContrast = -1; //-1
					rectsFC[i].bestFocus = -1; //-1
				}
			}
		}
	}

	if (itvlCount != NUM_INTERVALS){
		LOG("DEPTH UPDATE update focal length, next itvlCount: %d, next focal dist: %d\n", itvlCount, discreteDioptres[itvlCount - 1]);
		lens->setFocus(discreteDioptres[itvlCount - 1]);
		itvlCount++;
		//drawRectangles(f);
		return true;
	}

	state = SWEEP_FIN_PHASE;

	logDepthsDump();

	//drawRectangles(f);

	//getDepthSamples();
	return true;
}

void FocusSweep::logDepthsDump()
{
	for (int i = 0; i < rectsFC.size(); i++)
	{
		int y_ind = i % NUM_RECTS_Y;
		int x_ind = i / NUM_RECTS_Y;
		int x_step_size = IMAGE_WIDTH / (NUM_RECTS_X + 1);
		int y_step_size = IMAGE_HEIGHT / (NUM_RECTS_Y + 1);

		int rect_y_center = y_step_size * (y_ind + 1);
		int rect_x_center = x_step_size * (x_ind + 1);
		float depth = 100.0f / discreteDioptres[rectsFC[i].bestFocus];
		LOG("DEPTH DEPTHS DUMP rect: %d, depth: %fcm, depth idx: %d\n", i, depth, rectsFC[i].bestFocus);
		LOG("DEPTH DEPTHS DUMP rect loc x: %d, loc y: %d\n", rect_x_center, rect_y_center);
	}
}

/* Must flip the state of focussweep to WAIT_PHASE after calling this function */
int** FocusSweep::getDepthSamples()
{
	LOG("DEPTH GET SAMPLES\n");
	for (int i = 0; i < rects.size(); i++)
	{
		int y_ind = i % NUM_RECTS_Y;
		int x_ind = i / NUM_RECTS_Y;
		samples[y_ind][x_ind] = rectsFC[i].bestFocus;
		//samples[i].z = 1.0f / discreteDioptres[rectsFC[i].bestFocus];
		LOG("DEPTH GET SAMPLES x_ind: %d, y_ind: %d, depth: %d\n", x_ind, y_ind, samples[y_ind][x_ind]);
	}
	state = WAIT_PHASE;
	return samples;
}


/* get the sharpness of each patch so that we could compute the confidence
 * for the smoothing
 */
int** FocusSweep::getSharpnessSamples()
{
	LOG("DEPTH GET SHARPNESS SAMPLES\n");
	for (int i = 0; i < rects.size(); i++)
	{
		int y_ind = i % NUM_RECTS_Y;
		int x_ind = i / NUM_RECTS_Y;
		//samples2[y_ind][x_ind] = rectsFC[i].bestFocus;//rectsFC[i].bestContrast;
		samples2[y_ind][x_ind] =rectsFC[i].bestContrast;
		LOG("DEPTH GET SHARPNESS SAMPLES x_ind: %d, y_ind: %d, depth: %d\n", x_ind, y_ind, samples2[y_ind][x_ind]);
	}
	state = WAIT_PHASE;
	return samples2;
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
}
