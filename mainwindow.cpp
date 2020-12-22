#include "mainwindow.h"
#include <QImage>
#include <QDir>

void MainWindow::initializeGL(){
    QImage image(width(), height(), QImage::Format_RGB32);
    //random_scene();
    //cornell_box();
    final_scene();
    //camera cam(width(), height(), 20, QVector3D(13, 2, -3), QVector3D(0, 2, 0), QVector3D(0, 1, 0), 0.1, 10, 0.0, 1.0);  // 用于random_scene
    //camera cam(width(), height(), 40, QVector3D(278, 278, 800), QVector3D(278, 278, 0), QVector3D(0, 1, 0), 0.1, 5, 0.0, 1.0);  // 用于random_scene
    camera cam(width(), height(), 40, QVector3D(478, 278, 600), QVector3D(278, 278, 0), QVector3D(0, 1, 0), 0.1, 1000, 0.0, 1.0);  // 用于random_scene

    // QColor会自动将每个分量都除以255，很狗的
    for (int i = 0; i < width(); ++i) { // 从左上角开始，从相机位置向虚拟屏幕中的每个像素发射光线
        for (int j = 0; j < height(); ++j) {
            QVector3D pixel_color(0, 0, 0); // 用于对虚拟屏幕每个像素每次采样得到的颜色值取平均
            for (int k = 0; k < cam.get_samples_num(); ++k) {   // 循环 向每个像素发射的光线的个数即采样次数 次
                double u = (i + random_double()) / (width() - 1);   // 在该像素的范围内随机偏移，以实现抗锯齿；真实的相机自带抗锯齿
                double v = (j + random_double()) / (height() - 1);
                ray r = cam.get_ray(u, v);  // 从相机位置发射光线
                QVector3D cur_color = my_world.hit_world(r, true);  // 由于浮点数精度问题，应忽略t非常接近0的交点
                pixel_color += cur_color;
            }
            float r = pixel_color.x();
            float g = pixel_color.y();
            float b = pixel_color.z();

            if (r != r) r = 0.0;    // 去除所有NaN，因为NaN不等于自身
            if (g != g) g = 0.0;
            if (b != b) b = 0.0;

            QVector3D tone_color = ACESToneMapping(pixel_color / cam.get_samples_num(), 2);   // 第二个参数越大越亮，大于1时比原图亮，小于1时比原图暗
            pixel_color = QVector3D(sqrt(tone_color.x()), sqrt(tone_color.y()), sqrt(tone_color.z()));   // 对虚拟屏幕每个像素每次采样得到的颜色值取平均；加到一起整体做除法浮点数误差比较小
            // 可以在这里进行gamma校正，如认为γ = 2，则pixel_color = sqrt(pixel_color / cam.get_samples_num())
            // 几乎所有图像查看器都假定图像是经过“伽马校正”的，这意味着0到1的值在存储为字节之前会有一些变换
            image.setPixelColor(i, j, QColor(qBound(0.0, static_cast<double>(pixel_color.x()), 1.0) * 255.0, qBound(0.0, static_cast<double>(pixel_color.y()), 1.0) * 255.0, qBound(0.0, static_cast<double>(pixel_color.z()), 1.0) * 255.0));
            // qBound等价于clamp，之所以要static_cast是因为要将float转换成double，这样模板函数的三个参数T才是相同类型，模板函数才能执行
        }
        qDebug()<<i;
    }

    initializeOpenGLFunctions();    // 必须有这句话
    m_shader = new QOpenGLShaderProgram();  // 初始化shader

    //m_texture = new QOpenGLTexture(QImage(":/imgs/test.png").mirrored());   // Windows系统和QPaintDevice中屏幕坐标原点在左上角，但OpenGL中纹理坐标原点在左下角，故需要将读入的图片垂直翻转以用在OpenGL中
    m_texture = new QOpenGLTexture(image.mirrored());

    // 图像变换都是反向映射
    m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);  // 纹理过大即需要纹理缩小时，反向映射即一个像素映射到多个像素，故需要mipmap即对覆盖的这些像素做平均
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);  // 纹理过小即需要纹理放大时，反向映射即多个像素映射到一个像素，故需要双线性插值

    m_vao = new QOpenGLVertexArrayObject();     // 初始化VBO的数组VAO
    m_vao->create();
    // OpenGL本身是一个巨大的状态机，上下文context决定接下来要如何渲染
    m_vao->bind();      // 每个OpenGL对象（如VAO、VBO、shader）都必须先bind()才能被QOpenGLFunctions对象使用，作用是设置该OpenGL对象为当前context()，不再使用了也必须立即release()
    glEnable(GL_DEPTH_TEST);      // QOpenGLFunctions中的函数，Z-Test
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);       // 初始化用于存储顶点坐标的缓冲区
    m_vbo->create();
    m_vbo->bind();  // 设置该缓冲区为当前context()的默认缓冲区
    std::unique_ptr<GLfloat> vertex_coordinate(new GLfloat[6 * 3]{  -1.0, 1.0, 0.0,   // 左上
                                                                    1.0,  1.0, 0.0,   // 右上
                                                                    1.0,  -1.0, 0.0,  // 右下
                                                                    -1.0, 1.0, 0.0, // 左上
                                                                    1.0,  -1.0, 0.0,  // 右下
                                                                    -1.0, -1.0, 0.0});    // 左下
    // 顶点在模型空间中的坐标，每个坐标的范围被映射到[-1, 1]；在屏幕前渲染一个三角形面片
    // 因为OpenGL是右手坐标系，故向右为x轴，向上为y轴，垂直于屏幕向外为z轴
    m_vbo->allocate(vertex_coordinate.get(), 6 * 3 * sizeof(GLfloat));    // 分配坐标缓冲区，第一个参数是初始时要填入的数据的数组（可省略），第二个参数是该缓冲区占的字节数即大小
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);    // QOpenGLFunctions中的函数
    // 第一个参数指定这是VAO中哪个索引上的缓冲区，第二个参数指定几个元素为一组，第四个参数指定是否需要对每组normalize，第三个和第五个参数分别为元素类型和每组占的字节数，最后一个参数为缓冲区起点位置的偏移量
    glEnableVertexAttribArray(0);        // QOpenGLFunctions中的函数，指定这是VAO中哪个索引上的缓冲区
    m_vbo->release();   // 及时释放，以便更改设置其他缓冲区为当前context()的默认缓冲区
    m_uvbo = new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);  // 初始化用于存储顶点uv坐标的缓冲区
    m_uvbo->create();
    m_uvbo->bind();
    std::unique_ptr<GLfloat> uvData(new GLfloat[6 * 2]{  0.0f, 1.0f,    // 左上
                                                         1.0f, 1.0f,    // 右上
                                                         1.0f, 0.0f,    // 右下
                                                         0.0f, 1.0f,     // 左上
                                                         1.0f, 0.0f,    // 右下
                                                         0.0f, 0.0f});  // 左下
    // OpenGL中纹理坐标原点在左下角，故需要将读入的图片垂直翻转以用在OpenGL中
    m_uvbo->allocate(uvData.get(), 6 * 2 * sizeof(GLfloat));  // 分配坐标缓冲区，第一个参数是初始时要填入的数据的数组（可省略），第二个参数是该缓冲区占的字节数即大小
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);    // QOpenGLFunctions中的函数
    glEnableVertexAttribArray(1);   // QOpenGLFunctions中的函数，指定这是VAO中哪个索引上的缓冲区
    m_uvbo->release();
    m_vao->release();
}

void MainWindow::paintGL() {    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);      // QOpenGLFunctions中的函数，每帧都先清空颜色和深度缓冲区
    m_vao->bind();
    rotate_pic(0);    // 两个rotate都有问题，应该是三角函数的问题
    //edge_detector(1, QVector4D(0, 0, 0, 1), QVector4D(1, 1, 1, 1));
    //gaussian_blur1D(50, 1.0, std::unique_ptr<float>(new float[3]{0.4026, 0.2442, 0.0545}));  // 为了避免改变图像整体亮度，所有权重相加的和必须为1
    //radial_blur(10, 0.02, QVector2D(0.5, 0.5)); // 第二个参数应属于[0, 0.05]，是每次步长变化的幅度
    //whirlpool(-40, QVector2D(0.5, 0.5));    // 第一个参数为正时纵向漩涡，为负时横向漩涡
    //RGB_split_glitch(1.0, QVector2D(-0.01, 0.01));
    m_vao->release();
}

void MainWindow::resizeGL(int w, int h) // 当前绘图区域的长和高
{
    paintGL();
}

MainWindow::MainWindow(QWidget *parent) : QOpenGLWidget(parent), my_world(QVector3D(0, 0, 0), 0.98, 0.001, INFINITY)
{
    resize(800, 800);
    /*m_layout = new QVBoxLayout(this);
    setLayout(m_layout);    // 设置布局管理器
    m_combo_box = new QComboBox(this);
    m_layout->addWidget(m_combo_box);
    m_combo_box->addItem("Red");
    m_combo_box->addItem("Blue");
    m_combo_box->addItem("Yellow");
    m_combo_box->addItem("Green");
    connect(m_combo_box, &QComboBox::currentTextChanged, this, &MainWindow::onComboBoxSelected);*/
}

MainWindow::~MainWindow()
{
    QImage image = grabFramebuffer();   // 写的shader会导致折痕，我也不知道为什么
    QFile file("raytracing.png");
    file.open(QIODevice::WriteOnly);
    if(image.save(&file, "PNG")){
        qDebug() << "done";
    }
}

/*void MainWindow::paintEvent(QPaintEvent *e){
    QPainter painter(this);
    random_scene();
    camera cam(width(), height(), 20, QVector3D(13, 2, -3), QVector3D(0, 0, 0), QVector3D(0, 1, 0), 0.1, 1);
    for (int i = 0; i < width(); ++i) { // 从左上角开始，从相机位置向虚拟屏幕中的每个像素发射光线
        for (int j = 0; j < height(); ++j) {
            QVector3D pixel_color(0, 0, 0); // 用于对虚拟屏幕每个像素每次采样得到的颜色值取平均
            for (int k = 0; k < cam.get_samples_num(); ++k) {   // 循环 向每个像素发射的光线的个数即采样次数 次，
                double u = (i + random_double()) / (width() - 1);   // 在该像素的范围内随机偏移
                double v = (j + random_double()) / (height() - 1);
                ray r = cam.get_ray(u, v);  // 从相机位置发射光线
                QVector3D cur_color = world.hit_world(r, 0.001, INFINITY, 0.98);  // 由于浮点数精度问题，应忽略t非常接近0的交点
                pixel_color += cur_color;
            }
            pixel_color /= cam.get_samples_num();   // 对虚拟屏幕每个像素每次采样得到的颜色值取平均；加到一起整体做除法误差比较小
            // 可以在这里进行gamma校正，如认为γ = 2，则pixel_color = sqrt(pixel_color / cam.get_samples_num())
            painter.setPen(QColor::fromRgb(256 * clamp(pixel_color.x(), 0.0, 0.999), 256 * clamp(pixel_color.y(), 0.0, 0.999), 256 * clamp(pixel_color.z(), 0.0, 0.999)));
            painter.drawPoint(i, j);
        }
    }
}*/

void MainWindow::random_scene() {  // 生成随机场景
    auto checker = std::make_shared<checker_texture>(QVector3D(0.2, 0.3, 0.1), QVector3D(0.9, 0.9, 0.9));
    auto ground_material = std::make_shared<lambertian>(checker);
    my_world.add(std::make_shared<sphere>(QVector3D(0, -1000, 0), 1000, ground_material));
    // 三个大家伙
    auto material1 = std::make_shared<dielectric>(1.5);
    my_world.add(std::make_shared<sphere>(QVector3D(0, 1, 0), 1.0, material1));
    auto material2 = std::make_shared<lambertian>(QVector3D(0.4, 0.2, 0.1));
    my_world.add(std::make_shared<sphere>(QVector3D(-4, 1, 0), 1.0, material2));
    auto material3 = std::make_shared<metal>(QVector3D(0.7, 0.6, 0.5), 0.0, 0.5);
    my_world.add(std::make_shared<sphere>(QVector3D(4, 1, 0), 1.0, material3));
    // 随机的小家伙
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = random_double();
            QVector3D center(a + 0.9 * random_double(), 0.2, -(b + 0.9 * random_double()));
            if ((center - QVector3D(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random_vec3() * random_vec3();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    auto center2 = center + QVector3D(0, random_double(0, .5), 0);
                    my_world.add(std::make_shared<moving_sphere>(center, center2, 0.2, sphere_material, 0.0, 1.0));    // 后两个参数必须与相机构造函数中的后两个参数相同，以与相机光圈的打开和关闭相匹配
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random_vec3(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz, 0.5);
                    my_world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    my_world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
    my_world.cal_bounding_box();       // 必须有这句话，表示物体已经添加完了，可以计算包围盒了
    // 空心玻璃球glass实际上没有阴影，这使它们看起来像是漂浮的
}

void MainWindow::cornell_box() {
    // 我们的程序中统一用[0, 1]的颜色，到最后mainwindow中再乘一个255
    // 由于光线很小，此图像非常嘈杂
    // 如果我们用烟雾和雾气（深色和浅色粒子）替换了两个块，并增大了灯光的亮度（并调暗了亮度，以免爆破场景），以便更快地收敛
    // 次表面散射材质，它有点像物体内的浓雾
    auto red   = std::make_shared<lambertian>(QVector3D(.65, .05, .05));
    auto white = std::make_shared<lambertian>(QVector3D(.73, .73, .73));
    auto green = std::make_shared<lambertian>(QVector3D(.12, .45, .15));
    auto light_mat = std::make_shared<diffuse_light>(QVector3D(15, 15, 15));   // 大于(1, 1, 1)即HDR，亮度更大，这样可以使其足够明亮以照亮事物

    my_world.add(std::make_shared<yz_rect>(0, 555, -555, 0, 0, green));
    my_world.add(std::make_shared<yz_rect>(0, 555, -555, 0, 555, red));
    auto light_obj = std::make_shared<flip_face>(std::make_shared<xz_rect>(213, 343, -332, -227, 554, light_mat));
    my_world.add_light(light_obj);
    my_world.add(light_obj);

    my_world.add(std::make_shared<xz_rect>(0, 555, -555, 0, 555, white));
    my_world.add(std::make_shared<xz_rect>(0, 555, -555, 0, 0, white));
    my_world.add(std::make_shared<xy_rect>(0, 555, 0, 555, -555, white));

    std::shared_ptr<hittable> box1 = std::make_shared<cube>(QVector3D(0, 0, -165), QVector3D(165, 330, 0), white);
    box1 = std::make_shared<rotate_y>(box1, -15);
    box1 = std::make_shared<translate>(box1, QVector3D(265, 0, -295));
    std::shared_ptr<hittable> box2 = std::make_shared<cube>(QVector3D(0, 0, -165), QVector3D(165, 165, 0), white);
    box2 = std::make_shared<rotate_y>(box2, 18);
    box2 = std::make_shared<translate>(box2, QVector3D(130, 0, -65));

    my_world.add(box1);
    my_world.add(box2);

    //my_world.add(std::make_shared<constant_medium>(box1, 0.01, QVector3D(0.2, 0.4, 0.6)));
    //my_world.add(std::make_shared<constant_medium>(box2, 0.01, QVector3D(0.6, 0.4, 0.2)));

    //ObjLoader loader(":/objs/test.obj", std::make_shared<lambertian>(std::make_shared<image_texture>(":/imgs/laopo.jpg")));
    //ObjLoader loader(":/objs/bench.obj", white);
    //my_world.add(loader.GetHittableListPtr());

    my_world.cal_bounding_box();       // 必须有这句话，表示物体已经添加完了，可以计算包围盒了
}

void MainWindow::final_scene(){
    // 用薄雾覆盖所有事物，然后用一个蓝色的次表面反射球体（我们没有明确实现这一点，但是电介质内部的媒介才是次表面的材料）
    // 渲染器中留下的最大限制是没有阴影射线，但这就是为什么我们免费获得焦散和次表面的原因。这是一个双刃剑
    hittable_list boxes1;
    auto ground = std::make_shared<lambertian>(QVector3D(0.48, 0.83, 0.53));

        const int boxes_per_side = 20;
        for (int i = 0; i < boxes_per_side; i++) {
            for (int j = 0; j < boxes_per_side; j++) {
                auto w = 100.0;
                auto x0 = -1000.0 + i*w;
                auto z0 = 1000.0 - j*w;
                auto y0 = 0.0;
                auto x1 = x0 + w;
                auto y1 = random_double(1,101);
                auto z1 = z0 - w;

                boxes1.add(std::make_shared<cube>(QVector3D(x0,y0,z1), QVector3D(x1,y1,z0), ground));
            }
        }
        boxes1.cal_bounding_box();
        my_world.add(std::make_shared<hittable_list>(boxes1));  // 很妙，这样以后添加模型的时候也可以这样添加了

        auto light_mat = std::make_shared<diffuse_light>(QVector3D(7, 7, 7));
        auto mat_obj = std::make_shared<flip_face>(std::make_shared<xz_rect>(123, 423, -412, -147, 554, light_mat));
        my_world.add_light(mat_obj);
        my_world.add(mat_obj);

        //auto light_obj = std::make_shared<sphere>(QVector3D(260, 150, -45), 50, light_mat);
        //my_world.add_light(light_obj);
        //my_world.add(light_obj);

        auto center1 = QVector3D(400, 400, -200);
        auto center2 = center1 + QVector3D(30,0,0);
        auto moving_sphere_material = std::make_shared<lambertian>(QVector3D(0.7, 0.3, 0.1));
        my_world.add(std::make_shared<moving_sphere>(center1, center2, 50, moving_sphere_material, 0, 1));

        //my_world.add(std::make_shared<sphere>(QVector3D(260, 150, -45), 50, std::make_shared<dielectric>(1.5)));
        my_world.add(std::make_shared<sphere>(QVector3D(0, 150, -145), 50, std::make_shared<metal>(QVector3D(0.8, 0.8, 0.9), 0.5, 0.5)));

        auto boundary = std::make_shared<sphere>(QVector3D(360,150,-145), 70, std::make_shared<dielectric>(1.5));
        my_world.add(boundary); // 为了要外面那层镜面反射效果
        //my_world.add(std::make_shared<constant_medium>(boundary, 0.2, QVector3D(0.2, 0.4, 0.9)));
        boundary = std::make_shared<sphere>(QVector3D(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
        //my_world.add(std::make_shared<constant_medium>(boundary, .0001, QVector3D(1,1,1)));

        auto emat = std::make_shared<lambertian>(std::make_shared<image_texture>(":/imgs/earthmap.jpg"));
        my_world.add(std::make_shared<sphere>(QVector3D(400,200,-400), 100, emat));
        auto pertext = std::make_shared<noise_texture>(0.1);
        my_world.add(std::make_shared<sphere>(QVector3D(220,280,-300), 80, std::make_shared<lambertian>(pertext)));

        hittable_list boxes2;
        auto white = std::make_shared<lambertian>(QVector3D(.73, .73, .73));
        int ns = 1000;
        for (int j = 0; j < ns; j++) {
            boxes2.add(std::make_shared<sphere>(random_vec3(0,165), 10, white));
        }
        boxes2.cal_bounding_box();
        my_world.add(std::make_shared<translate>(std::make_shared<rotate_y>(std::make_shared<hittable_list>(boxes2), -15), QVector3D(-100,270,-395)));
        my_world.cal_bounding_box();
}

void MainWindow::rotate_pic(float degree){
    if(!m_shader->isLinked()){
        m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vert.shader");  // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragRotateBiLinear.shader");   // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        // 路径以 : 开头表示在资源文件中查找资源
        if (m_shader->link()) {     // link()用于编译和链接GLSL
            qDebug("Shaders link success");
        }
        else {
            qDebug("Shaders link fail");
        }
    }
    m_shader->bind();       // 设置该shader为当前context()的默认shader，只有先bind()才能被QOpenGLFunctions对象使用

    float cos_value = cos(M_PI * -degree / 180);        // 要取负即反向旋转
    float sin_value = sin(M_PI * -degree / 180);
    // <QtMath>自带M_PI和角度转弧度float qDegreesToRadians(float degrees)；内置三角函数用的都是弧度
    std::unique_ptr<float> mat_data(new float[4]{cos_value, -sin_value, sin_value, cos_value});   // 二维旋转矩阵
    QMatrix2x2 rotate_mat(mat_data.get()); // 需要从内存中将对于每个片元值都相同（即uniform）的旋转矩阵传入gpu，以避免在shader中重复计算
    m_shader->setUniformValue("trans", rotate_mat);    // 用于在frag中旋转图片

    m_shader->setUniformValue("imgTexture", 0); // 将uniform sampler2D imgTexture绑定为编号为0的纹理

    m_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToBorder);    // 纹理循环方式
    m_texture->setBorderColor(0, 0, 0, 1);  // 黑边
    m_texture->bind(0);  // 将对于每个片元都相同（即uniform）的m_texture作为编号为0的纹理传入显存
    m_shader->setUniformValue("tex_size", QVector2D(m_texture->width(), m_texture->height()));    // 需要从内存中将对于每个片元值都相同（即uniform）的纹理大小传入显存，以用于双线性/双三次插值

    glDrawArrays(GL_TRIANGLES, 0, 6);    // QOpenGLFunctions中的函数；三角形面片，第二个参数指定从哪组开始绘制，第二个参数指定绘制几组
    m_shader->removeAllShaders();

    m_shader->release();
}

void MainWindow::edge_detector(float edge_level, const QVector4D &edge_color, const QVector4D &background_color){    // QVector4D不能用QColor代替，否则会出错
    if(!m_shader->isLinked()){
        m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertEdgeDetector.shader");  // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragEdgeDetector.shader");   // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        // 路径以 : 开头表示在资源文件中查找资源
        if (m_shader->link()) {     // link()用于编译和链接GLSL
            qDebug("Shaders link success");
        }
        else {
            qDebug("Shaders link fail");
        }
    }
    m_shader->bind();       // 设置该shader为当前context()的默认shader，只有先bind()才能被QOpenGLFunctions对象使用

    std::unique_ptr<float> mat_data(new float[9]{-1, -2, -1, 0, 0, 0, 1, 2, 1});   // 二维旋转矩阵
    QMatrix3x3 filter_x(mat_data.get()); // 需要从内存中将对于每个片元值都相同（即uniform）的旋转矩阵传入gpu，以避免在shader中重复计算
    mat_data.reset(new float[9]{-1, 0, 1, -2, 0, 2, -1, 0, 1});
    QMatrix3x3 filter_y(mat_data.get()); // 需要从内存中将对于每个片元值都相同（即uniform）的旋转矩阵传入gpu，以避免在shader中重复计算
    m_shader->setUniformValue("filter_x", filter_x);    // 用于在frag中旋转图片
    m_shader->setUniformValue("filter_y", filter_y);    // 用于在frag中旋转图片
    m_shader->setUniformValue("tex_size", QVector2D(width(), height()));    // 需要从内存中将对于每个片元值都相同（即uniform）的纹理大小传入显存，以用于双线性/双三次插值
    m_shader->setUniformValue("edge_level", edge_level);    // 用于在frag中旋转图片
    m_shader->setUniformValue("edge_color", edge_color);    // 用于在frag中旋转图片
    m_shader->setUniformValue("background_color", background_color);    // 用于在frag中旋转图片

    m_shader->setUniformValue("imgTexture", 0); // 将uniform sampler2D imgTexture绑定为编号为0的纹理
    m_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘
    m_texture->bind(0);  // 将对于每个片元都相同（即uniform）的m_texture作为编号为0的纹理传入显存

    glDrawArrays(GL_TRIANGLES, 0, 6);    // QOpenGLFunctions中的函数；三角形面片，第二个参数指定从哪组开始绘制，第二个参数指定绘制几组
    m_shader->removeAllShaders();

    m_shader->release();
}

void MainWindow::gaussian_blur1D(int blur_num, float blur_size, std::unique_ptr<float> gaussian_weight){
    QOpenGLFramebufferObject *m_fbo = new QOpenGLFramebufferObject(width(), height(), GL_TEXTURE_2D);
    for(int i = 1; i <= blur_num * 2; i++){
        if(i == 1){
            if(!m_shader->isLinked()){    // 不能再有这句话了，不然会出错
                m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertGaussianBlur1D.shader");
                m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragGaussianBlur1D.shader");
                m_shader->link();
                if (m_shader->link()) {
                    qDebug("Shaders link success");
                }
                else {
                    qDebug("Shaders link fail");
                }
            }

            m_shader->bind();
            m_fbo->bind();

            m_shader->setUniformValue("blur_size", blur_size);
            m_shader->setUniformValueArray("gaussian_weight", gaussian_weight.get(), 3, 1);
            m_shader->setUniformValue("imgTexture", 0);
            m_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘

            m_texture->bind(0);
            m_shader->setUniformValue("tex_size", QVector2D(m_texture->width(), m_texture->height()));

            m_shader->setUniformValue("hor_or_ver", QVector2D(1, 0));   // 水平

            glDrawArrays(GL_TRIANGLES, 0, 6);

            m_shader->release();
            m_fbo->release();
        }
        else if(i == blur_num * 2){
            m_shader->bind();

            m_shader->setUniformValue("blur_size", blur_size);
            m_shader->setUniformValueArray("gaussian_weight", gaussian_weight.get(), 3, 1);

            QOpenGLTexture *temp_texture = new QOpenGLTexture(m_fbo->toImage().mirrored());
            temp_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘
            temp_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);  // 纹理过大即需要纹理缩小时，反向映射即一个像素映射到多个像素，故需要mipmap即对覆盖的这些像素做平均
            temp_texture->setMagnificationFilter(QOpenGLTexture::Linear);  // 纹理过小即需要纹理放大时，反向映射即多个像素映射到一个像素，故需要双线性插值
            m_shader->setUniformValue("imgTexture", 0);

            temp_texture->bind(0);
            m_shader->setUniformValue("tex_size", QVector2D(temp_texture->width(), temp_texture->height()));

            m_shader->setUniformValue("hor_or_ver", QVector2D(0, 1));   // 竖直

            glDrawArrays(GL_TRIANGLES, 0, 6);   // 必须在所有Uniform都set完后再draw
            m_shader->removeAllShaders();

            m_shader->release();
        }
        else{
            m_shader->bind();
            m_fbo->bind();

            m_shader->setUniformValue("blur_size", blur_size);
            m_shader->setUniformValueArray("gaussian_weight", gaussian_weight.get(), 3, 1);

            QOpenGLTexture *temp_texture = new QOpenGLTexture(m_fbo->toImage().mirrored());

            temp_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘
            temp_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);  // 纹理过大即需要纹理缩小时，反向映射即一个像素映射到多个像素，故需要mipmap即对覆盖的这些像素做平均
            temp_texture->setMagnificationFilter(QOpenGLTexture::Linear);  // 纹理过小即需要纹理放大时，反向映射即多个像素映射到一个像素，故需要双线性插值
            m_shader->setUniformValue("imgTexture", 0);

            temp_texture->bind(0);
            m_shader->setUniformValue("tex_size", QVector2D(temp_texture->width(), temp_texture->height()));

            if(i % 2 == 0){
                m_shader->setUniformValue("hor_or_ver", QVector2D(0, 1));   // 竖直
            }
            else{
                m_shader->setUniformValue("hor_or_ver", QVector2D(1, 0));   // 水平
            }

            glDrawArrays(GL_TRIANGLES, 0, 6);   // 必须在所有Uniform都set完后再draw

            m_shader->release();
            m_fbo->release();
        }
    }
}

void MainWindow::radial_blur(int blur_num, float blur_size, const QVector2D &center){   // 速度感
    if(!m_shader->isLinked()){
        m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vert.shader");  // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragRadialBlur.shader");   // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        // 路径以 : 开头表示在资源文件中查找资源
        if (m_shader->link()) {     // link()用于编译和链接GLSL
            qDebug("Shaders link success");
        }
        else {
            qDebug("Shaders link fail");
        }
    }
    m_shader->bind();       // 设置该shader为当前context()的默认shader，只有先bind()才能被QOpenGLFunctions对象使用

    m_shader->setUniformValue("blur_num", blur_num);    // 用于在frag中旋转图片
    m_shader->setUniformValue("blur_size", blur_size);    // 用于在frag中旋转图片
    m_shader->setUniformValue("center", center);    // 用于在frag中旋转图片

    m_shader->setUniformValue("imgTexture", 0); // 将uniform sampler2D imgTexture绑定为编号为0的纹理
    m_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘
    m_texture->bind(0);  // 将对于每个片元都相同（即uniform）的m_texture作为编号为0的纹理传入显存

    glDrawArrays(GL_TRIANGLES, 0, 6);    // QOpenGLFunctions中的函数；三角形面片，第二个参数指定从哪组开始绘制，第二个参数指定绘制几组
    m_shader->removeAllShaders();

    m_shader->release();
}

void MainWindow::whirlpool(float rotate_level, const QVector2D &center){
    if(!m_shader->isLinked()){
        m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vert.shader");  // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragWhirlpool.shader");   // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        // 路径以 : 开头表示在资源文件中查找资源
        if (m_shader->link()) {     // link()用于编译和链接GLSL
            qDebug("Shaders link success");
        }
        else {
            qDebug("Shaders link fail");
        }
    }
    m_shader->bind();       // 设置该shader为当前context()的默认shader，只有先bind()才能被QOpenGLFunctions对象使用

    m_shader->setUniformValue("rotate_level", rotate_level);    // 用于在frag中旋转图片
    m_shader->setUniformValue("center", center);    // 用于在frag中旋转图片

    m_shader->setUniformValue("imgTexture", 0); // 将uniform sampler2D imgTexture绑定为编号为0的纹理
    m_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘
    m_texture->bind(0);  // 将对于每个片元都相同（即uniform）的m_texture作为编号为0的纹理传入显存

    glDrawArrays(GL_TRIANGLES, 0, 6);    // QOpenGLFunctions中的函数；三角形面片，第二个参数指定从哪组开始绘制，第二个参数指定绘制几组
    m_shader->removeAllShaders();

    m_shader->release();
}

void MainWindow::RGB_split_glitch(float split_level, const QVector2D &split_dir){
    if(!m_shader->isLinked()){
        m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vert.shader");  // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragRGBSplitGlitch.shader");   // 函数为addShaderFromSourceCode时第二个参数可以是const char *
        // 路径以 : 开头表示在资源文件中查找资源
        if (m_shader->link()) {     // link()用于编译和链接GLSL
            qDebug("Shaders link success");
        }
        else {
            qDebug("Shaders link fail");
        }
    }
    m_shader->bind();       // 设置该shader为当前context()的默认shader，只有先bind()才能被QOpenGLFunctions对象使用

    m_shader->setUniformValue("split_level", split_level);    // 用于在frag中旋转图片
    m_shader->setUniformValue("split_dir", split_dir);    // 用于在frag中旋转图片

    m_shader->setUniformValue("imgTexture", 0); // 将uniform sampler2D imgTexture绑定为编号为0的纹理
    m_texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);    // 纹理循环方式，重复边缘
    m_texture->bind(0);  // 将对于每个片元都相同（即uniform）的m_texture作为编号为0的纹理传入显存

    glDrawArrays(GL_TRIANGLES, 0, 6);    // QOpenGLFunctions中的函数；三角形面片，第二个参数指定从哪组开始绘制，第二个参数指定绘制几组
    m_shader->removeAllShaders();

    m_shader->release();
}

void MainWindow::onComboBoxSelected(const QString &text)        // slot函数的定义
{
}
// 可通过直接向光源发送光线实现更快的光线追踪，也可通过将散射的光线向光源倾斜，然后降低这些光线的权重来隐式实现
// 不做阴影射线（相反，我使射线更可能朝向光），双向方法，Metropolis方法或光子贴图
// MC可以通过对样本进行分层以提高效率，但这种提高会随着问题维度的升高而降低，而每个反射都会增加两个维度，因此这里不进行分层，但如果做单反射或阴影或某些2D问题，则肯定要分层
// 蒙特卡罗方法实际上就是把积分看作黎曼积分，一小条一小条加到一起，每条的宽是 定积分区间的宽/采样次数，高是采样点对应的函数值，然后把所有条加到一起即可，而因为每条的宽是相等的故可以提出来，而单纯先把每条的高加到一起，到最后整体乘上每条的宽即 定积分区间的宽/采样次数，这里的 1/定积分区间的宽 实际上就是在定积分区间上均匀分布的pdf
// for (int i = 0; i < N; i++) {auto x = random_double(0,2);sum += f(x);}std::cout << "I = " << 2 * sum/N << '\n';
// 小光源会产生太多的噪声。这是因为统一采样不能对这些光源进行足够频繁的采样。仅当光线向光源散射时才对光源进行采样，但这对于较小的光线或远处的光线不太可能发生
// 如果向该光发送更多的随机样本，则可以减少此问题，但这将导致场景的亮度不准确。可以通过降低这些样本的权重来针对过度采样进行调整，从而消除这种不准确性
// 不同pdf虽然在极限处都能收敛到正确结果，但选择好的pdf可以加快收敛速度；在采样过多的地方，应该减轻权重。PDF是完成多少采样的完美度量。因此，应服从一定的pdf进行采样，且采样后应除以该点处的pdf值，然后加到一起，最后整体除以 采样次数，这时没有 *定积分区间的宽 即 除以1/定积分区间的宽 了因为该pdf不像均匀分布一样每个点都一样故可以提出来，而是隐藏在 /pdf 中了
// 按服从一定pdf的分布采样是一件很难的事情，最简单的方法是逆函数法，然后有舍选法、蒙特卡洛法等，该pdf在定积分区间上一定不为0但在非定积分区间上一定为0
// for (int i = 0; i < N; i++) {auto x = sqrt(random_double(0,4));sum += f(x) / pdf(x);}std::cout << "I = " << sum/N << '\n';
// 由于我们在被积大的地方采样更多，因此可以期望得到更少的噪声，从而加快收敛速度，使用非均匀PDF称为重要性抽样；自然当采样用的pdf完全等于 被积函数/要求的积分值 时效率最高，即先人为选择了pdf形式为被积函数，然后对该pdf在定积分区间上归一化，
//（且这时一定满足在定积分区间上对该采样用的pdf积分得到1即归一化的pdf，因为可以把 /要求的积分值提出去 得 对被积函数在定积分区间上定积分 / 积分值，得到的肯定是1），因为这时 /pdf 即把被积函数值除下去了，无论哪个采样点采样一次得到的都是要求的那个积分值，但这个要求的积分值肯定是不可知的即归一化系数不可知，
// 故应选择形式与被积函数类似的但能求出归一化系数的函数作为pdf，记得它要满足在定积分区间上一定不为0但在非定积分区间上一定为0
// 总地来说，对f(x)求积分≈对f(r) / p(r)做平均where r is a random number with PDF p
// 在光线追踪器中，选择随机方向，并且方向可以表示为单位球体上的点。应用与以前相同的方法，但是现在需要在2D上定义PDF，可以用球面坐标(θ, φ)，但更常用的是立体角
// for (int i = 0; i < N; i++) {vec3 d = random_on_unit_halfsphere_surface();auto cosine_squared = d.z()*d.z();sum += cosine_squared / pdf(d);}std::cout << "I = " << sum/N << '\n';这里的pdf(d)就是均匀分布的pdf即 1/半球的表面积
// 使其向光源发送一束额外的光线，从而使图像的噪点更少；pdf是正常反射方向的pdf即间接光照和向光源方向的光线的pdf即直接光照的加权平均，可以是任意形式的加权平均因为记住任意pdf都会收敛于正确结果，只是收敛的快慢不同而已
// 对于直接光照，设面光源面积为A，则最简单的均匀采样时pdf为1/A，还需要把渲染方程中的积分从对入射光立体角的积分转换为对面光源上的点的积分，这只需求出微分立体角dw和微分光源面dA的关系即可，方法是连接着色点和面光源上的某采样点，
// 设面光源上该采样点处的法线和上述连线的夹角是θ'，则由立体角定义 对应的投影到球面上的面积 / 半径^2 得dw = dAcosθ' / (光源上的该采样点 - 着色点)的模的平方，而采样dw和采样dA的概率应相等即pdf_向光源发射光线 * dw = (1/A) * dA，故得pdf_向光源发射光线 = (dA / dw) / A = (光源上的该采样点 - 着色点)的模的平方 / (cosθ' * A)；
// 这样每次递归就分为两部分：直接光照部分对面光源上的点采样并判断当前着色点和该采样点的连线上是否有其他物体，若有则此部分的值为0，若没有则用积分变量为面光源上的点的渲染方程求出直接光照的值
// 不会进行递归调用，因为只要连线上没有其他物体就百分百打中光源故无需RR（能直接看到面光源还不往上打就离谱，尤其当面光源较小时若不这样做则大量光线都被浪费了）；间接光照部分仍和之前的方法一样，即渲染方程的积分变量仍为入射光立体角且需要RR，唯一的区别是若此时随机的入射光立体角正好是光源方向则应直接返回0，因为它已经在直接光照部分计算过了
// 因为光线追踪一般是在世界空间中进行的，所以无法使用光栅化方法中比较常用的“视锥剔除（Frustum Culling）
// 混合密度方法应该没有传统的阴影射线，这是我个人喜欢的东西，因为除了灯光之外，您还可以采样窗户或门下的明亮裂缝或任何其他您认为很明亮的东西
// 我们的输出一直都是HDR；我们只是被截断了，但可以采用tone mapping；可能需要使用光谱颜色甚至极化；将渲染器从RGB转换为光谱。具有随机波长的每条光线，并且程序中几乎所有的RGB都会变成浮点
