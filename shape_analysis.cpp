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


//Given the segmented mask, and a list of shapes, this function
//analyzes finds its vertices, whether it is a triangle, if so what
//type, what is its area and creates its edges.
void analyze_shapes(vector<shape> &shapes, Mat image){
  int triangle_num = 0;
  for(shape &s : shapes){
//    cout << "Shape: " << (int)s.code << endl;
    s.vertices.push_back(find_third_vertex(s,image));
    s.is_triangle = is_triangle(s,image);
    if (!s.is_triangle) continue;
    s.index = ++triangle_num;
    s.lowest_vertex_index = (s.vertices[2].y > s.vertices[1].y) ?
      (s.vertices[2].y > s.vertices[0].y ? 2 : 0):
      (s.vertices[1].y > s.vertices[0].y ? 1 : 0);
    s.average_x = (int)((s.vertices[0].x +
                         s.vertices[1].x +
                         s.vertices[2].x)*0.33);
    create_edges(s);
    s.triangle_type = classify_triangle_type(s);
    s.area = compute_triangle_area(s);
  }
}


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

// Determines if shape is a triangle by ensuring that every edge is filled
// on one and only one side.

bool is_triangle(shape s, Mat image){
  assert(s.vertices.size() >=3); //otherwise not enough info
  if (classify_edge(s.vertices[0], s.vertices[1], s.code, image) == one_filled &&
      classify_edge(s.vertices[1], s.vertices[2], s.code, image) == one_filled &&
      classify_edge(s.vertices[0], s.vertices[2], s.code, image) == one_filled){
    return true;
  } else {
    return false;
  }
}

vector<coord> dbg_vertices;

// Given two points, determines whether the line formed by them is contains the shape
// on no side (enum line), one side (enum triangle), or both sides (enum more_than_three).
// This is done by checking the pixels on both sides of the edge. See Explanation #5 in doc.
edge_type classify_edge(coord pt1, coord pt2, uint8_t code, Mat image){
  coord mid_pt = (pt1+pt2)*0.5;
  float dir_x = pt2.y-pt1.y;
  float dir_y = pt1.x-pt2.x;
  float magnitude = MAXIMUM_WIDTH_OF_LINE / pow(dir_x*dir_x + dir_y*dir_y,0.5);
  coord offset = (coord){(int)round(dir_x * magnitude),
                         (int)round(dir_y * magnitude)};
  coord right_pt = mid_pt + offset;
  coord left_pt  = mid_pt - offset;
  //cout << "Code: " << (int)code << " Right pt: " << right_pt << " Left pt: " << left_pt << endl;
  //cout << (int)image.at<uchar>(right_pt.y,right_pt.x) << " " << (int)image.at<uchar>(left_pt.y,left_pt.x) << endl;
  //dbg_vertices.push_back(right_pt);
  //dbg_vertices.push_back(left_pt);

  if (image.at<uchar>(right_pt.y,right_pt.x) == code &&
      image.at<uchar>(left_pt.y, left_pt.x)  == code){
    return both_filled;
  } else if (image.at<uchar>(right_pt.y,right_pt.x) != code &&
             image.at<uchar>(left_pt.y, left_pt.x)  != code){
    return none_filled;
  } else {
    return one_filled;
  }
}

void create_edges(shape &s){
  assert(s.vertices.size() == 3);
  s.edges.emplace_back(0,&(s.vertices[0]),&(s.vertices[1]),
                       (s.vertices[0]+s.vertices[1])*0.5, s.vertices[0].dist(s.vertices[1]));
  s.edges.emplace_back(1,&(s.vertices[1]),&(s.vertices[2]),
                       (s.vertices[1]+s.vertices[2])*0.5, s.vertices[1].dist(s.vertices[2]));
  s.edges.emplace_back(2,&(s.vertices[0]),&(s.vertices[2]),
                       (s.vertices[0]+s.vertices[2])*0.5, s.vertices[0].dist(s.vertices[2]));
}

//Determine if the triangle is equilateral, isosceles or scalene
shape::triangle_types classify_triangle_type(shape s){
  assert(s.edges.size() == 3);
  if      (abs(s.edges[0].length - s.edges[1].length) <= MARGIN_OF_ERROR &&
           abs(s.edges[1].length - s.edges[2].length) <= MARGIN_OF_ERROR &&
           abs(s.edges[0].length - s.edges[2].length) <= MARGIN_OF_ERROR){
    return shape::equilateral;
  }else if(abs(s.edges[0].length - s.edges[1].length) <= MARGIN_OF_ERROR ||
           abs(s.edges[1].length - s.edges[2].length) <= MARGIN_OF_ERROR ||
           abs(s.edges[0].length - s.edges[2].length) <= MARGIN_OF_ERROR){
    return shape::isosceles;
  }else{
    return shape::scalene;
  }
}

//Calculate area of each triangle, in pixels, by computing the cross product
//of two of its edges, and taking half its magnitude.
float compute_triangle_area(shape s){
  //int cross_x = 0;
  //int cross_y = 0;
  int cross_z = (s.vertices[1].x-s.vertices[0].x)*(s.vertices[2].y-s.vertices[0].y) -
                (s.vertices[1].y-s.vertices[0].y)*(s.vertices[2].x-s.vertices[0].x);
  //Magnitude is same as length, because x,y components are 0
  return abs(cross_z)*0.5;
}


