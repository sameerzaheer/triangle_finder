#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//Consts

//This is how wide a line segment is presumed to be.
//Anything wider than 6 pixels is assumed to be a polygon.
const int MAXIMUM_WIDTH_OF_LINE = 6;
const int MARGIN_OF_ERROR = 3;

//The point that is guaranteed to represent background
const int BACKGROUND_X = 0;
const int BACKGROUND_Y = 0;


struct coord{
  int x;
  int y;
  coord(int x, int y):x(x),y(y){
  }
  coord(const coord& c):x(c.x),y(c.y){
  }
  coord():x(-1),y(-1){
  } //default
  bool operator==(const coord &rhs){
    return (this->x == rhs.x) && (this->y && rhs.y);
  }
  coord operator+(const coord &rhs){
    return (coord){this->x + rhs.x, this->y + rhs.y};
  }
  coord operator-(const coord &rhs){
    return (coord){this->x - rhs.x, this->y - rhs.y};
  }
  coord operator*(double k){
    return (coord){(int)(this->x*k),(int)(this->y*k)};
  }
  float dist(coord c){
    return pow(pow(x-c.x,2) + pow(y-c.y,2),0.5);
  }
  //In most cases we care about relative distance, not absolute,
  //so can save a sqrt computation
  float dist_sq(int x_, int y_){
    return pow(x-x_,2) + pow(y-y_,2);
  }
};

inline ostream& operator<<(ostream& os, const coord& c) {
  return os << "(" << c.x << "," << c.y << ")";
}

struct edge{
  coord midpoint;
  float length;
  coord *pt1; //reference into shape's vertices
  coord *pt2;
  int index;
edge(int index, coord *pt1, coord *pt2,coord midpoint, float length):index(index),pt1(pt1),pt2(pt2),midpoint(midpoint),length(length){}
};

struct shape{
  int index;
  vector<coord> vertices;
  vector<edge> edges;
  float area;
  uint8_t code; //the pixel value assigned during segmentation
  shape(coord c){
    vertices.push_back(c);
  }
  void add(coord c){
    vertices.push_back(c);
  }
  bool is_triangle = false;
  enum triangle_types{equilateral, isosceles, scalene};
  triangle_types triangle_type;
  static const char* triangle_type_tostring(triangle_types t){
    switch (t){
    case equilateral: return "equilateral";
    case isosceles:   return "isosceles";
    case scalene:     return "scalene";
    }
  }
  //Variables helpful for printing
  int lowest_vertex_index;
  int average_x;
};

enum edge_type{both_filled, one_filled, none_filled};

extern vector<coord> dbg_vertices;

#endif
