/**
 * @file AravisDetectorPlugin.cpp
 * @author George S. Chira (gsc@observatorysciences.co.uk)
 * @brief 
 * @date 2024-03-04
 * 
 * The most minimal version of a plugin (that doesn't do anything)
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "AravisDetectorPlugin.h"
#include "version.h"
#include <boost/algorithm/string.hpp>


namespace FrameProcessor
{

  // Default values and names TBA
  ;

/**
 * @brief Construct for the plugin
 * Connects to the first available camera, or it crashes.
 */
AravisDetectorPlugin::AravisDetectorPlugin()
{

  try{
    // Open a connection to the first camera available 
    // If you change the first value from NULL to a valid ip/camera model you can select different models
    camera =  arv_camera_new(NULL, &error);
    if (ARV_IS_CAMERA (camera)) {
      printf("Found camera '%s'\n", arv_camera_get_model_name (camera, NULL));
    }else{
      printf("Error: \n '%s' \n Encountered when connecting to first available camera \n", error->message);}
  }
  catch(GError error){
    LOG4CXX_ERROR(logger_, "Aravis camera not connected");
  }

  logger_ = Logger::getLogger("FP.AravisDetectorPlugin");
  LOG4CXX_INFO(logger_, "AravisDetectorPlugin loaded");
}

/**
 * Class Destructor. Closes the Publish socket
 */
AravisDetectorPlugin::~AravisDetectorPlugin()
{
  LOG4CXX_TRACE(logger_, "AravisDetectorPlugin destructor.");
}

/**
 * @brief Currently the function doesn't process the frame. It simply logs different messages depending if the plugin is connected or not
 * (still in progress)
 * @param[in] frame - pointer to frame object 
 */
void AravisDetectorPlugin::process_frame(boost::shared_ptr<Frame> frame)
{
  if(is_bound_){
    LOG4CXX_INFO(logger_, "Test message: this is where I would put my frame processor, if I had one");
  }
  else{
    LOG4CXX_WARN(logger_, "Socket is UNBOUND");
  }
  LOG4CXX_TRACE(logger_, "Pushing Data Frame");
  this->push(frame);
}

/**
 * @brief Takes json configuration files and ignores them (still in progress)
 * 
 * @param[in] config - IpcMessage containing configuration data
 * @param[out] reply - Response IpcMessage
 */
void AravisDetectorPlugin::configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply){
  LOG4CXX_INFO(logger_, "Test message: configurations were received. They are ignored but they were received");
}

int AravisDetectorPlugin::get_version_major()
{
  return ARAVIS_DETECTOR_VERSION_MAJOR;
}

int AravisDetectorPlugin::get_version_minor()
{
  return ARAVIS_DETECTOR_VERSION_MINOR;
}

int AravisDetectorPlugin::get_version_patch()
{
  return ARAVIS_DETECTOR_VERSION_PATCH;
}

std::string AravisDetectorPlugin::get_version_short()
{
  return ARAVIS_DETECTOR_VERSION_STR_SHORT;
}

std::string AravisDetectorPlugin::get_version_long()
{
  return ARAVIS_DETECTOR_VERSION_STR;
}
}