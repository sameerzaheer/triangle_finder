#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include <shape_analysis.h>
#include <segmentation.h>

using namespace cv;
using namespace std;

void draw_shapes(Mat image, vector<shape> shapes){
  for (shape const &s : shapes){
    cout << "Shape: " << (int)s.code << " num_v:" << s.vertices.size() <<  endl;
    for (coord const &v : s.vertices){
      cout << v << endl;
      circle(image,Point(v.x,v.y), 5,Scalar(255), FILLED, LINE_8);
    }
    break;
  }
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat image;
    image = imread(argv[1], 1);
    cout << "read image " << endl;
    Mat grey_image(image.size(),CV_8U);
    greyscale_image(image, grey_image);
    cout << " grayed image " << endl;

    if (!image.data) {
        printf("No image data \n");
        return -1;
    }

    Mat mask(grey_image.size(),CV_8U);
    mask = 0;
    uint8_t val_after_thresh = 40;
    mask_threshold(grey_image, mask, 250, val_after_thresh);
    cout << " applied mask " << endl;
    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", mask);
    waitKey(0);
    cout << "next image" << endl;
    auto shapes = mask_segmentation(mask, val_after_thresh);
    cout << "Num of shapes found: " << shapes.size() << endl;
    for(shape &s : shapes){
      cout << "Shape: " << (int)s.code << endl;
      //cout << s.vertices.size() << endl;
      s.vertices.push_back(find_third_vertex(s,mask));
      //cout << s.vertices.size() << endl;
      break;
    }

    //Mat mask2 = mask(Rect(200,30,20,20));
    //cout << "mask2 cols " << mask2.cols << endl;
    //imwrite("test.bmp",mask2);

    draw_shapes(mask,shapes);
    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", mask);
    waitKey(0);

    return 0;
}

//Algorithm:
//1. inpainting segmentation
//2. boundary finding
//3. vertex finding:
//3a. centroid and then graph of radius
//3b. peaks in graph
//
