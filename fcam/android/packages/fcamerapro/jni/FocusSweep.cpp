
#include "FocusSweep.h"
#include <android/LOG.h>
#include "AsyncImageWriter.h"
#include <new>

FocusSweep::FocusSweep(FCam::Tegra::Lens *l, FCam::Rect r){
	lens = l;
	rect = r;

	LOG("DEPTH FOCUSSWEEP \n");
	state = WAIT_PHASE;
	setRects();
	samples = new float*[NUM_RECTS_Y];
	for (int y_ind = 0; y_ind < NUM_RECTS_Y; y_ind++)
	{
		samples[y_ind] = new float[NUM_RECTS_X];
	}

	is = NULL;

}

FocusSweep::~FocusSweep() {
	delete[] samples;
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

			sum /= filterArea;
			int temp = *image(x, y) - sum;

			totalValue += temp * temp;
		}
	}
	LOG("DEPTH total value: %d\n", totalValue);
	LOG("DEPTH compute contrast end\n======================\n");
	return totalValue;
}

/* UPDATE
 */
void FocusSweep::update(const FCam::Frame &f) {
	FCam::Image image = f.image();
	if (!image.valid()){
		LOG("DEPTH UPDATE Invalid image\n");
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
		drawRectangles(f);
		return;
	}

	// saves the current frame as image into the current ImageSet
	FileFormatDescriptor fmt(FileFormatDescriptor::EFormatJPEG, 95);
	is->add(fmt, f);

	for (int i = 0; i < rects.size(); i++)
	{
		int totalContrast = computeImageContrast(image, i);
		if (rectsFC[i].bestContrast < totalContrast)
		{
			LOG("DEPTH UPDATE Store contrast\n");
			rectsFC[i].bestContrast = totalContrast;
			rectsFC[i].bestFocus = itvlCount;
		}
		else if (itvlCount - 1 == 1)
		{
			LOG("DEPTH UPDATE Catch outliers for the first checkpoint\n");
			if((rectsFC[i].bestContrast * 0.6f) > totalContrast && rectsFC[i].bestFocus == 0)
			{
				rectsFC[i].bestContrast = totalContrast; //-1
				rectsFC[i].bestFocus = itvlCount; //-1
			}
		}
	}

	if (itvlCount != NUM_INTERVALS){
		LOG("DEPTH UPDATE update focal length, next itvlCount: %d, next focal dist: %d\n", itvlCount, discreteDioptres[itvlCount - 1]);
		lens->setFocus(discreteDioptres[itvlCount - 1]);
		itvlCount++;
		drawRectangles(f);
		return;
	}

	state = SWEEP_FIN_PHASE;

	//logDepthsDump();

	drawRectangles(f);

	//getDepthSamples();
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
		float depth = 1.0f / discreteDioptres[rectsFC[i].bestFocus];
		LOG("DEPTH DEPTHS DUMP rect: %d, depth: %fm, depth idx: %d\n", i, depth, rectsFC[i].bestFocus);
		LOG("DEPTH DEPTHS DUMP rect loc x: %d, loc y: %d\n", rect_x_center, rect_y_center);
	}
}

//JASON!!! USE THIS FUNCTION!!!
/* Must flip the state of focussweep to WAIT_PHASE after calling this function */
float** FocusSweep::getDepthSamples()
{
	LOG("DEPTH GET SAMPLES\n");
	for (int i = 0; i < rects.size(); i++)
	{
		int y_ind = i % NUM_RECTS_Y;
		int x_ind = i / NUM_RECTS_Y;
		samples[y_ind][x_ind] = 1.0f / discreteDioptres[rectsFC[i].bestFocus];
		//samples[i].z = 1.0f / discreteDioptres[rectsFC[i].bestFocus];
		LOG("DEPTH GET SAMPLES x_ind: %d, y_ind: %d, depth: %d\n", x_ind, y_ind, samples[y_ind][x_ind]);
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
}

