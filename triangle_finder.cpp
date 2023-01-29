#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <algorithm>

#include <shape_analysis.h>
#include <segmentation.h>

using namespace cv;
using namespace std;

//Draw the shapes on the original image. Note that printing the edges makes the image messy.
void draw_shapes(Mat image, vector<shape> shapes, bool print_edges=false){
  for (shape const &s : shapes){
    //cout << "Shape: " << (int)s.code << " num_v:" << s.vertices.size() << " is triangle: " << s.is_triangle <<  endl;
    if (s.is_triangle){
      string label="t" + to_string(s.index) + " Area:" + to_string((int)s.area) +
        " Type:" + shape::triangle_type_tostring(s.triangle_type);
      putText(image, label, Point(max(min(s.average_x - 100,(int)(image.cols*0.6)),1),
                                  s.vertices[s.lowest_vertex_index].y + 30),
              FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0,0,100),2);
      cout << label << endl;
      //for (coord const &v : s.vertices){
      //  //cout << v << endl;
      //  circle(image,Point(v.x,v.y), 5,Scalar(0,50,0), FILLED, LINE_8);
      //}
      for (edge const &e : s.edges){
        //cout << v << endl;
        string edge_label="e" + to_string(e.index) + " Length:" + to_string((int)e.length) +
          " of t" + to_string(s.index);
        if (print_edges){
          putText(image, edge_label, Point(e.midpoint.x - 40,e.midpoint.y - 10),
                  FONT_HERSHEY_SIMPLEX, 0.4, Scalar(120,0,0));
        }
        cout << edge_label << endl;
      }
      cout << endl;
    }
    //Debug
    //for (coord const &v : dbg_vertices){
    //  //cout << v << endl;
    //  circle(image,Point(v.x,v.y), 5,Scalar(240), 1, LINE_8);
    //}
  }
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat original_image;
    original_image = imread(argv[1], 1);
    if (!original_image.data) {
        printf("No image data \n");
        return -1;
    }

    cout << "Read image " << endl;
    Mat grey_image(original_image.size(),CV_8U);
    greyscale_image(original_image, grey_image);
    cout << "Grayed image " << endl;

    Mat mask(grey_image.size(),CV_8U);
    mask = 0;
    uint8_t val_after_thresh = 40;

    //STEP A: Create a binary image.
    mask_threshold(grey_image, mask, 250, val_after_thresh);
    cout << "Applied mask " << endl;

    //STEP B: Segment mask into different shapes.
    auto shapes = mask_segmentation(mask, val_after_thresh);
    cout << "Mask segmented. Num of shapes found: " << shapes.size() << endl;

    //STEPS C, D and E:
    //C: Find the 3rd vertex.
    //D: Determine if shape is triangle.
    //E: Compute analytics.
    analyze_shapes(shapes, mask);

    //Create output image for this assignment.
    draw_shapes(original_image,shapes, true);
    string in_file(argv[1]);
    string out_file("output_" + in_file.substr(in_file.find_last_of("/\\") + 1));
    cout << "Out file: " << out_file << endl;
    imwrite(out_file, original_image);

    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", original_image);
    waitKey(0);

    return 0;
}
