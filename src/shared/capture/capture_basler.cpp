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
  printf("^^^^^^^^^^^^^^^^^^^^^\n");
  try{
    converter.OutputPixelFormat =  PixelType_RGB8packed;
  }
 catch (const GenericException &e)
  {
      // Error handling.
      cerr << "An exception occurred." << endl << e.GetDescription() << endl;
      // exitCode = 1;
  }

  
  printf("Enter: CaptureBasler\n");
  is_capturing=false;

  settings->addChild ( conversion_settings = new VarList ( "Conversion Settings" ) );
  settings->addChild ( capture_settings = new VarList ( "Capture Settings" ) );
  settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));

  conversion_settings->addChild(v_colorout=new VarStringEnum("convert to mode",Colors::colorFormatToString(COLOR_RGBA8)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
  // v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));

  //=======================CONVERSION SETTINGS=======================
  // conversion_settings->addChild ( v_colorout=new VarStringEnum ( "convert to mode",Colors::colorFormatToString ( COLOR_YUV422_UYVY ) ) );
  // v_colorout->addItem ( Colors::colorFormatToString ( COLOR_RGB8 ) );
  // v_colorout->addItem ( Colors::colorFormatToString ( COLOR_YUV422_UYVY ) );

  //=======================CAPTURE SETTINGS==========================
  // capture_settings->addChild(v_colormode        = new VarStringEnum("capture mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB8));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB16));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW8));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW16));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO8));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO16));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV411));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
  // v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV444));
  capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
  capture_settings->addChild(v_colorout         = new VarStringEnum("color mode", Colors::colorFormatToString(COLOR_MONO8)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_MONO8));
  // v_colorout->addItem(Colors::colorFormatToString(CO));
  capture_settings->removeChild(v_colorout);
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
  // Pylon::PylonAutoInitTerm                autoInitTerm;
  //Camera
  // PylonInitialize();
  pCamera = NULL;
  pStreamGrabber = NULL;
  connectionStatus = false;
  mFrameCounter = 0;
  mExposureAvailable = true;
  mGainAvailable = true;

  // //Enumerate GigE cameras
  // pTlFactory = &CTlFactory::GetInstance();
  // pTl = pTlFactory->CreateTl(CBaslerGigECamera ::DeviceClass());
  // pTl->EnumerateDevices(devices);
  //~Camera  
  


  
  #ifndef VDATA_NO_QT
    mvc_connect(dcam_parameters);
    mutex.unlock();
  #endif
  
}

  // CaptureBasler::getCamerasList() {

  //   vector<pair<int,string>> camerasList;

  //   try {

  //       int id = 0;
  //       if(!devices.empty()) {
  //           DeviceInfoList_t::const_iterator it;
  //           for(it = devices.begin(); it != devices.end(); ++it ) {
  //               if(!devices.empty()){
  //                   if(devices[id].GetFullName().find_first_of("Basler")==0||devices[id].GetFullName().find_first_of("Prosilica")==0) {
  //                       pair<int,string> c;
  //                       c.first = id;
  //                       c.second = "NAME[" + devices[id].GetModelName() + "] S/N[" + devices[id].GetSerialNumber() + "] SDK[PYLON]";
  //                       camerasList.push_back(c);
  //                   }
  //               }
  //               id++;
  //           }
  //       }

  //   }catch (GenICam::GenericException &e){

  //       // BOOST_LOG_SEV(logger,fail) << "An exception occured : " << e.GetDescription() ;
  //       cout << "An exception occured : " << e.GetDescription() << endl;

  //   }

  //   return camerasList;

  // }

  // CaptureBasler::listCameras() {

  //     try {

  //         cout << endl << "------------ GIGE CAMERAS WITH PYLON -----------" << endl << endl;

  //         int id = 0;
  //         DeviceInfoList_t::const_iterator it;

  //         for(it = devices.begin(); it != devices.end(); ++it ) {
  //             if(!devices.empty()){
  //                 if(devices[id].GetFullName().find_first_of("Basler")==0||devices[id].GetFullName().find_first_of("Prosilica")==0) {
  //                     cout << "-> ID[" << id << "]  NAME[" << devices[id].GetModelName().c_str() << "]  S/N[" << devices[id].GetSerialNumber().c_str() <<"]"<< endl;
  //                 }
  //             }
  //             id++;
  //         }

  //         cout << endl << "------------------------------------------------" << endl << endl;

  //     }catch (GenICam::GenericException &e){

  //         // BOOST_LOG_SEV(logger,fail) << "An exception occured : " << e.GetDescription() ;
  //         cout << "An exception occured : " << e.GetDescription() << endl;
  //         return false;
  //     }

  //     return true;

  // }
  // CaptureBasler::createDevice(int id){

  //     try {

  //         if(!devices.empty()) {

  //             // Create a camera object
  //             if(id >= 0 && id < devices.size()){
  //                 pCamera = new CBaslerGigECamera( pTl->CreateDevice((devices[id]) ));
  //             }else {
  //                 return false;
  //             }

  //             // Open the camera object
  //             pCamera->Open();

  //             if(pCamera->IsOpen())
  //                 cout << endl << "Success to open the device." << endl << endl;
  //                 // BOOST_LOG_SEV(logger,notification) << "Success to open the device.";

  //             return true;
  //         }

  //     }catch (GenICam::GenericException &e){

  //         std::cout << e.GetDescription() << endl;
  //         return false;
  //     }

  //     return false;

  // }
  // CaptureBasler::getDeviceNameById(int id, string &device) {

  //     if(!devices.empty()) {
  //         cout << " Camera (ID:" << id << ") detected " << endl;
  //         cout << " Name :        " << devices[id].GetModelName().c_str() << endl;
  //         return true;
  //     }

  //     return false;

  // }

  // CaptureBasler::grabInitialization(){

  //     if(pCamera){

  //         if(pCamera->IsOpen()){

  //             try{

  //                 //Disable acquisition start trigger if available
  //                 {
  //                     GenApi::IEnumEntry* acquisitionStart = pCamera->TriggerSelector.GetEntry( TriggerSelector_AcquisitionStart);

  //                     if ( acquisitionStart && GenApi::IsAvailable( acquisitionStart)){

  //                         pCamera->TriggerSelector.SetValue( TriggerSelector_AcquisitionStart);
  //                         pCamera->TriggerMode.SetValue( TriggerMode_Off);

  //                     }
  //                 }

  //                 //Disable frame start trigger if available
  //                 {
  //                     GenApi::IEnumEntry* frameStart = pCamera->TriggerSelector.GetEntry( TriggerSelector_FrameStart);

  //                     if ( frameStart && GenApi::IsAvailable( frameStart)){

  //                         pCamera->TriggerSelector.SetValue( TriggerSelector_FrameStart);
  //                         pCamera->TriggerMode.SetValue( TriggerMode_Off);

  //                     }
  //                 }

  //                 //Set acquisition mode
  //                 pCamera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);

  //                 //Set exposure settings
  //                 pCamera->ExposureMode.SetValue(ExposureMode_Timed);

  //                 if (!pStreamGrabber){

  //                     pStreamGrabber = new (CBaslerGigECamera::StreamGrabber_t)(pCamera->GetStreamGrabber(0));

  //                 }

  //                 pStreamGrabber->Open();

  //                 // Get the image buffer size
  //                 const size_t ImageSize = (size_t)(pCamera->PayloadSize.GetValue());

  //                 // We won't use image buffers greater than ImageSize
  //                 pStreamGrabber->MaxBufferSize.SetValue(ImageSize);

  //                 // We won't queue more than nbBuffers image buffers at a time
  //                 pStreamGrabber->MaxNumBuffer.SetValue(nbBuffers);

  //                 pStreamGrabber->PrepareGrab();

  //                 for (int i = 0; i < nbBuffers; ++i){

  //                     //ppBuffers[i] = new unsigned char[ImageSize];
  //                     if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

  //                         ppBuffersUC[i] = new uint8_t[ImageSize];
  //                         handles[i] = pStreamGrabber->RegisterBuffer(ppBuffersUC[i], ImageSize);

  //                     }

  //                     if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

  //                         ppBuffersUS[i] = new uint16_t[ImageSize];
  //                         handles[i] = pStreamGrabber->RegisterBuffer(ppBuffersUS[i], ImageSize);

  //                     }

  //                     pStreamGrabber->QueueBuffer(handles[i], NULL);
  //                 }

  //                 return true;

  //             }catch (GenICam::GenericException &e){

  //                 // Error handling.
  //                 // BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
  //                 cout << "An exception occurred." << e.GetDescription() << endl;
  //                 return false;

  //             }


  //         }
  //     }

  //     return false;

  // }

  // CaptureBasler::getAvailablePixelFormats() {

  //     vector<string> pixfmt;

  //     if(pCamera != NULL) {

  //         if(pCamera->IsOpen()) {

  //             INodeMap *nodemap = pCamera->GetNodeMap();
  //             // Access the PixelFormat enumeration type node.
  //             CEnumerationPtr pixelFormat( nodemap->GetNode( "PixelFormat"));
  //             // Check if the pixel format Mono8 is available.
  //             if(IsAvailable(pixelFormat->GetEntryByName( "Mono8")))
  //                 pixfmt.push_back("MONO8");

  //             // Check if the pixel format Mono12 is available.
  //             if(IsAvailable(pixelFormat->GetEntryByName( "Mono12")))
  //                 pixfmt.push_back("MONO12");

  //             std::cout << endl <<  ">> Available pixel formats :" << endl;
  //             EParser<CamPixFmt> fmt;

  //             for( int i = 0; i != pixfmt.size(); i++ ) {
  //                 if(fmt.isEnumValue(pixfmt.at(i))) {
  //                     std::cout << "- " << pixfmt.at(i) << " available --> ID : " << fmt.parseEnum(pixfmt.at(i)) << endl;
  //                 }
  //             }
  //         }
  //     }
  // }

  // CaptureBasler::getExposureBounds(double &eMin, double &eMax){

  //     INodeMap *nodemap = pCamera->GetNodeMap();

  //     CIntegerPtr exposureTimeRaw(nodemap->GetNode("ExposureTimeRaw"));

  //     if(exposureTimeRaw.IsValid()) {

  //             eMin = exposureTimeRaw->GetMin();
  //             eMax = exposureTimeRaw->GetMax();

  //     }

  // }

  // CaptureBasler::getGainBounds(int &gMin, int &gMax){

  //     INodeMap *nodemap = pCamera->GetNodeMap();

  //     CIntegerPtr gainRaw(nodemap->GetNode("GainRaw"));

  //     if(gainRaw.IsValid()) {

  //             gMin = gainRaw->GetMin();
  //             gMax = gainRaw->GetMax();

  //     }

  // }
  // CaptureBasler::getPixelFormat(CamPixFmt &format){

  //     if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

  //         format = MONO8;
  //         return true;

  //     }else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

  //         format = MONO12;

  //         return true;

  //     }

  //     return false;

  // }
  // CaptureBasler::getFrameSize(int &w , int &h) {

  //     if(pCamera){

  //         try{

  //             if (pCamera->IsAttached() && pCamera->IsOpen()){

  //                 w = pCamera->Width.GetValue();
  //                 h = pCamera->Height.GetValue();

  //                 return true;

  //             }else{

  //                 BOOST_LOG_SEV(logger,fail) << "Can't access width image. Camera not opened or not attached." << endl;

  //             }

  //         }catch (GenICam::GenericException &e){

  //             // Error handling
  //             BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();

  //         }
  //     }

  //     return false;

  // }
  // CaptureBasler::getFPS(double &value) {

  //     if(pCamera!=NULL) {
  //         value = pCamera->ResultingFrameRateAbs.GetValue();
  //         return true;
  //     }

  //     value = 0;
  //     return false;

  // }
  // CaptureBasler::setExposureTime(double exposition) {

  //     if(pCamera){

  //         try{

  //             if( pCamera->IsAttached() && pCamera->IsOpen() ){

  //                 // Check whether auto exposure is available
  //                 if (IsWritable( pCamera->ExposureAuto)){

  //                     // Disable auto exposure.
  //                     cout << "Disable ExposureAuto." << endl;
  //                     pCamera->ExposureAuto.SetValue(ExposureAuto_Off);

  //                 }

  //                 pCamera->ExposureTimeAbs = exposition;

  //             }else{

  //                  std::cout << "Camera not opened or not attached" << endl;
  //             }

  //             return true;

  //         }catch (GenICam::GenericException &e){

  //             // Error handling
  //             // BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
  //             cout << endl << ">> " << e.GetDescription() << endl;
  //             return false;
  //         }
  //     }

  //     return false;

  // }

  // CaptureBasler::setGain(int gain){

  //     if(pCamera){
  //         try{

  //             if( pCamera->IsAttached() && pCamera->IsOpen() ){

  //                 // Check whether auto exposure is available
  //                 if (IsWritable( pCamera->GainAuto)){

  //                     // Disable auto exposure.
  //                     cout << "Disable GainAuto." << endl;
  //                     pCamera->GainAuto.SetValue(GainAuto_Off);

  //                 }

  //                 pCamera->GainRaw = gain;

  //             }else{

  //                 // BOOST_LOG_SEV(logger,fail) << "Camera not opened or not attached";

  //             }

  //             return true;

  //         }catch (GenICam::GenericException &e){

  //             // Error handling
  //             // BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
  //             cout << endl << ">> " << e.GetDescription() << endl;
  //             return false;
  //         }
  //     }

  //     return false;

  // }

  // CaptureBasler::setFPS(double fps){

  //     pCamera->AcquisitionFrameRateAbs = fps;
  //     return true;
  // }

  // CaptureBasler::setPixelFormat(CamPixFmt format){

  //     Basler_GigECamera::PixelFormatEnums fpix;

  //     if(format == MONO8 ){

  //         fpix = PixelFormat_Mono8;

  //     }
  //     else if (format == MONO12 ){

  //         fpix = PixelFormat_Mono12;

  //     }

  //     if (pCamera){

  //         try{
  //             if(pCamera->IsAttached() && pCamera->IsOpen()){

  //                 pCamera->PixelFormat.SetValue(fpix);

  //             }else{

  //                // BOOST_LOG_SEV(logger,fail) << "Camera not opened or not attached";

  //             }
  //         }
  //         catch (GenICam::GenericException &e){

  //             // Error handling
  //             // BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
  //             cout << endl << ">> " << e.GetDescription() << endl;

  //         }

  //         return true;
  //     }

  //     return false;

  // }

  // CaptureBasler::getExposureTime(){

  //     if(pCamera!=0)
  //          return pCamera->ExposureTimeAbs.GetValue();
  //     else
  //         return 0;

  // }

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
  // printf("Enter: ~CaptureBasler\n");
  if(pStreamGrabber != NULL){
        delete pStreamGrabber;
    }

    if(pCamera != NULL) {
        if(pCamera->IsOpen()) pCamera->Close();
        delete pCamera;
    }

    if(pTlFactory != NULL)
        pTlFactory->ReleaseTl(pTl);
  // printf("Exit: ~CaptureBasler\n");
}
bool CaptureBasler::resetBus()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    
  return true;
}

bool CaptureBasler::stopCapture()
{
  // printf("Enter: stopCapture\n");
  if (isCapturing())
  {
    readAllParameterValues();
    
    // Need to release pointers here?
    // IPylonDevice->DestroyDevice(pDevice);
    // IPylonDevice->DestroyDevice(pCamera);
    // ReleaseTl(pTl);
    // pTlFactory->ReleaseTl(pTl);
    
    try
    {
      pCamera->Close();
    }
    catch(GenICam::GenericException &e)
    {
      fprintf(stderr, "BaslerPylon: An error occurred while closing the camera:  '%s')\n", e.GetDescription());
      return false;
    }
    
    is_capturing = false;
  }
 
  vector<VarType *> tmp = capture_settings->getChildren();
  for (unsigned int i=0; i < tmp.size();i++)
  {
    tmp[i]->removeFlags( VARTYPE_FLAG_READONLY );
  }
  
  dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );
  printf("Exit: stopCapture\n");
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
  // printf("Enter: startCapture\n");
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    cam_id = v_cam_bus->getInt();
    // Pylon::PylonAutoInitTerm                autoInitTerm;
    // Get the transport layer factory.
    pTlFactory = &CTlFactory::GetInstance();

    // Get all attached devices and exit application if no device is found.
    // DeviceInfoList_t devices;
    if ( pTlFactory->EnumerateDevices(devices) == 0 )
    {
        // throw RUNTIME_EXCEPTION( "No camera present.");
      fprintf(stderr, "BaslerPylon: No device/camera found");
    }
    fprintf(stderr, "BaslerPylon: Number of cams: %u\n", devices.size());

    if(cam_id >= devices.size())
    {
      fprintf(stderr, "BaslerPylon: Invalid cam_id: %u\n", cam_id);

      #ifndef VDATA_NO_QT
        mutex.unlock();
      #endif
      return false;
    }
    pDevice = pTlFactory->CreateDevice( devices[cam_id] );
    pCamera=new CInstantCamera(pDevice);
    //TODO: open cam and catch exception here
    try
    {
      pCamera->Open();
    }
    catch (GenICam::GenericException &e)
    {
        // Error handling.
      fprintf(stderr, "BaslerPylon: An error occurred while opening the device(error code:  '%s')\n", e.GetDescription());
      #ifndef VDATA_NO_QT
        mutex.unlock();
      #endif
      return false;
    }
    
    cout << "Using device " << pCamera->GetDeviceInfo().GetFullName() << "for cam_id:"<< cam_id << endl;
    pCamera->MaxNumBuffer = 5;
    // capture_format = Colors::stringToColorFormat(v_colormode->getString().c_str());

    pControl = &pCamera->GetNodeMap();
    width = pControl->GetNode("Width");
    height = pControl->GetNode("Height");
    offsetX = pControl->GetNode("OffsetX");
    offsetY = pControl->GetNode("OffsetY");
    pixelFormat = pControl->GetNode( "PixelFormat");
    gainAuto = pControl->GetNode( "GainAuto");
    // Maximize the Image AOI.
    if (IsWritable(offsetX))
    {
        offsetX->SetValue(offsetX->GetMin());
    }
    if (IsWritable(offsetY))
    {
        offsetY->SetValue(offsetY->GetMin());
    }
    width->SetValue(width->GetMax());
    height->SetValue(height->GetMax());
    // ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
    if(true)
    {
      if ( IsAvailable( pixelFormat->GetEntryByName( "Mono8")))// Or RGB8Packed??
      {
          pixelFormat->FromString( "Mono8");
          // cout << "New PixelFormat  : " << pixelFormat->ToString() << endl;
      }
    }
    if ( IsWritable( gainAuto))
    {
        gainAuto->FromString("Off");
    }
    // cout<< "*******************************************\n";

    pCamera->StartGrabbing();
    is_capturing=true;

    // if(pCamera == NULL) {
    //     printf("pCamera is NULL\n");
    //     // return result;
    // }

    // if(pCamera->IsOpen())
    //   {printf("Open status: true\n");}
    // else
    //   printf("Open status: false\n");
    // if(pCamera->IsGrabbing())
    //   {printf("Grabbing status: true\n");}
    // else
    //   printf("Grabbing status: false\n");
    // printf("Going to GrabOne\n");
    // pCamera->GrabOne(500 , ptrGrabResult , TimeoutHandling_Return);
    // printf("Done: GrabOne\n");
    // if(!ptrGrabResult->GrabSucceeded())
    // {
    //   cerr<< "BaslerPylon GrabOne failed:" << ptrGrabResult->GetErrorCode() << ptrGrabResult->GetErrorDescription() << endl;
    //   // fprintf(stderr, "BaslerPylon GrabOne failed: (%d, %s)\n", ptrGrabResult->GetErrorCode(), ptrGrabResult->GetErrorDescription());
    //   // return result;
    // }
    // FOR: non auto gain setting
    // if ( camera.GetSfncVersion() >= Sfnc_2_0_0)
    // {
    //     // Access the Gain float type node. This node is available for USB camera devices.
    //     // USB camera devices are compliant to SFNC version 2.0.
    //     CFloatPtr gain( nodemap.GetNode( "Gain"));
    //     double newGain = gain->GetMin() + ((gain->GetMax() - gain->GetMin()) / 2);
    //     gain->SetValue(newGain);
    //     cout << "Gain (50%)       : " << gain->GetValue() << " (Min: " << gain->GetMin() << "; Max: " << gain->GetMax() << ")" << endl;
    // }
    // else
    // {
    //     // Access the GainRaw integer type node. This node is available for IIDC 1394 and GigE camera devices.
    //     CIntegerPtr gainRaw( nodemap.GetNode( "GainRaw"));
    //     int64_t newGainRaw = gainRaw->GetMin() + ((gainRaw->GetMax() - gainRaw->GetMin()) / 2);
    //     // Make sure the calculated value is valid.
    //     newGainRaw = Adjust(newGainRaw, gainRaw->GetMin(), gainRaw->GetMax(), gainRaw->GetInc());
    //     gainRaw->SetValue(newGainRaw);
    //     cout << "Gain (50%)       : " << gainRaw->GetValue() << " (Min: " << gainRaw->GetMin() << "; Max: " << gainRaw->GetMax() << "; Inc: " << gainRaw->GetInc() << ")" << endl;
    // }
    // Set the pixel data format.
    // camera.PixelFormat.SetValue(PixelFormat_Mono16)

        // Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
        // CInstantCameraArray camera;

        // Create and attach all Pylon Devices.
        // for ( size_t i = 0; i < cameras.GetSize(); ++i)
        // {
        //     cameras[ i ].Attach( tlFactory.CreateDevice( devices[ i ]));

        //     // Print the model name of the camera.
        //     cout << "Using device " << cameras[ i ].GetDeviceInfo().GetModelName() << endl;
        // }

        // Starts grabbing for all cameras starting with index 0. The grabbing
        // is started for one camera after the other. That's why the images of all
        // cameras are not taken at the same time.
        // However, a hardware trigger setup can be used to cause all cameras to grab images synchronously.
        // According to their default configuration, the cameras are
        // set up for free-running continuous acquisition.
        // cameras.StartGrabbing();

        // This smart pointer will receive the grab result data.
        // CGrabResultPtr ptrGrabResult;

  //         // Grab c_countOfImagesToGrab from the cameras.
  //         for( int i = 0; i < c_countOfImagesToGrab && cameras.IsGrabbing(); ++i)
  //         {
  //             cameras.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

  //             // When the cameras in the array are created the camera context value
  //             // is set to the index of the camera in the array.
  //             // The camera context is a user settable value.
  //             // This value is attached to each grab result and can be used
  //             // to determine the camera that produced the grab result.
  //             intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();

  // #ifdef PYLON_WIN_BUILD
  //             // Show the image acquired by each camera in the window related to each camera.
  //             Pylon::DisplayImage(cameraContextValue, ptrGrabResult);
  // #endif

  //             // Print the index and the model name of the camera.
  //             cout << "Camera " <<  cameraContextValue << ": " << cameras[ cameraContextValue ].GetDeviceInfo().GetModelName() << endl;

  //             // Now, the image data can be processed.
  //             cout << "GrabSucceeded: " << ptrGrabResult->GrabSucceeded() << endl;
  //             cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
  //             cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
  //             const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
  //             cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;
  //         }
  //     }
  //     catch (GenICam::GenericException &e)
  //     {
  //         // Error handling
  //         cerr << "An exception occurred." << endl
  //         << e.GetDescription() << endl;
  //         exitCode = 1;
  //     }

        
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    
  printf("BaslerPylon Info: Restoring Previously Saved Camera Parameters\n");
  // writeAllParameterValues();
  // readAllParameterValues();
  printf("Exit: startCapture\n");
  return true;
}
bool CaptureBasler::copyAndConvertFrame ( const RawImage & src, RawImage & target )
{
  // printf("Enter: copyAndConvertFrame\n");
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    // return false;
    // printf("CCF: 1\n");
    ColorFormat src_fmt = src.getColorFormat();
  // printf("CCF: 2\n");
  if(target.getData() == 0)
  {
    //allocate target, if it does not exist yet
    target.allocate(src_fmt, src.getWidth(), src.getHeight());
  } 
  else
  {
    target.ensure_allocation(src_fmt, src.getWidth(), src.getHeight());
  }
  // printf("CCF: 3\n");
  target.setTime(src.getTime());
  // printf("CCF: 4\n");
  // cout<<src_fmt<<endl;
  memcpy(target.getData(),src.getData(),src.getNumBytes());

  // if(src.getColorFormat() == COLOR_RGB8)
  // {
  //   printf("CCF: 4.1\n");
  //   src.getNumBytes();
  //   src.getData();
  //   target.getData();
  //   printf("YOLO\n");
  //   memcpy(target.getData(),src.getData(),src.getNumBytes());
  //   printf("CCF: 4.2\n");
  // }
  // else
  // {
  //   printf("CCF: 4.3\n");
  //   for(int i = 0; i < src.getNumBytes(); i += 2)
  //   {
  //     target.getData()[i+1] = src.getData()[i];
  //     target.getData()[i] = src.getData()[i+1];
  //     // printf("CCF: 4.4\n");
  //   }
  // }
  // printf("CCF: 5\n");
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
  // printf("Exit: copyAndConvertFrame\n");
    return true;
}

RawImage CaptureBasler::getFrame()
{
  // printf("Enetered getFrame\n");
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    // printf("Enetered getFrame mutexlock\n");
    RawImage result;
    result.setColorFormat(COLOR_RGB8);
    result.setWidth(0);
    result.setHeight(0);
    result.setTime(0.0);
    result.setData(0);
    // Create a target image.
    
    
    // printf("RawImage result crossed\n");
    // CGrabResultPtr ptrGrabResult;
    if(pCamera == NULL) {
        // printf("pCamera is NULL\n");
        return result;
    }

   /* if(pCamera->IsOpen())
      {printf("Open status: true\n");}
    else
      printf("Open status: false\n");
    if(pCamera->IsGrabbing())
      {printf("Grabbing status: true\n");}
    else
      printf("Grabbing status: false\n");*/
    // printf("Going to GrabOne\n");
    
    try{

      // if ( pCamera->WaitForFrameTriggerReady(50, TimeoutHandling_Return))
      // {
      //     // printf("OOOOO: inside loop\n");
      //     pCamera->ExecuteSoftwareTrigger();
      //     // printf("OOOOO: Executed Software Trigger\n");
      // }
      // // pCamera->ExecuteSoftwareTrigger();
      // // printf("OOOOO: Going to retrieve image rn\n");
      pCamera->RetrieveResult(50, ptrGrabResult, TimeoutHandling_Return);
      
      // pCamera->GrabOne(25, ptrGrabResult , TimeoutHandling_Return);

    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << e.GetDescription() << endl;
        return result;
        // exitCode = 1;
    }
    // printf("Done: GrabOne\n");
    if(ptrGrabResult->GrabSucceeded())
    {
      // printf("Grab successful\n");
    }
    else
    {
      cerr<< "BaslerPylon GrabOne failed:" << ptrGrabResult->GetErrorCode() << ptrGrabResult->GetErrorDescription() << endl;
      // fprintf(stderr, "BaslerPylon GrabOne failed: (%d, %s)\n", ptrGrabResult->GetErrorCode(), ptrGrabResult->GetErrorDescription());
      #ifndef VDATA_NO_QT
        mutex.unlock();
      #endif
      return result;
    }
    converter.Convert( targetImage, ptrGrabResult);
    timeval tv;
    gettimeofday(&tv,NULL);
    result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
    // printf("HERE::: 1\n");
    result.setWidth(ptrGrabResult->GetWidth());
    // printf("HERE::: 2\n");
    result.setHeight(ptrGrabResult->GetHeight());
    // cout<<"Colorout"<<Colors::stringToColorFormat(v_colorout->getSelection().c_str())<<endl;
    // printf("HERE::: 3\n");
    ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
    result.setColorFormat(COLOR_RGB8);
    // printf("HERE::: 4\n");
    result.setData((unsigned char*)targetImage.GetBuffer());
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    // printf("Exit: getFrame\n");
    return result;
}

void CaptureBasler::releaseFrame()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    //TODO release frame here
    ptrGrabResult.Release();
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

string CaptureBasler::getCaptureMethodName() const
{
  return "Basler";
}
