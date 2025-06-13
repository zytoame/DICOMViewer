#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QApplication>

// 前向声明 Qt UI 类 (如果使用 Qt Designer 生成 .ui 文件)
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// VTK Headers
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h> // 或者 vtkGPUVolumeRayCastMapper
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAxesActor.h> // 用于显示坐标轴
#include <vtkOrientationMarkerWidget.h> // 用于显示坐标轴
#include <vtkRayCastImageDisplayHelper.h>

#include <QVTKOpenGLNativeWidget.h>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openDICOMFolder();
    void updateAxialSlice(int slice);
    void updateSagittalSlice(int slice);
    void updateCoronalSlice(int slice);
    void update3DOpacity(int value); // 示例：调整3D体渲染不透明度

private:

    // --- Qt UI 元素
    QPushButton *openButton;
    QPushButton *loadButton;
    QVTKOpenGLNativeWidget *qvtkWidget3D;
    QVTKOpenGLNativeWidget *qvtkWidgetAxial;
    QVTKOpenGLNativeWidget *qvtkWidgetSagittal;
    QVTKOpenGLNativeWidget *qvtkWidgetCoronal;

    QSlider *axialSlider;
    QSlider *sagittalSlider;
    QSlider *coronalSlider;
    QLabel *axialLabel;
    QLabel *sagittalLabel;
    QLabel *coronalLabel;

    QSlider *opacitySlider3D; // 示例

    // --- VTK 组件 ---
    vtkSmartPointer<vtkDICOMImageReader> dicomReader;
    vtkImageData* loadedImageData; // 保存读取的图像数据指针

    // 3D 视图
    vtkSmartPointer<vtkRenderer> renderer3D;
    vtkSmartPointer<vtkVolume> volume;
    vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper;
    vtkSmartPointer<vtkVolumeProperty> volumeProperty;
    vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction;
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style3D;
    vtkSmartPointer<vtkAxesActor> axes3D; // 可选的坐标轴
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarkerWidget3D; // 可选的坐标轴指示器

    // Axial (轴状) 切片视图
    vtkSmartPointer<vtkRenderer> rendererAxial;
    vtkSmartPointer<vtkImageReslice> resliceAxial;
    vtkSmartPointer<vtkImageMapToWindowLevelColors> wlAxial;
    vtkSmartPointer<vtkImageActor> actorAxial;
    vtkSmartPointer<vtkInteractorStyleImage> styleAxial;
    vtkSmartPointer<vtkCamera> axialCam; // 用于轴状视图的相机
    int axialSliceMin, axialSliceMax, currentAxialSlice;

    // Sagittal (矢状) 切片视图
    vtkSmartPointer<vtkRenderer> rendererSagittal;
    vtkSmartPointer<vtkImageReslice> resliceSagittal;
    vtkSmartPointer<vtkImageMapToWindowLevelColors> wlSagittal;
    vtkSmartPointer<vtkImageActor> actorSagittal;
    vtkSmartPointer<vtkInteractorStyleImage> styleSagittal;
    vtkSmartPointer<vtkCamera> sagittalCam; // 用于矢状视图的相机
    int sagittalSliceMin, sagittalSliceMax, currentSagittalSlice;

    // Coronal (冠状) 切片视图
    vtkSmartPointer<vtkRenderer> rendererCoronal;
    vtkSmartPointer<vtkImageReslice> resliceCoronal;
    vtkSmartPointer<vtkImageMapToWindowLevelColors> wlCoronal;
    vtkSmartPointer<vtkImageActor> actorCoronal;
    vtkSmartPointer<vtkInteractorStyleImage> styleCoronal;
    vtkSmartPointer<vtkCamera> coronalCam; // 用于冠状视图的相机
    int coronalSliceMin, coronalSliceMax, currentCoronalSlice;

    // --- 初始化函数 ---
    void setupUI();         // 设置 Qt 界面布局
    void initializeVTK();  // 添加VTK初始化函数声明
    void setupVTKColorAndOpacity(); // 设置颜色和不透明度函数
    void setup3DView();
    void setupSliceViews(); // 一个统一的函数来设置所有切片视图
    void connectSignalsSlots(); // 连接信号和槽

    void updateSliceLimits(); // 读取DICOM后更新切片范围
    void updateSliceActor(vtkImageActor* actor, vtkImageReslice* reslice, int slice, int orientation);
    void setupReslice(vtkSmartPointer<vtkImageReslice> reslice, int orientation);
    void updateSliceViewport(vtkRenderer* renderer, vtkImageActor* actor);// 更新切片视图的显示范围

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // MAINWINDOW_H