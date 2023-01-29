#include <opencv2/opencv.hpp>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include <shape_analysis.h>

using namespace cv;
using namespace std;

//Inspired by http://users.utcluj.ro/~ancac/Resurse/PI/PI-L6e.pdf
//void border_tracing(){
  //Comment, I was going to implement this border tracing method and use it find edges and vertices. But then I found an easier method and decided to take advantage of that. :-)
//}


// Finds the vertex that is farthest from the line formed by first two vertices.
coord find_third_vertex(shape s, Mat image){
  assert (s.vertices.size() >=2);
  uint8_t* image_ptr = (uint8_t*)image.data;
  coord pt1 = s.vertices[0], pt2 = s.vertices[1];
  //Distance from line to pt (x,y) can be given as abs(a*x+b*y+c)/D
  //a=y1-y2, b=x2-x1, c=x1*y2-x2*y1
  int a = pt1.y-pt2.y, b=pt2.x-pt1.x, c=pt1.x*pt2.y-pt2.x*pt1.y;
  float d = pow(a*a+b*b,0.5);

  coord farthest_pt;
  float max_dist =-1;
  int iters =0;
  bool shape_over;
  for(int y = pt1.y; y < image.rows; y++){
    shape_over = true;
    for(int x = 0; x < image.cols; x++){
      if (image_ptr[y*image.cols + x] == s.code){ //check to see inside shape
        shape_over = false;
        float dist = abs(a*x+b*y+c)/d;
        //cout << " Dist:" <<  dist << endl;
        if (dist > max_dist){
          max_dist = dist;
          farthest_pt.x=x; farthest_pt.y=y;
        }
      }
    }
    if (shape_over) break; //no need to visit other pixels
  }
  return farthest_pt;
}

// Finds the vertex that is farthest from the line formed by first two vertices.
// I tried to write a more efficient algorithm, but this algorithm had the problem
// of initialization. For a triangle that had another triangle inside it this
// method can get stuck if the initializtion isn't right. Anyway, this is how it works:

// This algorithm is efficient and only follows a path. In each 3x3 neighborhood,
// it finds the vertex that is farthest from the line and inside the shape. It
// stops when such a point is the largest in its neighborhood.
coord find_third_vertex_efficient(shape s, Mat image){
  assert (s.vertices.size() >=2);
  uint8_t* image_ptr = (uint8_t*)image.data;
  coord pt1 = s.vertices[0], pt2 = s.vertices[1];
  //Distance from line to pt (x,y) can be given as abs(a*x+b*y+c)/D
  //a=y1-y2, b=x2-x1, c=x1*y2-x2*y1
  int a = pt1.y-pt2.y, b=pt2.x-pt1.x, c=pt1.x*pt2.y-pt2.x*pt1.y;
  float d = pow(a*a+b*b,0.5);

  coord c_pt=(pt1+pt2)*0.5; //current point
  coord p_pt; //previous point
  float max_dist =-1;
  int iters =0;
  cout << "Starting point: " << c_pt << " other point: " << pt2 << endl;
  do{
    p_pt = c_pt;
    int min_x = (c_pt.x-1)<0 ? 0 : c_pt.x-1, max_x = (c_pt.x+1)>image.cols? image.cols : c_pt.x+2;
    int min_y = (c_pt.y-1)<0 ? 0 : c_pt.y-1, max_y = (c_pt.y+1)>image.rows? image.rows : c_pt.y+2;
    for (int y=min_y; y<max_y;y++){
      for (int x=min_x; x<max_x;x++){
        //cout << "(" << x << "," << y << "): "<< (int)image_ptr[y*image.cols + x];
        if (image_ptr[y*image.cols + x] == s.code){ //check to see inside shape
          float dist = abs(a*x+b*y+c)/d;
          //cout << " Dist:" <<  dist << endl;
          if (dist > max_dist){
            max_dist = dist;
            c_pt.x=x; c_pt.y=y;
          }
        }
        //cout << endl;
      }
    }
    iters++;
    //cout << "-----Next point: " << c_pt << endl;
  } while (!(c_pt == p_pt));
  cout << c_pt << "after iters: " << iters<< endl;
  return c_pt;
}

// Classifies the shape by looking at the midpoint
// Assumes the 2nd vertex is the point in the shape that is farthest from 1st point.
//

shape::shape_type classify_shape(shape s, Mat image){
  assert(s.vertices.size() >=3); //otherwise not enough info
  
  return shape::line;
}



