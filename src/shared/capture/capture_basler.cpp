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
  \file    capture_generator.cpp
  \brief   C++ Implementation: CaptureGenerator
  \author  Stefan Zickler, (C) 2009
*/
//========================================================================

#include "capture_basler.h"
#include "conversions.h"


#ifndef VDATA_NO_QT
CaptureBasler::CaptureBasler ( VarList * _settings, int default_camera_id, QObject * parent) : QObject ( parent ), CaptureInterface ( _settings )
#else
CaptureBasler::CaptureBasler ( VarList * _settings, int default_camera_id) : CaptureInterface ( _settings )
#endif
{
  is_capturing=false;

  settings->addChild ( conversion_settings = new VarList ( "Conversion Settings" ) );
  settings->addChild ( capture_settings = new VarList ( "Capture Settings" ) );
  settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));

  //=======================CONVERSION SETTINGS=======================
  conversion_settings->addChild ( v_colorout=new VarStringEnum ( "convert to mode",Colors::colorFormatToString ( COLOR_YUV422_UYVY ) ) );
  v_colorout->addItem ( Colors::colorFormatToString ( COLOR_RGB8 ) );
  v_colorout->addItem ( Colors::colorFormatToString ( COLOR_YUV422_UYVY ) );

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
  capture_settings->addChild(v_colorout         = new VarStringEnum("color mode", Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
  capture_settings->addChild ( v_framerate = new VarDouble ( "Framerate (FPS)", 60.0 ) );
  capture_settings->addChild ( v_width = new VarInt ( "Width (pixels)", 780 ) );
  capture_settings->addChild ( v_height = new VarInt ( "Height (pixels)", 580 ) );
  capture_settings->addChild ( v_test_image = new VarBool ( "Generate Color Test Image", false ) );

  //=======================CAMERA PARAMETERS=======================
  // v_aoi_width = new VarInt("AOI width", 752, 0, 752);
  dcam_parameters->addChild(v_aoi_width = new VarInt("AOI width", 752, 0, 752) );
  dcam_parameters->addChild(v_aoi_height = new VarInt("AOI height", 480, 0, 480) );
  dcam_parameters->addChild(v_aoi_left = new VarInt("AOI left", 0, 0, 752) );
  dcam_parameters->addChild(v_aoi_top = new VarInt("AOI top", 0, 0, 480) );
  dcam_parameters->addChild(v_gain_db = new VarDouble("Gain [dB]", 0.0, 0.0, 12.0) );
  // dcam_parameters->addChild(v_pixel_clock);
  // dcam_parameters->addChild(v_expose_us);
  // dcam_parameters->addChild(v_expose_overlapped);
  // dcam_parameters->addChild(v_hdr_mode);
  // dcam_parameters->addChild(v_mirror_top_down);
  // dcam_parameters->addChild(v_mirror_left_right);
  // dcam_parameters->addChild(v_wb_red);
  // dcam_parameters->addChild(v_wb_green);
  // dcam_parameters->addChild(v_wb_blue);
  // dcam_parameters->addChild(v_sharpen);
  // dcam_parameters->addChild(v_gamma);
  // dcam_parameters->addChild(v_color_twist_mode);

  
  


  
  #ifndef VDATA_NO_QT
    mvc_connect(dcam_parameters);
    mutex.unlock();
  #endif
  
}

#ifndef VDATA_NO_QT
void CaptureBasler::mvc_connect(VarList * group)
{
  vector<VarType *> v=group->getChildren();
  for (unsigned int i=0;i<v.size();i++)
  {
    connect(v[i],SIGNAL(wasEdited(VarType *)),group,SLOT(mvcEditCompleted()));
  }
  connect(group,SIGNAL(wasEdited(VarType *)),this,SLOT(changed(VarType *)));
}

void CaptureBasler::changed(VarType * group)
{
  if (group->getType()==VARTYPE_ID_LIST)
  {
    writeParameterValues( (VarList *)group );
    readParameterValues( (VarList *)group );
  }
}
#endif

void CaptureBasler::readAllParameterValues()
{
  readParameterValues(dcam_parameters);
}

void CaptureBasler::writeAllParameterValues()
{
  writeParameterValues(dcam_parameters);
}

void CaptureBasler::readParameterValues(VarList * item)
{
  if(item != dcam_parameters)
    return;
  
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    
    // TODO: could do a read-out, but why?
//   v_expose_us->setInt(pSettings->expose_us.read());
    
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}
void CaptureBasler::writeParameterValues(VarList * item)
{
  //TODO" writeParameterValues(0)
}

CaptureBasler::~CaptureBasler()
{
}

bool CaptureBasler::stopCapture()
{
  cleanup();
  return true;
}

void CaptureBasler::cleanup()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    // is_capturing=false;
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

bool CaptureBasler::startCapture()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  
      

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    return true;
}

bool CaptureBasler::copyAndConvertFrame ( const RawImage & src, RawImage & target )
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
  //   ColorFormat output_fmt = Colors::stringToColorFormat ( v_colorout->getSelection().c_str() );
  //   ColorFormat src_fmt=src.getColorFormat();

  //   if ( target.getData() ==0 ) {
  //     target.allocate ( output_fmt, src.getWidth(), src.getHeight() );
  //   } else {
  //     target.ensure_allocation ( output_fmt, src.getWidth(), src.getHeight() );
  //   }
  //   target.setTime ( src.getTime() );

  //   if ( output_fmt == src_fmt ) {
  //     if ( src.getData() != 0 ) memcpy ( target.getData(),src.getData(),src.getNumBytes() );
  //   } else if ( src_fmt == COLOR_RGB8 && output_fmt == COLOR_YUV422_UYVY ) {
  //     if ( src.getData() != 0 ) {
  //       dc1394_convert_to_YUV422 ( src.getData(), target.getData(), src.getWidth(), src.getHeight(),
  //                                  DC1394_BYTE_ORDER_UYVY, DC1394_COLOR_CODING_RGB8, 8 );
  //     }
  //   } else {
  //     fprintf ( stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
  //               Colors::colorFormatToString ( src_fmt ).c_str(),
  //               Colors::colorFormatToString ( output_fmt ).c_str() );
  // #ifndef VDATA_NO_QT
  //     mutex.unlock();
  // #endif
  //     return false;
  //   }
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    return true;
}

RawImage CaptureBasler::getFrame()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    // limit.waitForNextFrame();
    // result.setColorFormat ( COLOR_RGB8 );
    // result.setTime ( GetTimeSec() );
    // result.allocate ( COLOR_RGB8,v_width->getInt(),v_height->getInt() );
    // rgbImage img;
    // img.fromRawImage(result);

    // if (v_test_image->getBool()) {
    //   int w = result.getWidth();
    //   int h = result.getHeight();
    //   int n_colors = 8;
    //   int slice_width = w/n_colors;
    //   rgb color=RGB::Black;
    //   rgb color2;
    //   for (int x = 0 ; x < w; x++) {
    //     int c_idx= x/slice_width;
    //     if (c_idx==0) {
    //       color=RGB::White;
    //     } else if (c_idx==1) {
    //       color=RGB::Red;
    //     } else if (c_idx==2) {
    //       color=RGB::Green;
    //     } else if (c_idx==3) {
    //       color=RGB::Blue;
    //     } else if (c_idx==4) {
    //       color=RGB::Cyan;
    //     } else if (c_idx==5) {
    //       color=RGB::Pink;
    //     } else if (c_idx==6) {
    //       color=RGB::Yellow;
    //     } else if (c_idx==7) {
    //       color=RGB::Black;
    //     }
    //     for (int y = 0 ; y < h; y++) {
    //       color2.r= max(0,min(255,((int)color.r * (h-1-y))/h));
    //       color2.g= max(0,min(255,((int)color.g * (h-1-y))/h));
    //       color2.b= max(0,min(255,((int)color.b * (h-1-y))/h));
    //       img.setPixel(x,y,color2);
    //     }
    //   }
    // } else {
    //   img.fillBlack();
    // }


  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    return result;
}

void CaptureBasler::releaseFrame()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
      //TODO release frame here
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

string CaptureBasler::getCaptureMethodName() const
{
  return "Basler";
}
