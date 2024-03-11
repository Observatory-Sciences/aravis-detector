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
  /** Default configurations */
  const std::string AravisDetectorPlugin::DEFAULT_CAMERA_IP = "tcp://127.0.0.1:3956";
  const float     AravisDetectorPlugin::DEFAULT_EXPOSURE_TIME = 1000.0;

  /** Config names*/
  const std::string AravisDetectorPlugin::LIST_DEVICES = "list_devices";
  const std::string AravisDetectorPlugin::CONFIG_CAMERA_IP = "ip_address";
  const std::string AravisDetectorPlugin::CONFIG_EXPOSURE = "exposure_time";


/**
 * @brief Construct for the plugin
 * 
 * On plugin call it will display the number of available devices, as well as their id
 */
AravisDetectorPlugin::AravisDetectorPlugin() :
  working_(true)
{
  // Start the status thread to monitor the camera
  thread_ = new boost::thread(&AravisDetectorPlugin::status_task, this);

  logger_ = Logger::getLogger("FP.AravisDetectorPlugin");
  LOG4CXX_INFO(logger_, "AravisDetectorPlugin loaded");

  display_aravis_cameras();
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
 * @brief Takes json configuration files
 * 
 * 
 * 
 * @param[in] config - IpcMessage containing configuration data
 * @param[out] reply - Response IpcMessage
 */
void AravisDetectorPlugin::configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply){
  /** List of config options:
   Find devices flag
   2. Start/Stop 
   3. Exposure time
   4. frame count
   5. Data type (bit depth)
   */
  try{
    /** List all devices*/
    if (config.has_param(LIST_DEVICES))
    {
      display_aravis_cameras();
    }
    if (config.has_param(CONFIG_CAMERA_IP)){
      connect_aravis_camera(config.get_param<std::string>(CONFIG_CAMERA_IP));
    }
    if (config.has_param(CONFIG_EXPOSURE)){
      set_exposure(config.get_param<float>(CONFIG_EXPOSURE));
    }
  }
  catch (std::runtime_error& e)
  {
    std::stringstream ss;
    ss << "Bad ctrl msg: " << e.what();
    this->set_error(ss.str());
    throw;
  }
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

/** @brief Change exposure time in miliseconds
 * 
 */
void AravisDetectorPlugin::set_exposure(float exposure_time_us){
  try{
    arv_camera_set_exposure_time(camera, exposure_time_us, &error);
    LOG4CXX_INFO(logger_, ("Setting exposure time to %f", exposure_time_us));
  }catch(GError error){
    LOG4CXX_ERROR(logger_, ("When setting exposure time the following error ocurred: %s", error.message));
  }
}

/** @brief Connects to a camera using the ip address
 * 
 * Can in theory work with any of the following names:
 * 
 * - <vendor>-<model>-<serial>
 * - <vendor_alias>-<serial>
 * - <vendor>-<serial>
 * - <user_id>
 * - <ip_address>
 * - <mac_address>
 *  
 * The function checks for the number of devices connected. If it's zero it logs an error.
 * If not then it replaces the camera object with a new camera connection. Reports back if 
 * it was successful or not.
 *  
 * @param ip std::string of ip address
 */
void AravisDetectorPlugin::connect_aravis_camera(std::string ip){
  LOG4CXX_INFO(logger_, "Connecting to camera");
  
  arv_update_device_list();
  number_of_cameras = arv_get_n_devices();
  if(number_of_cameras == 0){
    LOG4CXX_ERROR(logger_, "No aravis cameras found");
  }else{
      // there might be a better way to do this
      // arv_camera_new needs a char pointer, so I'm changing the string to a char
      const char *ip_copy = ip.c_str();
      try{
        camera = arv_camera_new(ip_copy, &error);
      }catch(GError error){
        LOG4CXX_ERROR(logger_, "Error when connecting to camera. Please confirm camera is connected");
      }
     if (ARV_IS_CAMERA (camera)) {
       LOG4CXX_INFO(logger_,("Connected to camera '%s'\n", arv_camera_get_model_name (camera, NULL)));
     }
   }
}

/** @brief Checks for available devices
 * 
 * Displays in the console a list of all available cameras in the format:
 * 
 * [unsigned int] cameras were detected:
 * Device index [int] has the id [str] and address [str]
 */
void AravisDetectorPlugin::display_aravis_cameras(){
  // Updating the device list is required before using get device id
  arv_update_device_list();
  number_of_cameras = arv_get_n_devices();
  if(number_of_cameras==0){
    LOG4CXX_WARN(logger_, "No cameras were detected. Please confirm camera is connected");
  }else{
    LOG4CXX_INFO(logger_, ("%i cameras were detected: \n ", number_of_cameras));
    for(int i=0; i<number_of_cameras; i++){
      const char* device_id = arv_get_device_id(i);
      const char* device_address = arv_get_device_address(i);
      LOG4CXX_INFO(logger_,("Device index %i has the id %s  and address %s\n", i, device_id, device_address));
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