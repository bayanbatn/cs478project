#ifndef MYFACEDETECTOR_H_
#define MYFACEDETECTOR_H_

#include <vector>

#include <FCam/Base.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#define IMAGE_WIDTH 			640
#define IMAGE_HEIGHT 			480

class MyFaceDetector {
public:
	MyFaceDetector(const char* filename) : classifier(filename) {
		// Empty constructor. Nothing to do.
	}

	std::vector<cv::Rect> detectFace(const FCam::Image& im) {
		/* [CS478]
		 * You will need to call the proper method belonging to
		 * classifier. See the OpenCV documentation.
		 * http://opencv.willowgarage.com/documentation/cpp/objdetect_cascade_classification.html
		 */
		std::vector<cv::Rect> ret;
		cv::Mat imageMat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1, im(0,0));
		classifier.detectMultiScale(imageMat, ret);
		//cv::Rect rect(300, 300, 50, 50);
		//ret.push_back(rect);
		// TODO TODO TODO
		// TODO TODO TODO
		// TODO TODO TODO
		// TODO TODO TODO
		// TODO TODO TODO
		return ret;
	}
private:
	cv::CascadeClassifier classifier;
};



#endif /* MYFACEDETECTOR_H_ */
