//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    capture_generator.h
  \brief   C++ Interface: CaptureGenerator
  \author  Stefan Zickler, (C) 2009
*/
//========================================================================

#ifndef CAPTUREBASLER_H
#define CAPTUREBASLER_H

#include "captureinterface.h"
#include <string>
#include "VarTypes.h"
#include "framecounter.h"
#include "framelimiter.h"
#include "image.h"
#include <limits>
#ifndef VDATA_NO_QT
  #include <QMutex>
#else
  #include <pthread.h>
#endif
// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
// Namespace for using pylon objects.
// #include "Frame.h"
// #include "TimeDate.h"
// #include "Conversion.h"
// #include "SaveImg.h"
// #include "Camera.h"
// #include "ECamPixFmt.h"
// #include "EParser.h"
// #include <boost/log/common.hpp>
// #include <boost/log/attributes.hpp>
// #include <boost/log/sources/logger.hpp>
// #include <boost/log/core.hpp>
// #include "ELogSeverityLevel.h"

// #include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <pylon/gige/BaslerGigECamera.h>
#include <pylon/PylonImage.h>
#include <pylon/Pixel.h>
#include <pylon/ImageFormatConverter.h>

using namespace Pylon;
using namespace GenApi;
// using namespace cv;
using namespace std;
using namespace Basler_GigECameraParams;

static const uint32_t nbBuffers = 20;



#ifndef VDATA_NO_QT
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureBasler : public QObject, public CaptureInterface
#else
class CaptureBasler : public CaptureInterface
#endif
{
  #ifndef VDATA_NO_QT
    Q_OBJECT
     public slots: 
     void changed(VarType * group); 
    protected:
    QMutex mutex;
    public:
  #endif

  protected:
    bool is_capturing;
    RawImage result;
    FrameLimiter limit;
    //processing variables:
    VarStringEnum * v_colorout;

    VarList * capture_settings;
    VarList * conversion_settings;

    VarInt * v_width;
    VarInt * v_height;
    VarDouble * v_framerate;
    VarBool * v_test_image;

    VarInt* v_expose_us;
    VarBool* v_expose_overlapped;
    VarDouble* v_gain_db;
    VarStringEnum* v_hdr_mode;
    VarBool* v_mirror_top_down;
    VarBool* v_mirror_left_right;
    VarDouble* v_wb_red;
    VarDouble* v_wb_green;
    VarDouble* v_wb_blue;
    VarBool* v_sharpen;
    VarDouble* v_gamma;
    VarStringEnum* v_color_twist_mode;
    VarInt    * v_aoi_width;
    VarInt    * v_aoi_height;
    VarInt    * v_aoi_left;
    VarInt    * v_aoi_top;
    VarStringEnum* v_pixel_clock;
    
    //capture variables:
    VarInt    * v_cam_bus;
    // VarStringEnum * v_colorout;
    
    VarList * dcam_parameters;
    // VarList * capture_settings;

    // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
    // is initialized during the lifetime of this object.
    Pylon::PylonAutoInitTerm                autoInitTerm;
    CPylonImage targetImage;
  
    CImageFormatConverter converter;

    uint8_t*                                ppBuffersUC[nbBuffers];         // Buffer for the grabbed images in 8 bits format.
    uint16_t*                               ppBuffersUS[nbBuffers];         // Buffer for the grabbed images in 16 bits format.
    StreamBufferHandle                      handles[nbBuffers];
    CTlFactory                              *pTlFactory;
    ITransportLayer                         *pTl;                    // Pointer on the transport layer.
    CInstantCamera                          *pCamera;                       // Pointer on basler camera.
    CBaslerGigECamera::StreamGrabber_t      *pStreamGrabber;
    DeviceInfoList_t                        devices;
    IPylonDevice                            *pDevice;
    CGrabResultPtr                          ptrGrabResult;
    void*                                   ptrCvt;
    // //GrabResult                              result;
    bool                                    connectionStatus;
    int                                     mFrameCounter;
    //Camera.h
    bool                mExposureAvailable;
    bool                mGainAvailable;
    bool                mCamSizeToMax;
    int                 mCamSizeWidth;
    int                 mCamSizeHeight;
    bool                mVerbose;

    INodeMap              *pControl;// = camera.GetNodeMap();
    // Get the parameters for setting the image area of interest (Image AOI).
    CIntegerPtr           width;// = control.GetNode("Width");
    CIntegerPtr           height;// = control.GetNode("Height");
    CIntegerPtr           offsetX;// = control.GetNode("OffsetX");
    CIntegerPtr           offsetY;// = control.GetNode("OffsetY");
    CEnumerationPtr       pixelFormat;
    CEnumerationPtr       gainAuto;

    unsigned int cam_id;
    ColorFormat capture_format;
public:
  #ifndef VDATA_NO_QT
    CaptureBasler(VarList * _settings, int default_camera_id=0, QObject * parent=0);
    void mvc_connect(VarList * group);
  #else
    CaptureBasler(VarList * _settings, int default_camera_id=0);
  #endif
  ~CaptureBasler();
    
  virtual bool startCapture();
  virtual bool stopCapture();
  virtual bool isCapturing() { return is_capturing; };
  
  virtual RawImage getFrame();
  virtual void releaseFrame();
   
  void cleanup();
  virtual bool resetBus();

  void readParameterValues(VarList * item);

  void writeParameterValues(VarList * item);

  virtual void readAllParameterValues();

  void writeAllParameterValues();

  virtual bool copyAndConvertFrame(const RawImage & src, RawImage & target);
  virtual string getCaptureMethodName() const;
};

#endif
