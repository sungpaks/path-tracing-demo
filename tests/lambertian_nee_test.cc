#include "camera.h"
#include "cube.h"
#include "hittable_list.h"
#include <stdexcept>
#include <iostream>

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int main() {
 try {
  auto light = make_shared<quad_light>(point3(-1,2,-1), vec3(2,0,0),
                                      vec3(0,0,2), color(1,1,1));
  auto diffuse = make_shared<lambertian>(color(0.72,0.72,0.72));
  hittable_list world;
  world.add(light);
  world.add(make_shared<cube>(point3(0,-0.1,0),vec3(100,0.2,100),diffuse));
  hit_record rec;
  require(light->hit(ray(point3(0,0,0),vec3(0,1,0)), interval(0.001,infinity),rec), "light hit");
  require(rec.front_face && rec.mat->emitted(rec).x()==1, "light front emission");
  require(light->hit(ray(point3(0,3,0),vec3(0,-1,0)), interval(0.001,infinity),rec), "light back hit");
  require(!rec.front_face && rec.mat->emitted(rec).length_squared()==0, "light back dark");
  color albedo;
  rec.p=point3(0.1,0,0.1); rec.normal=vec3(0,1,0);
  require(diffuse->nee_albedo(rec,albedo) && albedo.x()==0.72, "Lambertian provides albedo");
  rec.p=point3(0.6,0,0.1);
  require(diffuse->nee_albedo(rec,albedo) && albedo.x()==0.72, "uniform albedo at another point");
  require(lambertian(color(1,1,1)).nee_albedo(rec,albedo) && albedo.x()==1, "white Lambertian opts in");
  require(!dielectric(1.5).nee_albedo(rec,albedo), "glass stays unchanged");
  require(!metal(color(1,1,1),0).nee_albedo(rec,albedo), "metal stays unchanged");
  hittable_list blocked=world;
  blocked.add(make_shared<cube>(point3(0,1,0),vec3(10,0.1,10),diffuse));
  rec.p=point3(0,0,0);
  require(light->sample_direct(rec,color(1,1,1),blocked).length_squared()==0,"occlusion");

  // 고정 광선과 깊이 2로 바닥→광원 경로만 검사한다.
  // 두 방식의 평균을 별도의 면적 수치 적분값과 비교한다.
  camera cam; cam.image_width=1; cam.vfov=0; cam.max_depth=2;
  cam.EYE=point3(0,1,0); cam.AT=point3(0,0,0); cam.UP=vec3(0,0,1);
  cam.use_sky_background=false; cam.direct_light=light;
  double reference=0;
  const int grid=400;
  for(int z=0;z<grid;++z) for(int x=0;x<grid;++x) {
    double px=-1+(x+0.5)*2/grid, pz=-1+(z+0.5)*2/grid;
    double d2=4+px*px+pz*pz;
    reference+=(0.72/pi)*4/(d2*d2)*4/(grid*grid);
  }
  for(bool enabled : {false,true}) {
    random_generator().seed(12345);
    cam.enable_nee=enabled;cam.reset_accumulation();
    for(int i=0;i<100000;++i)cam.render_pass(world);
    double actual=cam.averaged_color(0,0).x();
    std::cout << "NEE="<<enabled<<" mean="<<actual<<" reference="<<reference<<'\n';
    require(std::abs(actual-reference)<0.005,"direct illumination mean / no double counting");
  }
  cam.max_depth=1;cam.reset_accumulation();cam.render_pass(world);
  require(cam.averaged_color(0,0).length_squared()==0,"depth cutoff equivalence");
  cam.max_depth=2;cam.EYE=point3(0,1,0);cam.AT=point3(0,2,0);
  cam.reset_accumulation();cam.render_pass(world);
  require(cam.averaged_color(0,0).x()==1,"camera sees emitter");
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
