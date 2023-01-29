#include <opencv2/opencv.hpp>
#include <common.h>

using namespace cv;

void analyze_shapes(vector<shape> &shapes, Mat image);

coord find_third_vertex(shape s, Mat image);

coord find_third_vertex_efficient(shape s, Mat image);

bool is_triangle(shape s, Mat image);

edge_type classify_edge(coord pt1, coord pt2, uint8_t code, Mat image);

void create_edges(shape &s);

shape::triangle_types classify_triangle_type(shape s);

float compute_triangle_area(shape s);


