#include <opencv2/opencv.hpp>
#include <common.h>

using namespace cv;

coord find_third_vertex(shape s, Mat image);

coord find_third_vertex_efficient(shape s, Mat image);

enum shape::shape_type classify_shape(shape s, Mat image);


