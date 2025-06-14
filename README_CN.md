# DICOM三维重建与切片查看器

## 项目概述

本项目是基于Qt5和VTK构建的医学影像可视化软件，专注于DICOM格式医学影像的三维重建和多平面切片查看功能。该软件提供了直观的用户界面和高效的图像处理能力。

## 架构设计

### 核心模块
- **DICOM读取器**：使用vtkDICOMImageReader加载DICOM序列
- **三维重建引擎**：基于vtkSmartVolumeMapper的体绘制管线
- **多平面重建**：通过vtkImageReslice实现轴状位、矢状位和冠状位切片

### 视图模块
- **3D视图**：交互式三维体绘制视图
- **切片视图**：
  - 轴状位视图(Axial)
  - 矢状位视图(Sagittal) 
  - 冠状位视图(Coronal)

### 交互控制
- 切片导航滑块
- 3D透明度调节
- 视图重置功能

## 功能特性

- **DICOM序列加载**：支持完整DICOM序列的读取和显示
- **三维体绘制**：可调节透明度的体绘制可视化
- **多平面切片**：同步显示三个正交平面的切片
- **交互控制**：
  - 各平面独立切片导航
  - 3D视图旋转/平移/缩放
  - 窗宽窗位自动设置
- **方向指示**：3D坐标轴辅助定位

## 依赖项

### 必需库
- **Qt 5.15+**
  - QtCore
  - QtWidgets
  - QtGui
  - QVTKOpenGLNativeWidget

- **VTK 9.0+**
  - vtkDICOMImageReader
  - vtkSmartVolumeMapper
  - vtkImageReslice
  - vtkImageMapToWindowLevelColors

### 编译器要求
- **支持C++17的编译器**
- **CMake 3.16+**

## 构建说明

### 前提条件
1. 安装Qt 5.15+
2. 安装VTK 9.0+(带Qt支持)
3. 安装CMake 3.16+

### 构建步骤
```bash
git clone https://github.com/yourusername/dicom-viewer.git
cd dicom-viewer
mkdir build
cd build
cmake ..
make
```

## 项目结构

```
dicom-viewer/
├── CMakeLists.txt         # 主CMake配置
├── README.md              # 英文文档
├── README_CN.md           # 中文文档（本文件）
├── build/                 # 构建输出目录
├── src/                   # 源代码目录
    ├── main.cpp               # 程序入口
    └── mainwindow.h/cpp       # 主窗口实现
```

## 使用说明

1. 点击"打开DICOM文件夹"按钮加载DICOM序列
2. 使用滑动条浏览不同切片
3. 调节3D透明度滑块改变体绘制效果
4. 鼠标交互：
   - 左键拖动旋转3D视图,调节切片曝光度
   - 右键拖动平移视图
   - 滚轮缩放

## 开发指南

### 代码结构
- **MainWindow**：主界面和核心逻辑
- **VTK管线**：封装在initializeVTK()和setup*View()函数中
- **信号槽**：通过connectSignalsSlots()连接

### 扩展建议
1. 添加更多DICOM元数据显示
2. 实现测量工具
3. 增加图像处理滤波器
4. 支持更多医学图像格式

## 技术特点

- **高效的VTK管线设计**：优化渲染性能
- **响应式UI**：Qt信号槽机制实现流畅交互
- **模块化架构**：便于功能扩展
- **跨平台支持**：基于Qt和VTK的跨平台能力

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request

## 联系方式

[邮箱/项目主页]

---

*最后更新：2025年6月14日*  
*当前版本：1.0.0*
