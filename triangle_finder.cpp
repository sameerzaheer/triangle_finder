#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include <segmentation.h>

using namespace cv;
using namespace std;


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
    mask_threshold(grey_image, mask, 210, val_after_thresh);
    cout << " applied mask " << endl;

    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", mask);
    waitKey(0);
    cout << "next image" << endl;
    mask_segmentation(mask, val_after_thresh);
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
