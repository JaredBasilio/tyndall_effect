#include "triangle.h"

#include "CGL/CGL.h"
#include "GL/glew.h"
#define msg(s) cerr << "[PathTracer] " << s << endl;

namespace CGL {
namespace SceneObjects {

Triangle::Triangle(const Mesh *mesh, size_t v1, size_t v2, size_t v3) {
  p1 = mesh->positions[v1];
  p2 = mesh->positions[v2];
  p3 = mesh->positions[v3];
  n1 = mesh->normals[v1];
  n2 = mesh->normals[v2];
  n3 = mesh->normals[v3];
  bbox = BBox(p1);
  bbox.expand(p2);
  bbox.expand(p3);

  bsdf = mesh->get_bsdf();
}

BBox Triangle::get_bbox() const { return bbox; }

bool Triangle::has_intersection(const Ray &r) const {
  // Part 1, Task 3: implement ray-triangle intersection
  // The difference between this function and the next function is that the next
  // function records the "intersection" while this function only tests whether
  // there is a intersection.
    // Calculate the plane normal
    cout << "hello";
    Vector3D v = p2 - p1;
    Vector3D w = p3 - p1;
    
    Vector3D N = cross(v, w);
    
    float divisor = dot(r.d, N);
    
    if (divisor == 0) {
        return false;
    }
    
    float t = dot((p3 - r.o), N) / divisor;
    
    if (t < 0 || t < r.min_t || t > r.max_t) {
        return false;
    }
    
    Vector3D hit_point = r.o + r.d * t;
    
    //Barycentric coordinates: ceng2.ktu.edu.tr/~cakir/files/grafikler/Texture_Mapping.pdf
    
    Vector3D v0 = p2 - p1;
    Vector3D v1 = p3 - p1;
    Vector3D v2 = hit_point - p1;
    
    float alpha, beta, gamma;
    
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    
    float denom = d00 * d11 - d01 * d01;
    alpha = (d11 * d20 - d01 * d21) / denom;
    beta = (d00 * d21 - d01 * d20) / denom;
    gamma = 1.0 - alpha - beta;
    
    auto e1 = p2 - p1, e2 = p3 - p1, s = r.o - p1;
      auto s1 = cross(r.d, e2), s2 = cross(s, e1);
      auto result = 1 / dot(s1, e1) * Vector3D(dot(s2, e2), dot(s1, s), dot(s2, r.d));
      auto b1 = result[1], b2 = result[2], b3 = 1.0 - b1 - b2;
    
    cout << alpha << beta << gamma << endl;
    cout << b1 << b2 << b3 << endl;
    
    if (alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1) {
        return true;
    }
    
    return false;

}

bool Triangle::intersect(const Ray &r, Intersection *isect) const {
  // Part 1, Task 3:
  // implement ray-triangle intersection. When an intersection takes
  // place, the Intersection data should be updated accordingly
    
    // Calculate the plane normal

    Vector3D v = p2 - p1;
    Vector3D w = p3 - p1;
    
    Vector3D N = cross(v, w);
    
    float divisor = dot(r.d, N);
    
    if (divisor == 0) {
        return false;
    }
    
    float t = dot((p3 - r.o), N) / divisor;
    
    if (t < 0 || t < r.min_t || t > r.max_t) {
        return false;
    }
    
    Vector3D hit_point = r.o + r.d * t;
    
    //Formula for Barycentric coordinates: ceng2.ktu.edu.tr/~cakir/files/grafikler/Texture_Mapping.pdf
    
    Vector3D v0 = p2 - p1;
    Vector3D v1 = p3 - p1;
    Vector3D v2 = hit_point - p1;
    float alpha, beta, gamma;
    
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    
    float denom = d00 * d11 - d01 * d01;
    alpha = (d11 * d20 - d01 * d21) / denom;
    beta = (d00 * d21 - d01 * d20) / denom;
    gamma = 1.0 - alpha - beta;
    
    if (alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1) {
        r.max_t = t;
        isect->t = t;
        isect->n = alpha * n2 + beta * n3 + gamma * n1;
        isect->primitive = this;
        isect->bsdf = get_bsdf();
        return true;
    }
    
    return false;
}

void Triangle::draw(const Color &c, float alpha) const {
  glColor4f(c.r, c.g, c.b, alpha);
  glBegin(GL_TRIANGLES);
  glVertex3d(p1.x, p1.y, p1.z);
  glVertex3d(p2.x, p2.y, p2.z);
  glVertex3d(p3.x, p3.y, p3.z);
  glEnd();
}

void Triangle::drawOutline(const Color &c, float alpha) const {
  glColor4f(c.r, c.g, c.b, alpha);
  glBegin(GL_LINE_LOOP);
  glVertex3d(p1.x, p1.y, p1.z);
  glVertex3d(p2.x, p2.y, p2.z);
  glVertex3d(p3.x, p3.y, p3.z);
  glEnd();
}

} // namespace SceneObjects
} // namespace CGL
