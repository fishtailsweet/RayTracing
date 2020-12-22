#include "rotate.h"

bool rotate_y::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const{
    // 不能在这儿修改传入光线的origin，因为并不一定光线就是打到了这个hittable，只有完全确定光线确实打到了这个hittable后，才能在传入光线本身上修改它的信息，即应在scatter_ray中修改而不是hit中
    // 但是不在这儿修改的话，即传入的不是r的话，r的hit_record中的t就没法被赋值了；能进行到这儿说明它以下的东西都是被平移的，故可以先修改，改完之后再改回来
    //ray moved_r(r.getOrigin() - offset, r.getDirection(), r.getTime());
    initial_origin = r.getOrigin();
    initial_direction = r.getDirection();

    double radian_angle = qDegreesToRadians(degree_angle);
    double sin_theta = sin(radian_angle);
    double cos_theta = cos(radian_angle);

    QVector3D new_origin, new_direction;
    new_origin[0] = cos_theta * initial_origin[0] - sin_theta * initial_origin[2];  // 反向旋转
    new_origin[1] = initial_origin[1];
    new_origin[2] = sin_theta * initial_origin[0] + cos_theta * initial_origin[2];

    new_direction[0] = cos_theta * initial_direction[0] - sin_theta * initial_direction[2];
    new_direction[1] = initial_direction[1];
    new_direction[2] = sin_theta * initial_direction[0] + cos_theta * initial_direction[2];

    r.setOrigin(std::move(new_origin));
    r.setDirection(std::move(new_direction));
    if (hittable_ptr->hit(r, t_min, t_max, hit_object)){

        hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);;这句话绝对不能落，因为我们要回到这里来传播
        r.setOrigin(initial_origin);
        r.setDirection(initial_direction);
        return true;
    }
    r.setOrigin(initial_origin);
    r.setDirection(initial_direction);
    return false;
}
void rotate_y::cal_bounding_box(){
   QVector3D min(INFINITY, INFINITY, INFINITY);
   QVector3D max(-INFINITY, -INFINITY, -INFINITY);

   for (int i = 0; i <= 1; i++) {
       for (int j = 0; j <= 1; j++) {
           for (int k = 0; k <= 1; k++) {
               double x = i * hittable_ptr->get_Bounding_box().getMax().x() + (1 - i) * hittable_ptr->get_Bounding_box().getMin().x();
               double y = j * hittable_ptr->get_Bounding_box().getMax().y() + (1 - j) * hittable_ptr->get_Bounding_box().getMin().y();
               double z = k * hittable_ptr->get_Bounding_box().getMax().z() + (1 - k) * hittable_ptr->get_Bounding_box().getMin().z();

               double radian_angle = qDegreesToRadians(degree_angle);
               double sin_theta = sin(radian_angle);
               double cos_theta = cos(radian_angle);

               double new_x =  cos_theta * x + sin_theta * z;
               double new_z = -sin_theta * x + cos_theta * z;

               QVector3D temp(new_x, y, new_z);

               for (int c = 0; c < 3; c++) {
                   min[c] = fmin(min[c], temp[c]);
                   max[c] = fmax(max[c], temp[c]);
               }
           }
       }
   }
   box = aabb(min, max);
}
