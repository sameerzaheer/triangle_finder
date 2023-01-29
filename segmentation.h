#include <opencv2/opencv.hpp>
#include <common.h>

using namespace cv;


void greyscale_image(Mat input, Mat &output);

void mask_threshold(Mat input, Mat output, uint8_t threshold, uint8_t mask_value);

coord inpaint(Mat input, coord c_start, uint8_t replace_val, uint8_t paint_val);

vector<shape> mask_segmentation(Mat input, uint8_t foreground);

