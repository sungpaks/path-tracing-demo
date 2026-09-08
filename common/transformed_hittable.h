#ifndef TRANSFORMED_HITTABLE_H
#define TRANSFORMED_HITTABLE_H

#include "transform.h"
#include "hittable.h"

#include <memory>
#include <stdexcept>

class transformed_hittable : public hittable {
public:
  transformed_hittable(std::shared_ptr<hittable> object, const transform& local_to_parent)
      : object(object), local_to_parent(local_to_parent) {
    if (!object)
      throw std::invalid_argument("object must not be null");
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    const ray local_ray(local_to_parent.inverse_point(r.origin()),
                        local_to_parent.inverse_vector(r.direction()));

    hit_record local_rec;
    if (!object->hit(local_ray, ray_t, local_rec))
      return false;

    const vec3 local_outward = local_rec.front_face ? local_rec.normal : -local_rec.normal;

    rec = local_rec;
    rec.p = r.at(rec.t);
    rec.set_face_normal(r, local_to_parent.apply_normal(local_outward));

    return true;
  }

private:
  std::shared_ptr<hittable> object;
  transform local_to_parent;
};

#endif