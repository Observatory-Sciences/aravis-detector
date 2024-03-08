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
#include "logging.h"
#include <boost/algorithm/string.hpp>


namespace FrameProcessor
{

  // Default values and names TBA
  ;

/**
 * @brief Construct for the plugin
 * Currently it only displays a message when the plugin is loaded
 */
AravisDetectorPlugin::AravisDetectorPlugin() :
  working_(true)
{

  try{
    // Open a connection to the first camera available 
    // If you change the first value from NULL to a valid ip/camera model you can select different models
    camera =  arv_camera_new(NULL, &error);
    if (ARV_IS_CAMERA (camera)) {
      printf("Found camera '%s'\n", arv_camera_get_model_name (camera, NULL));
    }else{throw(error);}
  }
  catch(GError error){
    LOG4CXX_ERROR(logger_, "Aravis camera not connected");
    }
  
  // Start the status thread to monitor the camera
  thread_ = new boost::thread(&AravisDetectorPlugin::status_task, this);

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
  LOG4CXX_INFO(logger_, "Test message: this is where I would put my frame processor, if I had one");
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

/** Status execution thread for this class.
 *
 * The thread executes in a continuous loop until the working_ flag is set to false.
 * This thread queries the camera status
 */
void AravisDetectorPlugin::status_task()
{
  // Error return value
	GError *error = NULL;
  // Configure logging for this thread
  OdinData::configure_logging_mdc(OdinData::app_path.c_str());

  // Main worker task of this callback
  // Check the queue for messages
  while (working_) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

    // Lock the camera object if required here

    if (ARV_IS_CAMERA(camera)) {
      // TODO: Change this example
      const char *pixel_format;
      // Read out camera status items here and store to member variable cache
      pixel_format = arv_camera_get_pixel_format_as_string (camera, &error);

      if (error == NULL) {
        LOG4CXX_INFO(logger_, "Pixel format = " << pixel_format);
      }
    }
  }
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