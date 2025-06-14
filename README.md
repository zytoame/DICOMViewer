# DICOM 3D Reconstruction and Slice Viewer

## Project Overview

This project is a medical imaging visualization software built on Qt5 and VTK, focusing on 3D reconstruction and multi-planar slice viewing of DICOM format medical images. The software provides an intuitive user interface and efficient image processing capabilities.

## Architecture Design

### Core Modules
- **DICOM Reader**: Uses vtkDICOMImageReader to load DICOM series
- **3D Reconstruction Engine**: Volume rendering pipeline based on vtkSmartVolumeMapper
- **Multi-Planar Reconstruction**: Implements axial, sagittal and coronal slices through vtkImageReslice

### View Modules
- **3D View**: Interactive volume rendering view
- **Slice Views**:
  - Axial view
  - Sagittal view
  - Coronal view

### Interaction Controls
- Slice navigation sliders
- 3D opacity adjustment
- View reset functionality

## Key Features

- **DICOM Series Loading**: Supports reading and displaying complete DICOM series
- **Volume Rendering**: Adjustable transparency volume visualization
- **Multi-Planar Slices**: Synchronized display of three orthogonal plane slices
- **Interactive Controls**:
  - Independent slice navigation for each plane
  - 3D view rotation/pan/zoom
  - Automatic window level setting
- **Orientation Indicator**: 3D axes for spatial reference

## Dependencies

### Required Libraries
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

### Compiler Requirements
- **C++17 compatible compiler**
- **CMake 3.16+**

## Build Instructions

### Prerequisites
1. Install Qt 5.15+
2. Install VTK 9.0+ (with Qt support)
3. Install CMake 3.16+

### Build Steps
```bash
git clone https://github.com/yourusername/dicom-viewer.git
cd dicom-viewer
mkdir build
cd build
cmake ..
make
```

## Project Structure

```
dicom-viewer/
├── CMakeLists.txt          # Main CMake configuration
├── main.cpp               # Program entry
├── mainwindow.h/cpp       # Main window implementation
├── README.md              # Project documentation
└── resources/             # Resource files
    └── icons/             # Icon resources
```

## Usage Guide

1. Click "Open DICOM Folder" button to load DICOM series
2. Use sliders to navigate through different slices
3. Adjust 3D opacity slider to change volume rendering effect
4. Mouse interactions:
   - Left drag to rotate 3D view
   - Right drag to pan view
   - Scroll wheel to zoom

## Development Guide

### Code Structure
- **MainWindow**: Main interface and core logic
- **VTK Pipeline**: Encapsulated in initializeVTK() and setup*View() functions
- **Signal-Slot**: Connected via connectSignalsSlots()

### Extension Suggestions
1. Add more DICOM metadata display
2. Implement measurement tools
3. Add image processing filters
4. Support more medical image formats

## Technical Highlights

- **Efficient VTK Pipeline**: Optimized rendering performance
- **Responsive UI**: Qt signal-slot mechanism enables smooth interaction
- **Modular Architecture**: Easy to extend functionality
- **Cross-Platform Support**: Powered by Qt and VTK's cross-platform capabilities

## License

MIT License

## Contributing

Issues and Pull Requests are welcome

## Contact

[Your email/project homepage]

---

*Last updated: June 14, 2025*  
*Current version: 1.0.0*
