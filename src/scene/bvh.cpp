#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>
#include <random>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->draw(c, alpha);
    }
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->drawOutline(c, alpha);
    }
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}

BVHNode *BVHAccel::construct_bvh(std::vector<Primitive *>::iterator start,
                                 std::vector<Primitive *>::iterator end,
                                 size_t max_leaf_size) {

  // TODO (Part 2.1):
  // Construct a BVH from the given vector of primitives and maximum leaf
  // size configuration. The starter code build a BVH aggregate with a
  // single leaf node (which is also the root) that encloses all the
  // primitives.
    
    BBox bbox;

    for (auto p = start; p != end; p++) {
      BBox bb = (*p)->get_bbox();
      bbox.expand(bb);
    }
    
    if (distance(start, end) <= max_leaf_size) {

        BVHNode *node = new BVHNode(bbox);
        node->start = start;
        node->end = end;

        return node;
    } else {
        // Step 2: Split the tree partition function (en.cppreference.com/w/cpp/algorithm/partition)
        
        Vector3D extent = bbox.extent;
        
        std::vector<Primitive *>::iterator middle;
                
        if (extent.x >= extent.y && extent.x >= extent.z) {
            float split_pt = 0.0;
                    
            for (auto p = start; p != end; p++) {
                split_pt += (*p)->get_bbox().centroid().x;
            }
            
            split_pt /= distance(start, end);
            
            middle = std::partition(start, end, [split_pt](const Primitive* p) {
                return p->get_bbox().centroid().x < split_pt;
            });
        } else if (extent.y >= extent.x && extent.y >= extent.z) {
            float split_pt = 0.0;
                    
            for (auto p = start; p != end; p++) {
                split_pt += (*p)->get_bbox().centroid().y;
            }
            
            split_pt /= distance(start, end);
            
            middle = std::partition(start, end, [split_pt](const Primitive* p) {
                return p->get_bbox().centroid().y < split_pt;
            });
        } else {
            float split_pt = 0.0;
                    
            for (auto p = start; p != end; p++) {
                split_pt += (*p)->get_bbox().centroid().z;
            }
            
            split_pt /= distance(start, end);
            
            middle = std::partition(start, end, [split_pt](const Primitive* p) {
                return p->get_bbox().centroid().z < split_pt;
            });
        }
        
        // Ensure we make some progress in the split
        
        if (middle == start || middle == end) {
            middle = start + (end - start) / 2;
        }
        
        auto leftChild = construct_bvh(start, middle, max_leaf_size);
        auto rightChild = construct_bvh(middle, end, max_leaf_size);
                
        BVHNode *node = new BVHNode(bbox);
        
        node->l = leftChild;
        node->r = rightChild;
        
        return node;
    }

}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.



  for (auto p : primitives) {
    total_isects++;
    if (p->has_intersection(ray))
      return true;
  }
  return false;


}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
    
      double t0, t1;
    
    if (node->bb.intersect(ray, t0, t1) == false) {
        return false;
    }

      if (node->isLeaf()) { // Leave node
            bool hit = false;
            for (auto p = node->start; p != node->end; p++) {
                total_isects++;
                hit = (*p)->intersect(ray, i) || hit;
            }
            return hit;
      }

      bool hit1 = false, hit2 = false;
    
      if (node->l) hit1 = intersect(ray, i, node->l);
      if (node->r) hit2 = intersect(ray, i, node->r);
        
      return hit1 || hit2;
}

} // namespace SceneObjects
} // namespace CGL
