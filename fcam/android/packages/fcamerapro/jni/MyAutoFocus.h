#ifndef _MYAUTOFOCUS_H
#define _MYAUTOFOCUS_H

#include <FCam/Tegra/AutoFocus.h>
#include <FCam/Tegra/Lens.h>
#include <FCam/Base.h>
#include <android/log.h>
#include <vector>
#include <opencv2/core/core.hpp>

#define NUM_INTERVALS 			23
#define RECT_EDGE_LEN 			100
#define IMAGE_WIDTH 			640
#define IMAGE_HEIGHT 			480
#define FILTER_SIZE 			5
#define NUM_NIGHT_IMAGES 		2
//Face Detect
#define FD_MAX_FRAMES			3
//Focus states
#define AUTO_FOCUS_FACE_DETECT 	2
#define AUTO_FOCUS_FOCUS 		1
#define AUTO_FOCUS_WAIT 		0
//Max number of areas to focus

typedef unsigned char uchar;


static const float discreteDioptres[] = {0.2f, 0.25f, 0.33f, 0.5f, 1.0f, 1.11f, 1.25f, 1.42f, 1.66f, 2.0f,
										 2.5f, 3.33f, 5.0f, 5.26f, 5.56f, 5.88f, 6.25f, 6.67f, 7.14f, 7.69f,
										 8.33f, 9.09f, 10.0f};

struct FocusContrast
{
	int bestContrast;
	int bestFocus;

	FocusContrast(int c, int f)
	{
		bestContrast = -1;
		bestFocus = -1;
	}
};

class MyAutoFocus : public FCam::Tegra::AutoFocus {
public:

       MyAutoFocus(FCam::Tegra::Lens *l, FCam::Rect r = FCam::Rect()) : FCam::Tegra::AutoFocus(l,r) {
    	   lens = l;
    	   rect = r;
    	   /* [CS478]
    	    * Do any initialization you need.
    	    */
    	   for (int i = 0; i < NUM_INTERVALS; i++)
    		   sharpVals[i] = 0.0f;
    	   state = AUTO_FOCUS_WAIT;
    	   fdFrameCount = 0;
       }

       void fdWait()
       {
    	   if (fdFrameCount == 0)
    		   lens->setFocus(discreteDioptres[4]); //hardcoded values for now
    	   else if (fdFrameCount == 1)
    		   lens->setFocus(discreteDioptres[11]);
    	   else if (fdFrameCount == 2)
    		   lens->setFocus(discreteDioptres[18]);

    	   if (fdFrameCount == FD_MAX_FRAMES)
    	   {
    		   fdFrameCount = 0;
    		   if (rects.size() == 0)
    			   state = AUTO_FOCUS_WAIT; //no faces detected
    		   else
    		   {
    			   state = AUTO_FOCUS_FOCUS;
    			   startSweep();
    		   }
    	   }
    	   fdFrameCount++;
       }

       /* Meet the world's most inefficient implementation of median calculation.
        */
       int findMedianIdx()
       {
    	   int medianPos = (rects.size() + 1) / 2;
		   int maxFocus = 0;

    	   for (int i = 0; i < medianPos; i++)
    	   {
    		   maxFocus = 0;
    		   int maxIdx = 0;
    		   for (int j = 0; j < rects.size(); j++)
    		   {
    			   if (maxFocus <= rectsFC[j].bestFocus)
    			   {
    				   maxFocus = rectsFC[j].bestFocus;
    				   maxIdx = j;
    			   }
    		   }
    		   rectsFC[maxIdx].bestFocus = -1;
    	   }
    	   LOG("MYFOCUS median idx: %d\n", maxFocus);
    	   return maxFocus; //error
       }

       /* Sets focus region. For global, it's the entire frame */
       void setRect(int x, int y, int width = RECT_EDGE_LEN, int height = RECT_EDGE_LEN)
       {
		   FCam::Rect rect = FCam::Rect(std::max(x, 0), std::max(y, 0), width, height);
		   rects.push_back(rect);
		   FocusContrast fc(-1, -1);
		   rectsFC.push_back(fc);
       }

       bool trialsContains(cv::Rect& rect)
       {
    	   for (int i = 0; i < trials.size(); i++)
    	   {
    		   if(rect.x > trials[i].x - 25 && rect.x < trials[i].x + 25 && rect.y > trials[i].y - 25 && rect.y < trials[i].y + 25)
    		   {
    			   rects.push_back(trials[i]);
    			   trials.erase(trials.begin() + i);
    			   FocusContrast fc(-1, -1);
    			   rectsFC.push_back(fc);
    			   return true;
    		   }
    	   }
    	   return false;
       }

       bool rectsContains(cv::Rect& rect)
       {
    	   for (int i = 0; i < rects.size(); i++)
    	   {
    		   if(rect.x > rects[i].x - 25 && rect.x < rects[i].x + 25 && rect.y > rects[i].y - 25 && rect.y < rects[i].y + 25)
    		   {
    			   return true;
    		   }
    	   }
    	   return false;
       }



       void setRects(std::vector<cv::Rect>& cv_rects)//TODO
       {
    	   for (int i = 0; i < cv_rects.size(); i++)
    	   {
    		   if (trialsContains(cv_rects[i]))
    			   continue;
    		   else if(rectsContains(cv_rects[i]))
    			   continue;
    		   else
    		   {
    			   FCam::Rect rect = FCam::Rect(cv_rects[i].x, cv_rects[i].y, cv_rects[i].width, cv_rects[i].height);
    			   trials.push_back(rect);
    		   }
    	   }
       }

       /* Beging focusing */
       void startSweep() {
    	   if (!lens) return;
    	   /* [CS478]
    	    * This method should initiate your autofocus algorithm.
    	    * Before you do that, do basic checks, e.g. is the autofocus
    	    * already engaged?
    	    */
    	   if (state != AUTO_FOCUS_FOCUS) return;
    	   bestFocalDist = -1.0f;

    	   itvlCount = 0;
    	   lens->setFocus(discreteDioptres[itvlCount]);
    	   itvlCount++;

    	   logDump();
       }

       /* High Freq Pass filter - averages a region of pixels and takes the difference
        * between the center of the pixel and the average
        */
       int computeImageContrast(FCam::Image &image, int rectIdx)
       {
    	   LOG("MYFOCUS compute contrast begin\n======================\n");

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
    	   LOG("MYFOCUS total value: %d\n", totalValue);
    	   LOG("MYFOCUS compute contrast end\n======================\n");
    	   return totalValue;
       }


       void update(const FCam::Frame &f) {
    	   /* [CS478]
    	    * This method is supposed to be called in order to inform
    	    * the autofocus engine of a new viewfinder frame.
    	    * You probably want to compute how sharp it is, and possibly
    	    * use the information to plan where to position the lens next.
    	    */
    	   /* Extract frame and do stuff */

    	   FCam::Image image = f.image();
    	   if (!image.valid()){
    		   sharpVals[itvlCount-1] = -1;
    		   LOG("MYFOCUS Invalid image\n");
    	   }

    	   float expectedFocus = discreteDioptres[itvlCount - 1];
    	   LOG("MYFOCUS The expected focus setting: %f\n", expectedFocus);
    	   float actualFocus = (float) f["lens.focus"];
    	   LOG("MYFOCUS The average focus setting during the frame: %f\n", actualFocus);

    	   //If the lens focus request didn't go through, try again
    	   if ((actualFocus > expectedFocus + 0.003f) || (actualFocus < expectedFocus - 0.003f))
    	   {
    		   LOG("MYFOCUS Trying lens focus request again\n");
    		   lens->setFocus(expectedFocus);
    		   drawRectangles(f);
    		   return;
    	   }

    	   logDump();

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
    	   int medianFocus = findMedianIdx();
    	   logDump();

		   bestFocalDist = discreteDioptres[medianFocus];
		   LOG("MYFOCUS The best focus setting: %f\n", bestFocalDist);
		   state = AUTO_FOCUS_WAIT;
		   lens->setFocus(bestFocalDist);
		   drawRectangles(f);

    	   //Rect cleans after drawing
    	   rects.clear();
    	   trials.clear();
    	   rectsFC.clear();
       }

       void drawRectangles(const FCam::Frame &frame)
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

       void logDump()
       {
    	   LOG("MYFOCUS LOG DUMP BEGIN\n======================\n");
    	   LOG("MYFOCUS interval count: %d\n", itvlCount);
    	   LOG("MYFOCUS Focusing?: %d\n", state);
    	   LOG("MYFOCUS LOG DUMP END\n======================\n");
       }

       void logArrayDump()
       {
    	   LOG("MYFOCUS LOG ARRAY DUMP BEGIN\n======================\n");
    	   for (int i = 0; i < NUM_INTERVALS; i++)
    	   {
    		   LOG("MYFOCUS array index: %d, val: %d\n", i, sharpVals[i]);
    	   }
    	   LOG("MYFOCUS LOG ARRAY DUMP END\n======================\n");
       }

       void logRectDump()
       {
    	   LOG("MYFOCUS LOG RECT DUMP BEGIN\n======================\n");
    	   LOG("MYFOCUS rect x: %d\n", rect.x);
    	   LOG("MYFOCUS rect y: %d\n", rect.y);
    	   LOG("MYFOCUS rect width: %d\n", rect.width);
    	   LOG("MYFOCUS rect height: %d\n", rect.height);
    	   LOG("MYFOCUS LOG RECT DUMP END\n======================\n");
       }

       int state;

private:
       FCam::Tegra::Lens* lens;
       FCam::Rect rect;
       /* [CS478]
        * Declare any state variables you might need here.
        */
       std::vector<FCam::Rect> rects;
       std::vector<FCam::Rect> trials;
       std::vector<FocusContrast> rectsFC; //Stores the best contrast of the index up to date
       int itvlCount;
       int fdFrameCount;
       int sharpVals[NUM_INTERVALS];
       float nearFocus;
       float farFocus;
       float bestFocalDist;
};

#endif
