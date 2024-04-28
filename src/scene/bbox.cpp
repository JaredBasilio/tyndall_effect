#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>

namespace CGL {

bool BBox::intersect(const Ray &r, double &t0, double &t1) const {

  // TODO (Part 2.2):
  // Implement ray - bounding box intersection test
  // If the ray intersected the bouding box within the range given by
  // t0, t1, update t0 and t1 with the new intersection times.

  auto direction = r.d;
  auto origin = r.o;
  // xslabs
  auto point1 = this->min, point2 = this->max;
  auto X = Vector3D(1, 0, 0), Y = Vector3D(0, 1, 0), Z = Vector3D(0, 0, 1);
  double tmin = -std::numeric_limits<double>::infinity(),
         tmax = std::numeric_limits<double>::infinity();
  double xmin = -std::numeric_limits<double>::infinity(), 
       xmax = std::numeric_limits<double>::infinity(), 
       ymin = -std::numeric_limits<double>::infinity(), 
       ymax = std::numeric_limits<double>::infinity(), 
       zmin = -std::numeric_limits<double>::infinity(), 
       zmax = std::numeric_limits<double>::infinity();
  if (dot(direction, X) != 0) {
    auto x1 = ((this->min.x - origin.x)) / (direction.x);
    auto x2 = ((this->max.x - origin.x)) / (direction.x);
    xmin = std::min(x1, x2);
    xmax = std::max(x1, x2);
  }
  if (dot(direction, Y) != 0) {
    auto y1 = ((this->min.y - origin.y)) / (direction.y);
    auto y2 = ((this->max.y - origin.y)) / (direction.y);
    ymin = (std::min(y1, y2));
    ymax = (std::max(y1, y2));
  }
  if (dot(direction, Z) != 0) {
    auto z1 = ((this->min.z - origin.z)) / (direction.z);
    auto z2 = ((this->max.z - origin.z)) / (direction.z);
    zmin = std::min(z1, z2);
    zmax = std::max(z1, z2);
  }
  tmin = std::max(zmin, std::max(xmin, ymin));
  tmax = std::min(zmax, std::min(xmax, ymax));
if (tmin <= tmax && tmin < t1) {
  t0 = tmin;
  t1 = tmax;
  return true;
} return false;
}

void BBox::draw(Color c, float alpha) const {

  glColor4f(c.r, c.g, c.b, alpha);

  // top
  glBegin(GL_LINE_STRIP);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(max.x, max.y, max.z);
  glEnd();

  // bottom
  glBegin(GL_LINE_STRIP);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, min.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glEnd();

  // side
  glBegin(GL_LINES);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(min.x, min.y, max.z);
  glEnd();
}

std::ostream &operator<<(std::ostream &os, const BBox &b) {
  return os << "BBOX(" << b.min << ", " << b.max << ")";
}

} // namespace CGL
