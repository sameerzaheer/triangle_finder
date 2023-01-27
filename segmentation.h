#include <opencv2/opencv.hpp>
using namespace cv;


struct coord{
  int x;
  int y;
  coord(int x, int y):x(x),y(y){}
};

void greyscale_image(Mat input, Mat &output);

void mask_threshold(Mat input, Mat output, uint8_t threshold, uint8_t mask_value);

void inpaint(Mat input, coord c_start, uint8_t replace_val, uint8_t paint_val);

void mask_segmentation(Mat input, uint8_t foreground);

