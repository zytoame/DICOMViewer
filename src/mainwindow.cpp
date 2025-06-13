#include "mainwindow.h"

//#include <QFileDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkImageMapper3D.h> // For vtkImageActor's mapper

#include <vtkOutputWindow.h>
#include <vtkObject.h>

// 轴位、矢状位、冠状位的方向常量
const int AXIAL_ORIENTATION = 2;    // Z-axis slice
const int SAGITTAL_ORIENTATION = 0; // X-axis slice
const int CORONAL_ORIENTATION = 1;  // Y-axis slice


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 禁用所有VTK警告弹出窗口
    vtkOutputWindow::SetGlobalWarningDisplay(0); // 禁用VTK警告弹窗
    initializeVTK();  // 初始化VTK对象
    setupUI();
    setupVTKColorAndOpacity();
    setup3DView();
    setupSliceViews();
    connectSignalsSlots();

    setWindowTitle("DICOM 三维重建与切片查看器");
    resize(1200, 1000);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    // --- 主布局 ---
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    // --- 控制面板 ---
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    openButton = new QPushButton("打开 DICOM 文件夹");
    controlsLayout->addWidget(openButton);

    // 3D 不透明度控制 (示例)
    controlsLayout->addWidget(new QLabel("3D 不透明度:"));
    opacitySlider3D = new QSlider(Qt::Horizontal);
    opacitySlider3D->setRange(0, 100); // 0-1.0 映射到 0-100
    opacitySlider3D->setValue(30); // 默认值
    controlsLayout->addWidget(opacitySlider3D);
    controlsLayout->addStretch(); // 底部添加伸缩

    // --- 渲染窗口 ---
    // 3D 视图
    qvtkWidget3D = new QVTKOpenGLNativeWidget();
    // 切片视图
    qvtkWidgetAxial = new QVTKOpenGLNativeWidget();
    axialLabel = new QLabel("轴状位 (Axial): N/A");
    axialLabel->setFixedHeight(20); // 设置固定高度
    axialLabel->setAlignment(Qt::AlignCenter); // 文字居中
    
    axialSlider = new QSlider(Qt::Horizontal);
    QVBoxLayout* axialLayout = new QVBoxLayout();
    axialLayout->addWidget(axialLabel);
    axialLayout->addWidget(qvtkWidgetAxial);
    axialLayout->addWidget(axialSlider);

    qvtkWidgetSagittal = new QVTKOpenGLNativeWidget();
    sagittalLabel = new QLabel("矢状位 (Sagittal): N/A");
    sagittalLabel->setFixedHeight(20);
    sagittalLabel->setAlignment(Qt::AlignCenter);
    
    sagittalSlider = new QSlider(Qt::Horizontal);
    QVBoxLayout* sagittalLayout = new QVBoxLayout();
    sagittalLayout->addWidget(sagittalLabel);
    sagittalLayout->addWidget(qvtkWidgetSagittal);
    sagittalLayout->addWidget(sagittalSlider);

    qvtkWidgetCoronal = new QVTKOpenGLNativeWidget();
    coronalLabel = new QLabel("冠状位 (Coronal): N/A");
    coronalLabel->setFixedHeight(20);
    coronalLabel->setAlignment(Qt::AlignCenter);
    coronalSlider = new QSlider(Qt::Horizontal);
    QVBoxLayout* coronalLayout = new QVBoxLayout();
    coronalLayout->addWidget(coronalLabel);
    coronalLayout->addWidget(qvtkWidgetCoronal);
    coronalLayout->addWidget(coronalSlider);

    // --- 布局安排 ---
    // 左边控制，右边上部3D，下部三个切片视图
    mainLayout->addLayout(controlsLayout, 0, 0, 2, 1); // 控制面板占两行一列
    mainLayout->addWidget(qvtkWidget3D, 0, 1);       // 3D视图在右上

    QGridLayout *sliceViewsLayout = new QGridLayout();
    sliceViewsLayout->addLayout(axialLayout, 0, 0);
    sliceViewsLayout->addLayout(sagittalLayout, 0, 1);
    sliceViewsLayout->addLayout(coronalLayout, 0, 2);
    mainLayout->addLayout(sliceViewsLayout, 1, 1); // 切片视图在右下

    mainLayout->setColumnStretch(1, 3); // 让渲染区域占据更多空间
    mainLayout->setRowStretch(0, 3);    // 3D视图
    mainLayout->setRowStretch(1, 2);    // 切片视图区域
}

void MainWindow::initializeVTK() {
    try {
    // 初始化VTK智能指针
    dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    loadedImageData = nullptr;

    // 3D渲染相关
    renderer3D = vtkSmartPointer<vtkRenderer>::New();
    volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    volume = vtkSmartPointer<vtkVolume>::New();
    style3D = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    axes3D = vtkSmartPointer<vtkAxesActor>::New();
    orientationMarkerWidget3D = vtkSmartPointer<vtkOrientationMarkerWidget>::New();

    // 轴状面相关
    rendererAxial = vtkSmartPointer<vtkRenderer>::New();
    resliceAxial = vtkSmartPointer<vtkImageReslice>::New();
    wlAxial = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
    actorAxial = vtkSmartPointer<vtkImageActor>::New();
    styleAxial = vtkSmartPointer<vtkInteractorStyleImage>::New();
    axialCam = vtkSmartPointer<vtkCamera>::New();

    // 矢状面相关
    rendererSagittal = vtkSmartPointer<vtkRenderer>::New();
    resliceSagittal = vtkSmartPointer<vtkImageReslice>::New();
    wlSagittal = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
    actorSagittal = vtkSmartPointer<vtkImageActor>::New();
    styleSagittal = vtkSmartPointer<vtkInteractorStyleImage>::New();
    sagittalCam = vtkSmartPointer<vtkCamera>::New();

    // 冠状面相关
    rendererCoronal = vtkSmartPointer<vtkRenderer>::New();
    resliceCoronal = vtkSmartPointer<vtkImageReslice>::New();
    wlCoronal = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
    actorCoronal = vtkSmartPointer<vtkImageActor>::New();
    styleCoronal = vtkSmartPointer<vtkInteractorStyleImage>::New();
    coronalCam = vtkSmartPointer<vtkCamera>::New();

    } catch (std::exception& e) {
        QMessageBox::critical(this, "错误", 
            QString("VTK初始化失败：%1").arg(e.what()));
    }
}

void MainWindow::setupVTKColorAndOpacity() {
    // 为体绘制设置颜色和不透明度传输函数
    // 这是一个非常基础的示例，你可能需要根据图像数据进行调整
    // 通常CT数据，骨骼是高密度值，软组织是中低密度值

    // 不透明度函数 (Opacity Transfer Function - OTF)
    // vtkPiecewiseFunction *opacityTransferFunction = vtkPiecewiseFunction::New();
    opacityTransferFunction->AddPoint(0,    0.0); // 完全透明
    opacityTransferFunction->AddPoint(500,  0.15); // 假设这是软组织开始显现的值
    opacityTransferFunction->AddPoint(1000, 0.3);
    opacityTransferFunction->AddPoint(1150, 0.5);  // 假设这是骨骼开始显现的值
    opacityTransferFunction->AddPoint(2000, 0.8);

    // 颜色传输函数 (Color Transfer Function - CTF)
    // vtkColorTransferFunction *colorTransferFunction = vtkColorTransferFunction::New();
    vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
    colorTransferFunction->AddRGBPoint(0,    colors->GetColor3d("Black").GetRed(), colors->GetColor3d("Black").GetGreen(), colors->GetColor3d("Black").GetBlue());
    colorTransferFunction->AddRGBPoint(500,  colors->GetColor3d("Brown").GetRed() * 0.5, colors->GetColor3d("Brown").GetGreen() * 0.5, colors->GetColor3d("Brown").GetBlue() * 0.5);
    colorTransferFunction->AddRGBPoint(1000, colors->GetColor3d("Ivory").GetRed() * 0.8, colors->GetColor3d("Ivory").GetGreen() * 0.8, colors->GetColor3d("Ivory").GetBlue() * 0.8);
    colorTransferFunction->AddRGBPoint(1150, colors->GetColor3d("White").GetRed(), colors->GetColor3d("White").GetGreen(), colors->GetColor3d("White").GetBlue());

    volumeProperty->SetColor(colorTransferFunction);
    volumeProperty->SetScalarOpacity(opacityTransferFunction);
    volumeProperty->SetInterpolationTypeToLinear(); // 线性插值
    volumeProperty->ShadeOn(); // 开启阴影
    volumeProperty->SetAmbient(0.4);
    volumeProperty->SetDiffuse(0.6);
    volumeProperty->SetSpecular(0.2);
}

void MainWindow::setup3DView() {
    // 设置渲染器背景色等
    renderer3D->SetBackground(0.1, 0.2, 0.3); // 深蓝色背景

    // 将渲染器添加到QVTK部件的渲染窗口
    qvtkWidget3D->renderWindow()->AddRenderer(renderer3D);

    // 设置交互方式
    qvtkWidget3D->renderWindow()->GetInteractor()->SetInteractorStyle(style3D);

    // 设置体绘制映射器
    volumeMapper->SetBlendModeToComposite(); // 混合模式
    volumeMapper->SetInputConnection(dicomReader->GetOutputPort()); // 数据源在打开文件后设置

    // 设置体素
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    // 添加坐标轴指示器
    orientationMarkerWidget3D->SetOutlineColor(0.9300, 0.5700, 0.1300);
    orientationMarkerWidget3D->SetOrientationMarker(axes3D);
    orientationMarkerWidget3D->SetInteractor(qvtkWidget3D->renderWindow()->GetInteractor());
    orientationMarkerWidget3D->SetViewport(0.0, 0.0, 0.2, 0.2); // 右下角，20%大小
    orientationMarkerWidget3D->SetEnabled(1);
    orientationMarkerWidget3D->InteractiveOff();


    // 初始时不添加 volume，等待数据加载
    // renderer3D->AddVolume(volume);
}

void MainWindow::setupSliceViews() {
    // --- Axial View 轴向面 ---
    rendererAxial->SetBackground(0.1, 0.2, 0.2);
    qvtkWidgetAxial->renderWindow()->AddRenderer(rendererAxial);// 添加渲染器到QVTK部件

    // 设置轴向视图相机
    axialCam = rendererAxial->GetActiveCamera();
    axialCam->ParallelProjectionOn();  // 使用平行投影
    rendererAxial->SetActiveCamera(axialCam);

    setupReslice(resliceAxial, AXIAL_ORIENTATION);// 设置轴状视图的切片重采样
    wlAxial->SetInputConnection(resliceAxial->GetOutputPort());
    actorAxial->GetMapper()->SetInputConnection(wlAxial->GetOutputPort()); // vtkImageActor内部有自己的mapper
    rendererAxial->AddActor(actorAxial);
    qvtkWidgetAxial->renderWindow()->GetInteractor()->SetInteractorStyle(styleAxial);

    updateSliceViewport(rendererAxial, actorAxial); 

    // --- Sagittal View 矢状面 ---
    rendererSagittal->SetBackground(0.1, 0.2, 0.2);
    qvtkWidgetSagittal->renderWindow()->AddRenderer(rendererSagittal);

    // 设置矢状视图相机
    sagittalCam = rendererSagittal->GetActiveCamera();
    sagittalCam->ParallelProjectionOn();
    sagittalCam->Zoom(2.0); // 放大视图无用
    rendererSagittal->SetActiveCamera(sagittalCam);

    setupReslice(resliceSagittal, SAGITTAL_ORIENTATION);
    wlSagittal->SetInputConnection(resliceSagittal->GetOutputPort());
    actorSagittal->GetMapper()->SetInputConnection(wlSagittal->GetOutputPort());
    rendererSagittal->AddActor(actorSagittal);
    qvtkWidgetSagittal->renderWindow()->GetInteractor()->SetInteractorStyle(styleSagittal);
    

    // --- Coronal View 冠状面 ---
    rendererCoronal->SetBackground(0.1, 0.2, 0.2);
    qvtkWidgetCoronal->renderWindow()->AddRenderer(rendererCoronal);

    // 设置冠状视图相机
    coronalCam = rendererCoronal->GetActiveCamera();
    coronalCam->ParallelProjectionOn();
    rendererCoronal->SetActiveCamera(coronalCam);

    setupReslice(resliceCoronal, CORONAL_ORIENTATION);
    wlCoronal->SetInputConnection(resliceCoronal->GetOutputPort());
    actorCoronal->GetMapper()->SetInputConnection(wlCoronal->GetOutputPort());
    rendererCoronal->AddActor(actorCoronal);
    qvtkWidgetCoronal->renderWindow()->GetInteractor()->SetInteractorStyle(styleCoronal);
}

void MainWindow::setupReslice(vtkSmartPointer<vtkImageReslice> reslice, int orientation) {
    reslice->SetInputConnection(dicomReader->GetOutputPort()); // 数据加载后设置
    reslice->SetOutputDimensionality(2); // 输出二维图像
    reslice->SetInterpolationModeToLinear(); // 线性插值

    // 根据方向设置ResliceAxes - 默认是单位矩阵 (轴状面)
    // ResliceAxes 是一个4x4的变换矩阵.
    // 前三列定义了切片平面的X, Y, Z轴在输入体数据坐标系中的方向。
    // 第四列定义了切片平面的原点。
    // 这里我们仅旋转坐标轴，原点通过 SetSliceOrigin 或类似方法控制。

    vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->Identity(); // Start with identity

    switch (orientation) {
        case AXIAL_ORIENTATION: // Z-slice (X-Y plane) 默认矩阵就是这个方向: X = (1,0,0), Y = (0,1,0), Z = (0,0,1)
            break;
        case SAGITTAL_ORIENTATION: // X-slice (Y-Z plane)
         {
            double sagMatElements[16] = {
                0, 0, 1, 0,
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 0, 1
            };
            matrix->DeepCopy(sagMatElements);
         }
         break;
        case CORONAL_ORIENTATION: // Y-slice (X-Z plane)
        {
            // Slice X-axis maps to Volume X-axis
            // Slice Y-axis maps to Volume Z-axis
            // Slice Z-axis maps to Volume Y-axis (normal to slice)
            double corMatElements[16] = {
                1, 0, 0, 0,
                0, 0, 1, 0,
                0, 1, 0, 0,
                0, 0, 0, 1
            };
            matrix->DeepCopy(corMatElements);
            
            }
            break;
    }
    reslice->SetResliceAxes(matrix);
}

void MainWindow::connectSignalsSlots() {
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openDICOMFolder);
    connect(opacitySlider3D, &QSlider::valueChanged, this, &MainWindow::update3DOpacity);

    connect(axialSlider, &QSlider::valueChanged, this, &MainWindow::updateAxialSlice);
    connect(sagittalSlider, &QSlider::valueChanged, this, &MainWindow::updateSagittalSlice);
    connect(coronalSlider, &QSlider::valueChanged, this, &MainWindow::updateCoronalSlice);
}

void MainWindow::openDICOMFolder() {
    //QMessageBox::information(this, "调试信息", "已点击加载DICOM文件按钮！");
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "选择包含DICOM序列的文件夹",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly
    );
    
    if (dirPath.isEmpty()) {
        return;
    }

    try {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        // 清理之前的数据
        if (loadedImageData) {
            loadedImageData = nullptr;
        }
        
        // 配置和更新DICOM读取器
        dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
        dicomReader->SetDirectoryName(dirPath.toStdString().c_str());
        dicomReader->Update();

        loadedImageData = dicomReader->GetOutput();
        if (!loadedImageData || loadedImageData->GetScalarType() == VTK_VOID) {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this, "错误", "无法读取DICOM数据或数据为空!");
            return;
        }

        // 输出数据信息
        int* dimensions = loadedImageData->GetDimensions();
        //qDebug() << "DICOM序列维度:" << dimensions[0] << "x" << dimensions[1] << "x" << dimensions[2];

        // 更新体绘制管线
        volumeMapper->SetInputConnection(dicomReader->GetOutputPort());
        volume->SetMapper(volumeMapper);
        if (renderer3D->GetVolumes()->GetNumberOfItems() == 0) {
            renderer3D->AddVolume(volume);
        }

        // 更新切片视图管线
        resliceAxial->SetInputConnection(dicomReader->GetOutputPort());
        resliceSagittal->SetInputConnection(dicomReader->GetOutputPort());
        resliceCoronal->SetInputConnection(dicomReader->GetOutputPort());

        // 更新切片范围
        updateSliceLimits();

        // 设置窗宽窗位
        double range[2];
        loadedImageData->GetScalarRange(range);
        double window = range[1] - range[0];
        double level = (range[1] + range[0]) / 2.0;

        wlAxial->SetWindow(window);
        wlAxial->SetLevel(level);
        wlSagittal->SetWindow(window);
        wlSagittal->SetLevel(level);
        wlCoronal->SetWindow(window);
        wlCoronal->SetLevel(level);

        // 更新所有视图，感觉有没有都不影响
        updateAxialSlice(axialSlider->value());
        updateSliceViewport(rendererAxial, actorAxial);
        //updateSagittalSlice(sagittalSlider->value());
        //updateSliceViewport(rendererSagittal, actorSagittal);
        updateCoronalSlice(coronalSlider->value());

        // 重置并应用相机设置
        rendererAxial->ResetCamera();
        rendererSagittal->ResetCamera();
        rendererCoronal->ResetCamera();

        // 重置相机视角
        renderer3D->ResetCamera();
        renderer3D->GetActiveCamera()->Zoom(1.5);

        // 刷新所有渲染窗口
        qvtkWidget3D->renderWindow()->Render();
        qvtkWidgetAxial->renderWindow()->Render();
        qvtkWidgetSagittal->renderWindow()->Render();
        qvtkWidgetCoronal->renderWindow()->Render();

        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, "成功", 
            QString("DICOM序列加载完成!\n图像大小: %1x%2x%3")
            .arg(dimensions[0])
            .arg(dimensions[1])
            .arg(dimensions[2]));

    } catch (std::exception& e) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, "错误", 
            QString("加载DICOM序列时发生错误：%1").arg(e.what()));
    }
}

void MainWindow::updateSliceLimits() {
    if (!loadedImageData) return;

    int* dims = loadedImageData->GetDimensions(); // (nx, ny, nz)

    axialSliceMin = 0;
    axialSliceMax = dims[2] - 1;
    axialSlider->setRange(axialSliceMin, axialSliceMax);
    axialSlider->setValue((axialSliceMin + axialSliceMax) / 2);

    sagittalSliceMin = 0;
    sagittalSliceMax = dims[0] - 1;
    sagittalSlider->setRange(sagittalSliceMin, sagittalSliceMax);
    sagittalSlider->setValue((sagittalSliceMin + sagittalSliceMax) / 2);

    coronalSliceMin = 0;
    coronalSliceMax = dims[1] - 1;
    coronalSlider->setRange(coronalSliceMin, coronalSliceMax);
    coronalSlider->setValue((coronalSliceMin + coronalSliceMax) / 2);
}

void MainWindow::updateSliceActor(vtkImageActor* actor, vtkImageReslice* reslice, int slice, int orientation) {
    if (!loadedImageData) return;

    double spacing[3];
    loadedImageData->GetSpacing(spacing);
    double origin[3];
    loadedImageData->GetOrigin(origin);

    // 计算切片原点，这取决于vtkImageReslice的SetResliceAxes设置
    // ResliceAxes的第四列是切片原点在输入体数据坐标系中的位置
    // 如果ResliceAxes是单位阵，那么 (0,0,slice_z_coord) 就是切片中心
    // 为了简化，我们假设ResliceAxes只做了旋转，平移部分通过SetOutputOrigin或调整输入原点
    // 更通用的方法是直接修改 ResliceAxes 的平移部分

    vtkSmartPointer<vtkMatrix4x4> currentAxes = reslice->GetResliceAxes();
    if (!currentAxes) { // 确保矩阵存在
        currentAxes = vtkSmartPointer<vtkMatrix4x4>::New();
        currentAxes->Identity(); // 如果不存在，则初始化为单位矩阵
        reslice->SetResliceAxes(currentAxes);
    }

    double slicePosition = 0.0;
    switch (orientation) {
        case AXIAL_ORIENTATION:    // Z
            slicePosition = origin[2] + spacing[2] * slice;
            currentAxes->SetElement(2, 3, slicePosition); // Z-origin
            break;
        case SAGITTAL_ORIENTATION: // X
            slicePosition = origin[0] + spacing[0] * slice;
            currentAxes->SetElement(0, 3, slicePosition); // X-origin
            break;
        case CORONAL_ORIENTATION:  // Y
            slicePosition = origin[1] + spacing[1] * slice;
            currentAxes->SetElement(1, 3, slicePosition); // Y-origin
            break;
    }
    reslice->SetResliceAxes(currentAxes); // 应用更新后的矩阵
    reslice->Update(); // 非常重要：确保reslice更新

    // actor->GetMapper()->Update(); // actor的mapper会自动更新，但有时显式调用有帮助
}

void MainWindow::update3DOpacity(int value) {
    if (!loadedImageData) return;
    
    // 将滑块值(0-100)转换为不透明度因子(0.0-1.0)
    double newOpacityFactor = value / 100.0;
    
    // 重新设置不透明度传输函数
    opacityTransferFunction->RemoveAllPoints();
    opacityTransferFunction->AddPoint(0,    0.0);
    opacityTransferFunction->AddPoint(500,  0.15 * newOpacityFactor);
    opacityTransferFunction->AddPoint(1000, 0.3 * newOpacityFactor);
    opacityTransferFunction->AddPoint(1150, 0.5 * newOpacityFactor);
    opacityTransferFunction->AddPoint(2000, 0.8 * newOpacityFactor);

    volumeProperty->SetScalarOpacity(opacityTransferFunction);
    qvtkWidget3D->renderWindow()->Render();
}

void MainWindow::updateAxialSlice(int slice) {
    currentAxialSlice = slice;
    axialLabel->setText(QString("轴状位 (Axial): %1/%2").arg(slice).arg(axialSliceMax));
    if (loadedImageData) {
        updateSliceActor(actorAxial, resliceAxial, slice, AXIAL_ORIENTATION);
        //updateSliceViewport(rendererAxial, actorAxial);
        qvtkWidgetAxial->renderWindow()->Render();
    }
}

void MainWindow::updateSagittalSlice(int slice) {
    currentSagittalSlice = slice; 
    sagittalLabel->setText(QString("矢状位 (Sagittal): %1/%2").arg(slice).arg(sagittalSliceMax));
    if (loadedImageData) {
        updateSliceActor(actorSagittal, resliceSagittal, slice, SAGITTAL_ORIENTATION);
        //updateSliceViewport(rendererSagittal, actorSagittal);
        qvtkWidgetSagittal->renderWindow()->Render();
    }
}

void MainWindow::updateCoronalSlice(int slice) {
    currentCoronalSlice = slice;
    coronalLabel->setText(QString("冠状位 (Coronal): %1/%2").arg(slice).arg(coronalSliceMax));
    if (loadedImageData) {
        updateSliceActor(actorCoronal, resliceCoronal, slice, CORONAL_ORIENTATION);
        //updateSliceViewport(rendererCoronal, actorCoronal);
        qvtkWidgetCoronal->renderWindow()->Render();
    }
}

//实现切片自适应大小，，，未实现
void MainWindow::updateSliceViewport(vtkRenderer* renderer, vtkImageActor* actor) {
    if (!actor || !renderer) return;

    // 获取渲染窗口大小
    int* windowSize = renderer->GetRenderWindow()->GetSize();
    double windowAspect = static_cast<double>(windowSize[0]) / windowSize[1];

    // 获取图像尺寸
    int* dimensions = actor->GetInput()->GetDimensions();
    double imageAspect = static_cast<double>(dimensions[0]) / dimensions[1];

    // 调整相机参数以适应窗口
    vtkCamera* camera = renderer->GetActiveCamera();
    if (camera) {
        camera->ParallelProjectionOn();
        
        // 根据窗口和图像的宽高比调整缩放
        if (windowAspect >= imageAspect) {
            camera->SetParallelScale(dimensions[1] * 0.5);
        } else {
            camera->SetParallelScale(dimensions[0] * 0.5 / windowAspect);
        }
        
        // 调整相机位置使图像居中
        camera->SetFocalPoint(dimensions[0] * 0.5, dimensions[1] * 0.5, 0);
        camera->SetPosition(dimensions[0] * 0.5, dimensions[1] * 0.5, 1);
        camera->SetViewUp(0, 1, 0);
    }
    
}

//未实现
void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    
    if (loadedImageData) {
        // 更新所有切片视图
        updateSliceViewport(rendererAxial, actorAxial);
        updateSliceViewport(rendererSagittal, actorSagittal);
        updateSliceViewport(rendererCoronal, actorCoronal);
        
        // 刷新渲染
        qvtkWidgetAxial->renderWindow()->Render();
        qvtkWidgetSagittal->renderWindow()->Render();
        qvtkWidgetCoronal->renderWindow()->Render();
    }
}