#include <opencv2/opencv.hpp>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <time.h>

#include <segmentation.h>

using namespace cv;
using namespace std;


void greyscale_image(Mat input, Mat &output){
  assert(input.rows == output.rows && input.cols == output.cols);
  uint8_t* image_ptr = (uint8_t*)input.data;
  uint8_t* grey_image_ptr = (uint8_t*)output.data;
  int c_num = input.channels();

  //Average RGB channels into greyscale
  for(int y = 0; y < input.rows; y++){
    for(int x = 0; x < input.cols; x++){
      grey_image_ptr[y*input.cols + x] =
        (image_ptr[y*input.cols*c_num + x*c_num + 0] +
         image_ptr[y*input.cols*c_num + x*c_num + 1] +
         image_ptr[y*input.cols*c_num + x*c_num + 2]) * 0.333;
         //Multiplication by 0.333 should be cheaper than divide by 3
    }
  }
}

void mask_threshold(Mat input, Mat mask, uint8_t threshold, uint8_t mask_val){
  assert(input.rows == mask.rows && input.cols == mask.cols);
  assert(input.channels() == 1);
  uint8_t* input_ptr = (uint8_t*)input.data;
  uint8_t* mask_ptr = (uint8_t*)mask.data;
  for(int y = 0; y < input.rows; y++){
    for(int x = 0; x < input.cols; x++){
      if (input_ptr[y*input.cols + x] < threshold){
        mask_ptr[y*input.cols + x] = mask_val;
      }
    }
  }
}

/*
  Returns the point inpaint that is farthest from c_start.
 */
coord inpaint(Mat input, coord c_start, uint8_t replace_val, uint8_t paint_val){
  assert(input.channels() == 1);
  assert(replace_val != paint_val);
  uint8_t* input_ptr = (uint8_t*)input.data;
  float farthest_dist =0;
  coord farthest_pt;
  queue<coord> q;
  q.push(c_start);
  input_ptr[c_start.y*input.cols + c_start.x] = paint_val;
  //struct timespec remaining, request = { 2, 1 };

  while (!q.empty()){
    coord c = q.front(); q.pop();

    //Do the if statement outside the for loop, so the same expression is not re-evaluted every iteration.
    int min_x = (c.x-1)<0 ? 0 : c.x-1, max_x = (c.x+1)>input.cols? input.cols : c.x+2;
    int min_y = (c.y-1)<0 ? 0 : c.y-1, max_y = (c.y+1)>input.rows? input.rows : c.y+2;
    //DB//cout << "---Popped: " << c.x << "," << c.y << " || Lims: " << min_x << " " << max_x << " | " << min_y << " " << max_y << " ---"<< endl;
    //DB//nanosleep(&request,&remaining);

    for (int y=min_y; y<max_y;y++){
      for (int x=min_x; x<max_x;x++){
        if (input_ptr[y*input.cols + x] == replace_val){
          input_ptr[y*input.cols + x] = paint_val;
          //cout << x << " " << y <<  endl;
          q.emplace(x,y);
          float dist_sq = c_start.dist_sq(x,y);
          if (dist_sq > farthest_dist){
            farthest_dist = dist_sq;
            farthest_pt.x = x; farthest_pt.y = y;
          }
        }
      }
    }
  } //end while
  return farthest_pt;
}
/*Segments an input mask by changing the labels of the pixels to:
 90: for background
 100: for the first object
 110: for the second object, and so on.

 If an object is filled with background, then it is filled in with same label as its edges.
 If an object has an object inside it, then the inside object has a separate label.
 Pixel (0,0) is assumed to be background.

 Returns the coord anchors of each shape (at the top left hand corner).
*/

vector<shape> mask_segmentation(Mat input, uint8_t foreground){
  assert(input.channels() == 1);
  vector<shape> shapes;
  //First segment the background
  coord bkgrnd_c = (coord){BACKGROUND_X,BACKGROUND_Y};
  uint8_t bkgrnd_val = input.at<uint8_t>(bkgrnd_c.y,bkgrnd_c.x);
  uint8_t mask_label = 90;

  inpaint(input,bkgrnd_c,bkgrnd_val, mask_label);
  mask_label+=10;
  //Then segment all the shapes
  uint8_t* input_ptr = (uint8_t*)input.data;
  for(int y = 0; y < input.rows; y++){
    for(int x = 0; x < input.cols; x++){
      if (input_ptr[y*input.cols + x] == foreground){
        coord vertex_2nd = inpaint(input,(coord){x,y}, foreground, mask_label);
        shapes.emplace_back((coord){x,y});
        shapes.back().add(vertex_2nd);
        shapes.back().code = mask_label;
        mask_label+=10;
      } else if (input_ptr[y*input.cols + x] == bkgrnd_val){
        //Check for adjacent non-background pixel.
        //No need to check boundaries as this pixel will not be at boundary.
        uint8_t nearest_mask_label=0;
        for(int yy = y-1; yy < y+2; y++){
          for(int xx = x-1; xx < x+2; x++){
            if (input_ptr[y*input.cols + x] != bkgrnd_val ){
              nearest_mask_label = input_ptr[y*input.cols + x];
              goto inpaint_bkgrnd;
            }
          }
        }

      inpaint_bkgrnd:
        if (nearest_mask_label != 0){
          inpaint(input,(coord){x,y}, bkgrnd_val, nearest_mask_label);
        }
      }
    }
  }
  cout << "done segmentation" << endl;
  return shapes; 
}


