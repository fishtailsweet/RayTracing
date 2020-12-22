#include "objloader.h"

// 值得注意的是文件中的索引值是以1作为起点的
ObjLoader::ObjLoader(const char *filename, std::shared_ptr<material> m) : triangles(std::make_shared<hittable_list>())
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){ // 必须得先打开
        qDebug()<<"obj open failed";
    }
    QTextStream text_stream(&file);
    QString str(text_stream.readLine());

    while(!text_stream.atEnd()){
        if(str[0] == '#'){} // 忽略注释
        else if(str.mid(0, 2) == "vn") {    // 必须这两个优先于'v'，否则就被'v'抢去了
            QStringList sections = str.simplified().split(' ');
            normal.emplace_back(sections[1].toDouble(), sections[2].toDouble(), -sections[3].toDouble());
        }
        else if(str.mid(0, 2) == "vt") {
            QStringList sections = str.simplified().split(' ');
            uv.emplace_back(sections[1].toDouble(), sections[2].toDouble());
        }
        else if(str[0] == 'v') {      // 相当于substr
            QStringList sections = str.simplified().split(' ');    // 去掉连续的空格
            vertex.emplace_back(sections[1].toDouble() * 60 + 150, sections[2].toDouble() * 60, -sections[3].toDouble() * 60 - 250);
        }
        else if(str[0] == 'f') {    // 多边形都转化为三角形
            QStringList sections = str.simplified().split(' '); // 去掉末尾多出来的空格
            std::vector<QStringList> subsection;
            QStringList subsections1 = sections[1].split('/');
            QStringList subsections2 = sections[2].split('/');
            QStringList subsections3;
            for(int i = 3; i < sections.size(); i++){ // 开头的f算一节；顶点坐标索引/纹理坐标索引/法线向量索引
                subsections3 = sections[i].split('/');
                triangles->add(std::make_shared<triangle>(vertex[subsections1[0].toInt() - 1], vertex[subsections2[0].toInt() - 1], vertex[subsections3[0].toInt() - 1],
                        normal[subsections1[2].toInt() - 1], normal[subsections2[2].toInt() - 1], normal[subsections3[2].toInt() - 1],
                        uv[subsections1[1].toInt() - 1], uv[subsections2[1].toInt() - 1], uv[subsections3[1].toInt() - 1], m));
                subsections2 = std::move(subsections3);
            }
        }
        str = text_stream.readLine();
    }
    triangles->cal_bounding_box();      // 千万别忘了
    qDebug()<<"finish";
    file.close();
}
