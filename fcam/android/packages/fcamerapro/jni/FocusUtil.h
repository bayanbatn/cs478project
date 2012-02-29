
#ifndef FOCUSUTIL_H_
#define FOCUSUTIL_H_

//Number of discrete focal lengths we sweep through
#define NUM_INTERVALS 			10
#define IMAGE_WIDTH 			640
#define IMAGE_HEIGHT 			480

typedef unsigned char uchar;

/* Discrete focal length values that we sweep through */
/*static const float discreteDioptres[] = {0.2f, 0.25f, 0.33f, 0.5f, 1.0f, 1.11f, 1.25f, 1.42f, 1.66f, 2.0f,
										 2.5f, 3.33f, 5.0f, 5.26f, 5.56f, 5.88f, 6.25f, 6.67f, 7.14f, 7.69f,
										 8.33f, 9.09f, 10.0f};*/

static const float discreteDioptres[] = {1.0f, 1.25f, 1.66f, 2.5f, 5.0f, 5.56f, 6.25f, 7.14f, 8.33f, 10.0f};

/* Struct for storing the best focus value
 * and maximal contrast for a given region (rect)
 * in the image.
 */
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

#endif /* FOCUSUTIL_H_ */
