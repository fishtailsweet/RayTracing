#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <QVector3D>
#include <QFile>
#include "triangle.h"
#include "hittable_list.h"

class ObjLoader
{
public:
    ObjLoader(const char *filename, std::shared_ptr<material> m);
    std::shared_ptr<hittable_list> GetHittableListPtr(){return triangles;}
private:
    std::vector<QVector3D> vertex;
    std::vector<QVector3D> normal;
    std::vector<QVector2D> uv;
    std::shared_ptr<hittable_list> triangles;
};

#endif // OBJLOADER_H
