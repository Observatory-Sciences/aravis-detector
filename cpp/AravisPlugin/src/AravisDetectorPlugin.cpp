/**
 * @file AravisDetectorPlugin.cpp
 * @author George S. Chira (gsc@observatorysciences.co.uk)
 * @brief A plugin that connects to GenICam cameras for Odin-data
 * @date 2024-03-04
 * 
 * 
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "AravisDetectorPlugin.h"
#include "version.h"
#include "logging.h"
#include <boost/algorithm/string.hpp>

/** @brief destructs GError objects
 * 
 * This wrapper helps with error handling:
 * Creates a new error pointer and sets it to NULL
 */
struct GErrorWrapper {
  GError *error;
  GErrorWrapper(): error(NULL){}
  ~GErrorWrapper(){ if(error){
      g_error_free(error);
  }}
  GError** get() {return &error;}
  operator GError*() const { return error;}
  GError* operator->() const {return error;}

  /** @brief Safely access error messages
  * If error is NULL when it's called it prevents 
  * bugs by returning a predefined value. 
* @return std::string: Error message 
   */
  std::string message(){
    if(error) return error->message;
    return "Error message was called when error is NULL";
    }
};

namespace FrameProcessor
{
  /** Default configurations */
  const std::string AravisDetectorPlugin::DEFAULT_CAMERA_IP     = "127.0.0.1";
  const std::string AravisDetectorPlugin::DEFAULT_CAMERA_ID     = "";
  const std::string AravisDetectorPlugin::DEFAULT_CAMERA_SERIAL = "";
  const std::string AravisDetectorPlugin::DEFAULT_CAMERA_MODEL  = "";

  const double      AravisDetectorPlugin::DEFAULT_EXPOSURE_TIME = 1000.0;
  const double      AravisDetectorPlugin::DEFAULT_FRAME_RATE    = 5;
  const double      AravisDetectorPlugin::DEFAULT_FRAME_COUNT   = 0;
  const std::string AravisDetectorPlugin::DEFAULT_PIXEL_FORMAT  = "Mono8";
  const std::string AravisDetectorPlugin::DEFAULT_AQUISIT_MODE  = "Continuous";
  const size_t      AravisDetectorPlugin::DEFAULT_STATUS_FREQ   = 1000;
  const int         AravisDetectorPlugin::DEFAULT_EMPTY_BUFF    = 50;

  const std::string AravisDetectorPlugin::DEFAULT_FILE_PATH     = "/";
  const std::string AravisDetectorPlugin::DEFAULT_DATASET       = "data";
  const std::string AravisDetectorPlugin::DEFAULT_FILE_NAME     = "test";
  
  /** Flags*/
  const std::string AravisDetectorPlugin::START_STREAM        = "start";
  const std::string AravisDetectorPlugin::STOP_STREAM         = "stop";
  const std::string AravisDetectorPlugin::LIST_DEVICES        = "list_devices";
  const std::string AravisDetectorPlugin::ACQUIRE_BUFFER      = "frames";

  /** Config names*/
  const std::string AravisDetectorPlugin::CONFIG_CAMERA_IP    = "ip_address";
  const std::string AravisDetectorPlugin::CONFIG_CAMERA_ID    = "camera_id";
  const std::string AravisDetectorPlugin::CONFIG_CAMERA_SERIAL= "camera_serial_number";
  const std::string AravisDetectorPlugin::CONFIG_CAMERA_MODEL = "camera_model";

  const std::string AravisDetectorPlugin::CONFIG_EXPOSURE     = "exposure_time";
  const std::string AravisDetectorPlugin::CONFIG_FRAME_RATE   = "frame_rate";
  const std::string AravisDetectorPlugin::CONFIG_FRAME_COUNT  = "frame_count";
  const std::string AravisDetectorPlugin::CONFIG_PIXEL_FORMAT = "pixel_format";
  const std::string AravisDetectorPlugin::CONFIG_ACQUISITION_MODE = "acquisition_mode";
  const std::string AravisDetectorPlugin::CONFIG_STATUS_FREQ  = "status_frequency_ms";
  const std::string AravisDetectorPlugin::CONFIG_EMPTY_BUFF   = "empty_buffers";

  /** Names and settings */
  const std::string AravisDetectorPlugin::TEMP_FILES_PATH     = "file_path";
  const std::string AravisDetectorPlugin::DATA_SET_NAME       = "data_set_name";
  const std::string AravisDetectorPlugin::FILE_NAME           = "file_name"; 
  const std::string AravisDetectorPlugin::COMPRESSION_TYPE    = "compression";

/** @brief Constructor for the plugin
 * 
 * Sets default values, starts the status monitoring thread and logger object
 * Then it logs "AravisDetectorPlugin loaded"
 */
AravisDetectorPlugin::AravisDetectorPlugin() :
  working_(true),
  streaming_(false),
  camera_connected_(false),
  frame_count_(0)
{
  // Start the status thread to monitor the camera
  thread_ = new boost::thread(&AravisDetectorPlugin::status_task, this);

  logger_ = Logger::getLogger("FP.AravisDetectorPlugin");
  LOG4CXX_INFO(logger_, "AravisDetectorPlugin loaded");

}

/** @brief Class Destructor. Closes the Publish socket */
AravisDetectorPlugin::~AravisDetectorPlugin()
{
  arv_shutdown();
  LOG4CXX_TRACE(logger_, "AravisDetectorPlugin destructor.");
}

/** @brief Push the frame to the next plugin
 * 
 * No image processing is done here at the moment
 * 
 * @param[in] frame - pointer to frame object 
 */
void AravisDetectorPlugin::process_frame(boost::shared_ptr<Frame> frame)
{
  this->push(frame);
}

/** @brief Implements json configurations
 * 
 * @param[in] config - IpcMessage containing configuration data
 * @param[out] reply - Response IpcMessage
 */
void AravisDetectorPlugin::configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply){
  try{
    /** List all devices*/
    if (config.has_param(START_STREAM)){
      start_stream(reply);}
    if (config.has_param(STOP_STREAM)){
      stop_stream(reply);}
    if (config.has_param(LIST_DEVICES)){
      find_aravis_cameras(reply);}
    if (config.has_param(ACQUIRE_BUFFER)){
      acquire_n_buffer(config.get_param<int>(ACQUIRE_BUFFER), reply);}
    if (config.has_param(CONFIG_CAMERA_IP)){
      connect_aravis_camera(config.get_param<std::string>(CONFIG_CAMERA_IP), reply);}
    if (config.has_param(TEMP_FILES_PATH)){
      set_file_path(config.get_param<std::string>(TEMP_FILES_PATH), reply);}
    if (config.has_param(CONFIG_STATUS_FREQ)){
      set_status_poll_frequency(static_cast<size_t>(config.get_param<int>(CONFIG_STATUS_FREQ)), reply);}
    if (config.has_param(CONFIG_EXPOSURE)){
      set_exposure(config.get_param<double>(CONFIG_EXPOSURE), reply);
    }
    if (config.has_param(CONFIG_FRAME_RATE)){
      set_frame_rate(config.get_param<double>(CONFIG_FRAME_RATE), reply);
    }
    if (config.has_param(CONFIG_FRAME_COUNT)){
      set_frame_count(config.get_param<int32_t>(CONFIG_FRAME_COUNT), reply);
    }
    if (config.has_param(CONFIG_PIXEL_FORMAT)){
      set_pixel_format(config.get_param<std::string>(CONFIG_PIXEL_FORMAT), reply);
    }
    if (config.has_param(CONFIG_ACQUISITION_MODE)){
      set_acquisition_mode(config.get_param<std::string>(CONFIG_ACQUISITION_MODE), reply);
    }
    if (config.has_param(DATA_SET_NAME)){
      set_dataset_name(config.get_param<std::string>(DATA_SET_NAME), reply);
    }
    if (config.has_param(FILE_NAME)){
      set_file_name(config.get_param<std::string>(FILE_NAME), reply);
    }   
    if (config.has_param(COMPRESSION_TYPE)){
      set_compression_type(config.get_param<std::string>(COMPRESSION_TYPE), reply);
    }
  }
  catch (std::runtime_error& e)
  {
    std::stringstream ss;
    ss << "Bad ctrl msg: " << e.what();
    log_error(ss.str(), reply);
    throw;
  }
}

/** @brief Provides python client with current configuration data in json format
 * 
 * @param[out] reply - Response IpcMessage
 */
void AravisDetectorPlugin::requestConfiguration(OdinData::IpcMessage& reply){
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_CAMERA_IP, camera_address_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_CAMERA_ID, camera_id_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_CAMERA_SERIAL, camera_serial_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_CAMERA_MODEL, camera_model_);

    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_EXPOSURE, exposure_time_us_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_FRAME_RATE, frame_rate_hz_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_FRAME_COUNT, frame_count_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_PIXEL_FORMAT, pixel_format_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_ACQUISITION_MODE, acquisition_mode_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_STATUS_FREQ, status_freq_ms_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::CONFIG_EMPTY_BUFF, n_empty_buffers_);

    reply.set_param(get_name() + "/" + AravisDetectorPlugin::TEMP_FILES_PATH, temp_file_path_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::DATA_SET_NAME, data_set_name_);
    reply.set_param(get_name() + "/" + AravisDetectorPlugin::FILE_NAME, file_id_);

}

/** @brief Provides python client with current status of the camera in json format
 * 
 * @param status - Response IpcMessage
 */
void AravisDetectorPlugin::status(OdinData::IpcMessage &status){

  /** Camera parameters */
  status.set_param(get_name() + "/" + "camera_id", camera_id_);
  status.set_param(get_name() + "/" + "camera_ip", camera_address_);
  status.set_param(get_name() + "/" + "camera_model", camera_model_);
  status.set_param(get_name() + "/" + "camera_connected", camera_connected_);

  /** List all devices found on network by index*/
  status.set_param(get_name()+ "/" + "connected_devices",connected_devices_);
  for (auto& [key, val]: available_cameras_){
    status.set_param(get_name() + "/" + "camera_" + key + "_id", val.first);
    status.set_param(get_name() + "/" + "camera_" + key + "_address", val.second);
  }

  /** Stream parameters*/
  status.set_param(get_name() + "/" + "payload", payload_);
  status.set_param(get_name()+ "/" + "image_height",static_cast<long unsigned int>(image_height_px_));
  status.set_param(get_name()+ "/" + "image_width", static_cast<long unsigned int>(image_width_px_));

  status.set_param(get_name() + "/" + "streaming", streaming_);

  status.set_param(get_name() + "/" + "input_buffers", n_input_buff_);
  status.set_param(get_name() + "/" + "output_buffers", n_output_buff_);

  status.set_param(get_name()+ "/" + "frames_made", static_cast<long int>(n_frames_made_));
  status.set_param(get_name() + "/" + "completed_buff", n_completed_buff_);
  status.set_param(get_name() + "/" + "failed_buff", n_failed_buff_);
  status.set_param(get_name() + "/" + "underrun_buff", n_underrun_buff_);
}

/** @brief Reset stream statistics */
bool AravisDetectorPlugin::reset_statistics(){
    n_input_buff_ =0;
    n_output_buff_ =0;
    n_completed_buff_ =0;
    n_failed_buff_ =0;  
    n_underrun_buff_ =0;
    return true;
}

/** @brief Status execution thread for this class.
 *
 * The thread executes in a continuous loop until the working_ flag is set to false.
 * This thread queries the camera status
 */
void AravisDetectorPlugin::status_task()
{
  // Error return value
	GErrorWrapper error;
  // Configure logging for this thread
  OdinData::configure_logging_mdc(OdinData::app_path.c_str());

  // Main worker task of this callback
  // Check the queue for messages
  while (working_) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(status_freq_ms_));

    if (camera_connected_){
      get_config(GET_CONFIG_CAMERA_PARAMS);
      if(streaming_){
        get_config(GET_CONFIG_STREAM_STAT);
      }
    }
  }
}


/** @brief Send errors to the frame processor and set the Ipc reply*/
void AravisDetectorPlugin::log_error(std::string msg, OdinData::IpcMessage& reply){
  this->set_error(msg);
  reply.set_nack(msg);
}
/** @brief Overload for simple error logs.*/
void AravisDetectorPlugin::log_error(std::string msg){
  this->set_error(msg);
}
/** @brief Send warning to the frame processor and set the Ipc reply*/
void AravisDetectorPlugin::log_warning(std::string msg, OdinData::IpcMessage& reply){
  this->set_warning(msg);
  reply.set_nack(msg);
}
/** @brief Overload for simple warning logs.*/
void AravisDetectorPlugin::log_warning(std::string msg){
  this->set_warning(msg);
}


/** @brief Populates config variables
 * 
 * This function gets called periodically to set config variables that can be
 * accessed by read_config. 
 * 
 * @param get_option (int32_t): 0- all,  1- Camera init routine, 2- Camera parameter check, 3- Stream statistics, Others: error
 */
void AravisDetectorPlugin::get_config(int32_t get_option){

  switch(get_option){
    case GET_CONFIG_CAMERA_INIT:
     /** Camera init routine */

      get_camera_serial();
      get_camera_id();

      get_exposure_bounds();
      get_exposure();

      get_frame_rate_bounds();
      get_frame_rate();

      get_available_pixel_formats();
      get_pixel_format();

      get_acquisition_mode();
      get_frame_size();

      break;

    case GET_CONFIG_CAMERA_PARAMS: 
    /** Constant camera parameter check */

      check_connection();

      if(camera_connected_){
        get_frame_rate();
        get_exposure();
        get_pixel_format();
        get_acquisition_mode();
        get_frame_size();
      }
      break;
    case GET_CONFIG_STREAM_STAT:
    /** Stream Statistics*/ 
      get_stream_state();
      break;
    
    case GET_CONFIG_ALL:
    /** All values */
      get_exposure_bounds();
      get_exposure();

      get_frame_rate_bounds();
      get_frame_rate();

      get_available_pixel_formats();
      get_pixel_format();

      get_acquisition_mode();
      get_frame_size();

      get_stream_state();
      break;
      
  default:
    log_error("Invalid get_config option");      
}
}


/** @brief Change file name
 * 
 * @param file_id string
 * @param reply ipc message log
 */
void AravisDetectorPlugin::set_file_name(std::string file_id,  OdinData::IpcMessage& reply){
  LOG4CXX_INFO(logger_, "file_id_ | old: "<< file_id_ << " | new:" << file_id);
  file_id_ = file_id;
}

/** @brief Change file path
 * 
 * @param new_file_path string. checked to be valid
 * @param reply ipc message log
 */
void AravisDetectorPlugin::set_file_path(std::string new_file_path,  OdinData::IpcMessage& reply){
  struct stat s;
  if( stat(new_file_path.c_str(),&s) != 0){
    log_error("temporary file path "+temp_file_path_+" not valid", reply);
    return;
  }
  LOG4CXX_INFO(logger_, "temp_file_path_ | old: "<< temp_file_path_ << " | new:" << new_file_path);
  temp_file_path_ = new_file_path;
}

/** @brief Change data set name
 * 
 * @param data_set_name string
 * @param reply ipc message log
 */
void AravisDetectorPlugin::set_dataset_name(std::string data_set_name,  OdinData::IpcMessage& reply){
  LOG4CXX_INFO(logger_, "data_set_name_ | old: "<< data_set_name_ << " | new:" << data_set_name);
  data_set_name_ = data_set_name;
}

/** @brief Change compression type used 
 * 
 * @param compression_type string
 * @param reply ipc message log
 */
void AravisDetectorPlugin::set_compression_type(std::string compression_type,  OdinData::IpcMessage& reply){
  LOG4CXX_INFO(logger_, "compression_type_ | old: "<< get_compress_from_enum(compression_type_) << " | new:" << compression_type);
  compression_type_ = get_compression_from_string(compression_type);
}

/** @brief Change status polling frequency
 * 
 * @param status_freq_ms size_t, in miliseconds
 * @param reply ipc message log
 */
void AravisDetectorPlugin::set_status_poll_frequency(size_t status_freq_ms,  OdinData::IpcMessage& reply){
  LOG4CXX_INFO(logger_, "status_freq_ms_ | old: "<< status_freq_ms_ << " | new:" << status_freq_ms);
  status_freq_ms_ = status_freq_ms;
}


/*******************************
*      Callback functions      *
********************************/

/** @brief Called by GObject library when Aravis camera finishes a buffer
 * 
 * @param stream_temp pointer to currently used ArvStream object 
 * @param object_temp pointer to the AravisDetectorPlugin currently running
 */
static void buffer_callback(ArvStream *stream_temp, AravisDetectorPlugin *object_temp){
  object_temp->callback_access(stream_temp);
}

/** @brief Provides the callback function with access to acquire_stream_buffer
 * 
 * @param stream_temp pointer to currently used ArvStream object 
 */
void AravisDetectorPlugin::callback_access(ArvStream *stream_temp){
  stream_ = stream_temp;
  acquire_buffer();
} 


/*********************************
**       Camera Functions       **
**********************************/

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
 * @param ip_string std::string of ip address
 */
void AravisDetectorPlugin::connect_aravis_camera(std::string ip_string, OdinData::IpcMessage& reply){
  GErrorWrapper error;
  arv_update_device_list();
  unsigned int number_of_cameras = arv_get_n_devices();

  if(number_of_cameras == 0){
    log_warning("No camera found on network", reply);
    return;
  }

  camera_ = arv_camera_new(ip_string.c_str(), error.get());

  if(error){log_error("Error when connecting to camera: "+ error.message(), reply);
    return;}

  if (!ARV_IS_CAMERA (camera_)){log_error("Failed to create camera object", reply);
    return;}


  camera_model_ = arv_camera_get_model_name (camera_, NULL);
  LOG4CXX_INFO(logger_,"Connected to camera " << camera_model_);
    
  /***************************************
  **      Camera init routine
  ****************************************/

  LOG4CXX_INFO(logger_, "camera address old:"<< camera_address_ << " new:" << ip_string );

  camera_address_ = ip_string;
  camera_connected_ = true;

  // get config values 
  get_config(GET_CONFIG_CAMERA_INIT);

  save_genicam_xml(temp_file_path_);

  // display configs
  LOG4CXX_INFO(logger_, "The exposure time bounds are min: " << min_exposure_time_ << " and max: "<< max_exposure_time_);
  LOG4CXX_INFO(logger_, "The exposure time is set at " << exposure_time_us_ << " microseconds");
  
  LOG4CXX_INFO(logger_, "The frame rate bounds are min: " << min_frame_rate_ << " and max: "<< max_frame_rate_);
  LOG4CXX_INFO(logger_, "Frame rate is "<< frame_rate_hz_ << " frames per second");
  
  LOG4CXX_INFO(logger_, "There are "<< n_pixel_formats_ <<" pixel formats: \n" << available_pixel_formats_);
  LOG4CXX_INFO(logger_, "Currently using "<< pixel_format_ <<" format");

  LOG4CXX_INFO(logger_, "Camera acquisition mode is set on: "<< acquisition_mode_);
  LOG4CXX_INFO(logger_, "Frame size: "<< payload_);
}

/** @brief check that camera is still connected
 * 
 */
void AravisDetectorPlugin::check_connection(){

  if(camera_ == NULL){
    log_error("No connection, camera object removed unexpectedly during run");
    camera_connected_ = false;
    return;
  }

  arv_update_device_list();
  unsigned int number_of_cameras = arv_get_n_devices();

  if(number_of_cameras==0){camera_connected_= false;
    LOG4CXX_INFO(logger_, "No camera found on network");}

  bool found_match = false;
  for(int i=0; i<number_of_cameras; i++){
    if (camera_address_ == arv_get_device_address(i)){
      found_match =true;
    }
  }

  if(!found_match){     camera_connected_= false;
    LOG4CXX_INFO(logger_, "No connection, none of the cameras available match the address");
  }

  GErrorWrapper error;
  std::string serial_temp = arv_camera_get_device_serial_number(camera_, error.get());

  if(error){camera_connected_=false; 
    LOG4CXX_INFO(logger_, "No connection, error: "<< error.message());  
    }

  if(camera_serial_ != serial_temp){camera_connected_=false; 
    LOG4CXX_INFO(logger_, "Connected to different camera");    }

  // If camera is connected after all that, then exit
  if(camera_connected_) return;

  // if not, we need to stop all camera related processes
  // OdinData::IpcMessage msg;
  if(streaming_)auto_stop_stream();
  camera_ = NULL;
}


/** @brief Checks for available devices
 * 
 * Displays in the console a list of all available cameras in the format:
 * 
 * [unsigned int] cameras were detected:
 * Device index [int] has the id [str] and address [str]
 */
void AravisDetectorPlugin::find_aravis_cameras(OdinData::IpcMessage& reply){
  // Updating the device list is required before using get device id
  arv_update_device_list();
  unsigned int number_of_cameras = arv_get_n_devices();

  if(number_of_cameras==0){ log_warning("No camera found on network", reply);
    return;}

  connected_devices_ = number_of_cameras;
  for(int i=0; i<number_of_cameras; i++){
    available_cameras_[std::to_string(i)] = std::make_pair(arv_get_device_id(i),arv_get_device_address(i));
  }
}

/** @brief Get serial of the current connected camera
 *
 * Saved to camera_serial_
 */
void AravisDetectorPlugin::get_camera_serial(){
  GErrorWrapper error;

  if(camera_ == NULL){
    log_error("Cannot get camera serial number without connecting to camera");
    return;
  }

  std::string serial_temp = arv_camera_get_device_serial_number(camera_, error.get());

  if(error){ 
    log_error("When reading camera serial number the following error occurred: \n" + error.message());
      return;
    }
  camera_serial_ = serial_temp;
}

/** @brief Get id of the current connected camera
 *
 * Saved to camera_id_
 */
void AravisDetectorPlugin::get_camera_id(){
  GErrorWrapper error;

  if(camera_ == NULL){
    log_error("Cannot get camera id without connecting to camera");
    return;
  }

  std::string id_temp = arv_camera_get_device_id(camera_, error.get());

  if(error){ 
    log_error("When reading camera id the following error occurred: \n" + error.message());
      return;
    }
  camera_id_ = id_temp;
}


/** @brief Set acquisition mode for camera
 * 
 * @param acq_mode std::string = one of the following: "Continuous", "SingleFrame","MultiFrame"
 */
void AravisDetectorPlugin::set_acquisition_mode(std::string acq_mode, OdinData::IpcMessage& reply){
  
  if(!(acq_mode == "Continuous" || acq_mode == "SingleFrame" ||acq_mode == "MultiFrame")){
    log_error("the acquisition mode supplied: " + acq_mode +" is invalid and must be of the following: Continuous, SingleFrame, MultiFrame");
    return;
  }
  
  GErrorWrapper error;
  ArvAcquisitionMode temp= arv_acquisition_mode_from_string(acq_mode.c_str());
  arv_camera_set_acquisition_mode(camera_, temp, error.get());
  if(error){
    log_error("When setting acquisition mode the following error ocurred: \n" + error.message(), reply);
    return;
  }
  LOG4CXX_INFO(logger_, "Previous acquisition mode:"<< acquisition_mode_ << " new:" << acq_mode );
  acquisition_mode_= acq_mode;
}

/** @brief Get current acquisition mode
 * 
 * Saves acquisition_mode_ as one of the following: "Continuous", "SingleFrame","MultiFrame"
 */
void AravisDetectorPlugin::get_acquisition_mode(){
  GErrorWrapper error;
  ArvAcquisitionMode temp = arv_camera_get_acquisition_mode(camera_, error.get());
  if(error){
    log_error("When getting acquisition mode the following error ocurred: \n" + error.message());
    return;
  }
  acquisition_mode_ = arv_acquisition_mode_to_string(temp);
}

/** @brief Set exposure time in microseconds
 * 
 * On success prints:
 *  Setting exposure time to <exposure_time_us>
 * On failure:
 *  When setting exposure time the following error ocurred: <error.message()>
 * 
 * @param exposure_time_us
 */
void AravisDetectorPlugin::set_exposure(double exposure_time_us, OdinData::IpcMessage& reply){
  GErrorWrapper error;

  // aravis already checks this but we warn the user
  if(exposure_time_us < min_exposure_time_){
    log_error("The exposure time: "+ std::to_string(exposure_time_us) + " is out of bounds: min="+std::to_string(min_exposure_time_)+" and is set to minimum", reply);
    exposure_time_us = min_exposure_time_;
  } else if(exposure_time_us > max_exposure_time_){
    log_error("The exposure time: "+ std::to_string(exposure_time_us) + " is out of bounds: max="+std::to_string(max_exposure_time_) +" and is set to maximum", reply);
    exposure_time_us = max_exposure_time_;
  }
  
  arv_camera_set_exposure_time(camera_, exposure_time_us, error.get());

  if(error){ 
    log_error("When setting exposure time the following error ocurred: \n" + error.message(), reply);
    return;
  }   

  LOG4CXX_INFO(logger_, "exposure_time_us_ | old: "<< exposure_time_us_ << " | new:" << exposure_time_us);
  exposure_time_us_ = exposure_time_us;
}

/** @brief Get exposure time bounds in microseconds
* Saves the values to min_exposure_time_ and max_exposure_time_
*/
void AravisDetectorPlugin::get_exposure_bounds(){
  GErrorWrapper error;
  double min_expo, max_expo;
  arv_camera_get_exposure_time_bounds(camera_, &min_expo, &max_expo, error.get());

  if(error){
    log_error("When reading exposure time the following error ocurred: \n" + error.message());
    return;
  } 

  max_exposure_time_ = max_expo;
  min_exposure_time_ = min_expo;
}

/** @brief Get exposure time in microseconds
 * 
 * On success prints:
 *  Saves the value to exposure_time_us_
 * On failure:
 *  When reading exposure time the following error ocurred: <error.message()>
 */
void AravisDetectorPlugin::get_exposure(){
  GErrorWrapper error;
  double temp = arv_camera_get_exposure_time(camera_, error.get());

  if(error){ 
    log_error("When reading exposure time the following error ocurred: \n" + error.message());
    return;
  }

  exposure_time_us_ = temp;
}


/** @brief Set frame rate in hertz
 * 
 * Checks that it fits within the minimum and maximum frame rate.
 * 
 * On success prints:
 *  Setting frame rate to <frame_rate_hz>
 * On failure:
 *  When setting frame rate the following error ocurred: <error.message()>
 * 
 * @param frame_rate_hz number of frames per second
 */
void AravisDetectorPlugin::set_frame_rate(double frame_rate_hz, OdinData::IpcMessage& reply){
  GErrorWrapper error;

  // aravis already checks this but we change it for log reasons
  if(frame_rate_hz < min_frame_rate_){
    log_error("The frame rate: "+ std::to_string(frame_rate_hz) + " is out of bounds: min="+ std::to_string( min_frame_rate_)+" and is set to minimum", reply);
    frame_rate_hz= min_frame_rate_;
  } else if(frame_rate_hz > max_frame_rate_){
    log_error("The frame rate: "+ std::to_string( frame_rate_hz) + " is out of bounds: max="+ std::to_string(max_frame_rate_) + " and is set to maximum", reply);
    frame_rate_hz = max_frame_rate_;
  }

  arv_camera_set_frame_rate(camera_, frame_rate_hz, error.get());

  if(error){ 
    log_error("When setting frame rate the following error ocurred: \n" + error.message(), reply);
    return;
  }

  LOG4CXX_INFO(logger_, "frame_rate_hz_ | old: "<< frame_rate_hz_ << " | new:" << frame_rate_hz);
  frame_rate_hz_ = frame_rate_hz;
}

/** @brief Read frame rate bounds from the camera
 * Saves the values to min_frame_rate_ and max_frame_rate
 */
void AravisDetectorPlugin::get_frame_rate_bounds(){
  GErrorWrapper error;
  double min_temp, max_temp;

  arv_camera_get_frame_rate_bounds(camera_, &min_temp , &max_temp, error.get());

  if(error){ 
    log_error("When reading frame rate bounds the following error ocurred: \n" + error.message());
    return;
  }

  min_frame_rate_ = min_temp;
  max_frame_rate_ = max_temp;
}

/** @brief Get frame rate in hertz
 * 
 * On success prints:
 *  Saves frame rate value to frame_rate_hz_
 * On failure:
 *  When reading frame rate the following error ocurred: <error.message()>
 */
void AravisDetectorPlugin::get_frame_rate(){
  GErrorWrapper error;
  double temp = arv_camera_get_frame_rate(camera_, error.get());
  if(error){ 
    log_error("When reading frame rate the following error ocurred: \n" + error.message());
    return;
  }

  frame_rate_hz_ = temp;
}


/** @brief Set pixel format
 * 
 * use read_config() to see which pixel formats are supported
 * 
 * @param pixel_format string representation of the format (eg, Mono8, Mono12, RGB8)
 */
void AravisDetectorPlugin::set_pixel_format(std::string pixel_format, OdinData::IpcMessage& reply){
  GErrorWrapper error;

  arv_camera_set_pixel_format_from_string(camera_, pixel_format.c_str(), error.get());

  if(error){
    log_error("When setting pixel format the following error ocurred: \n" + error.message(), reply);
    return;
  }
  
  LOG4CXX_INFO(logger_, "pixel_format_ | old: "<< pixel_format_ << " | new:" << pixel_format);
  pixel_format_ = pixel_format;
}

/** @brief Get a list of available pixel formats
 * 
 * the list is saved as a string with the values indexed and a separated by newline, eg:
 * #1 Mono8
 * #2 Mono12
 * #3 RGB8
 * 
 * Saved in available_pixel_formats_
 */
void AravisDetectorPlugin::get_available_pixel_formats(){
  GErrorWrapper error;
  unsigned int temp; 
  const char** formats_temp = arv_camera_dup_available_pixel_formats_as_strings(camera_,&temp, error.get());

  if(error){
    log_error("When reading pixel formats the following error occurred: \n" + error.message());
    return;
  }

  n_pixel_formats_ =  temp;
  available_pixel_formats_ = "\n"; // clean from old strings and start new line
  if(n_pixel_formats_ > 1){
    for(int i=0; i< n_pixel_formats_; i++){
      available_pixel_formats_ += "#" + std::to_string(i+1) + " " + formats_temp[i] + "\n";
    }
  }else{
    available_pixel_formats_.append(formats_temp[0]);
  }
      
  free(formats_temp); // only need to free the container
}

/** @brief Get currently used pixel format from the camera
 *
 * The pixel format is saved in pixel_format_ variable as a string
 */
void AravisDetectorPlugin::get_pixel_format(){
  GErrorWrapper error;
  std::string temp = arv_camera_get_pixel_format_as_string(camera_, error.get());

  if(error){ 
    log_error("When reading current the pixel format the following error occurred: \n" + error.message());
      return;
    }

  pixel_format_ = temp;
}


void AravisDetectorPlugin::get_frame_size(){
  GErrorWrapper error;
  int temp = arv_camera_get_payload(camera_, error.get());

  if(error){    
    log_error("When getting frame size the following error occurred: \n" + error.message());
    return;
  }

  payload_ = temp;
}


/**********************************
**    Stream/buffer functions    **
***********************************/

/** @brief Creates a stream object and starts camera acquisition
 * 
 * This function does the following (in the order displayed):
 * - Checks camera is connected
 * - Deletes any previous stream objects
 * - Initializes a new stream
 * - Checks for errors
 * - Adds buffers
 * - starts camera acquisition and the buffer reading function
 */
void AravisDetectorPlugin::start_stream(OdinData::IpcMessage& reply){
  GErrorWrapper error;
  
  // check you are connected to a camera
  if (!ARV_IS_CAMERA(camera_)){
    log_error("Cannot start stream without connecting to a camera first.", reply);
    return;}

  // delete old stream
  if(stream_ != NULL){
    LOG4CXX_INFO(logger_, "Removing old stream");
    g_object_unref(stream_);
    stream_ = NULL;}

  // set camera on stream mode
  get_acquisition_mode();
  if(acquisition_mode_!="Continuous")
    set_acquisition_mode("Continuous", reply); 
  
  // create the stream object
  stream_ = arv_camera_create_stream (camera_, NULL, NULL, error.get());
  
  if(error){
    log_error("When creating camera stream the following error ocurred: \n" + error.message(), reply);
    return;}
  if(stream_== NULL){
    log_error("Stream was not initialized, error undetected", reply);
    return;}

  // and populate it with a few empty buffers (frames)
  for(int i =0; i<n_empty_buffers_; i++){
    arv_stream_push_buffer(stream_, arv_buffer_new(payload_, NULL));
  }

  // stream callback mechanism
  arv_stream_set_emit_signals (stream_, TRUE);
  g_signal_connect (stream_, "new-buffer", G_CALLBACK (buffer_callback), this);

  // Start the stream
  streaming_= true;
  n_frames_made_ = 0;
  arv_camera_start_acquisition (camera_, error.get());

  if(error)
    log_error("When starting buffer acquisition the following error occurred: \n" + error.message(), reply);

}

/** @brief Stop acquisition and destruct stream
 * 
 * use when Ipc Messages are required (config)
 * otherwise use auto_stop_stream
 * 
 * @param reply Ipc Message from config
 */
void AravisDetectorPlugin::stop_stream(OdinData::IpcMessage& reply){
  GErrorWrapper error;

  if(camera_ == NULL || stream_ == NULL){
    log_error("There is no stream to stop. Exiting process", reply);
    return;
  }

  arv_stream_set_emit_signals (stream_, FALSE);


  arv_camera_stop_acquisition (camera_, error.get());
  streaming_ = false;
  g_object_unref(stream_);
  stream_ = NULL;
  if(error){
    log_error("Stream acquisition failed to stop, error : \n" + error.message(), reply);
    return;
    }
  LOG4CXX_INFO(logger_,"Stopping continuous camera acquisition");
}

/** @brief Stops stream without ipc message
 * 
 * Identical to stop_stream but without a reply.
*/
void AravisDetectorPlugin::auto_stop_stream(){
  GErrorWrapper error;

  if(camera_ == NULL || stream_ == NULL){
    log_error("There is no stream to stop. Exiting process");
    return;
  }

  arv_stream_set_emit_signals (stream_, FALSE);

  arv_camera_stop_acquisition (camera_, error.get());
  streaming_ = false;
  g_object_unref(stream_);
  stream_ = NULL;
  if(error){
    log_error("Stream acquisition failed to stop, error : \n" + error.message());
    return;
    }
  LOG4CXX_INFO(logger_,"Reached " << frame_count_ <<" frames, stopping continuous camera acquisition");
}


/** @brief Sets maximum number of frames taken in stream mode
 * @param frame_count unsigned int: frame limit
 */
void AravisDetectorPlugin::set_frame_count(unsigned int frame_count, OdinData::IpcMessage& reply){
  LOG4CXX_INFO(logger_, "frame_count_ | old: "<< frame_count_ << " | new:" << frame_count);
  frame_count_ = frame_count;
}


/** @brief processes a fixed number of buffers
 * 
 */
void AravisDetectorPlugin::acquire_n_buffer(unsigned int n_buffers, OdinData::IpcMessage& reply){

  if(n_buffers == 1){
    set_acquisition_mode("SingleFrame", reply); // change to single frame
    acquire_buffer();
    return;
  }

  // check you are connected to a camera
  if (!ARV_IS_CAMERA(camera_)){
    log_error("Cannot start stream without connecting >to a camera first.", reply);
    return;}

  // set camera on stream mode
  get_acquisition_mode();
  if(acquisition_mode_!="MultiFrame")
    set_acquisition_mode("MultiFrame", reply); 
  set_frame_count(n_buffers, reply);

  for(int i=1; i<=n_buffers; i++){
    acquire_buffer();
  }
}

/** @brief Captures a frame buffer
 * 
 */
void AravisDetectorPlugin::acquire_buffer(){
  GErrorWrapper error;
  ArvBuffer *buffer;

  if (!ARV_IS_CAMERA (camera_)){
    log_error("Cannot acquire buffer without connecting to a camera first.");
    return;}

  if (!ARV_IS_STREAM (stream_)){
    log_error("Cannot acquire buffer without initialising a stream first");
    return;}

  buffer = arv_stream_pop_buffer(stream_);

  if(buffer_is_valid(buffer)){
    process_buffer(buffer);
  }

  // for stream we need to replenish the buffers
  arv_stream_push_buffer(stream_, buffer);
 }

/** @brief Captures one frame buffer from a continuos stream
 * 
 * Checks the buffer for errors and sends it to the frame processor if it is ok.
 * 
 * Essentially calls arv_buffer_get_status and sends the result through a switch
 */
bool AravisDetectorPlugin::buffer_is_valid(ArvBuffer *buffer){
  bool buffer_state = false;
  // if buffer is empty then it isn't finished.
  if (!ARV_IS_BUFFER (buffer))
    log_error("Buffer is empty");

  switch(arv_buffer_get_status(buffer)){
    case ARV_BUFFER_STATUS_SUCCESS:
      buffer_state = true;
      break;
    case ARV_BUFFER_STATUS_UNKNOWN:
      log_error("Error when getting the buffer: status unknown");
      break;
    case ARV_BUFFER_STATUS_TIMEOUT:
      log_error("Error when getting the buffer: timeout");
      break;
    case ARV_BUFFER_STATUS_MISSING_PACKETS:
      log_error("Error when getting the buffer: missing packets.");
      break;
    case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
      log_error("Error when getting the buffer: wrong packet id");
      break;
    case ARV_BUFFER_STATUS_SIZE_MISMATCH:
      log_error("Error when getting the buffer: size mismatch");
      break;
    case ARV_BUFFER_STATUS_FILLING:
      log_error("Error when getting the buffer: status still filling");
      break;
    case ARV_BUFFER_STATUS_ABORTED:
      log_error("Error when getting the buffer: aborted");
      break;
    case ARV_BUFFER_STATUS_PAYLOAD_NOT_SUPPORTED:
      log_error("Error when getting the buffer: payload not supported");
      break;
    default:
      log_error("Error when getting the buffer: unexpected buffer status");
  }
  return buffer_state;
 }


/** @brief Changes buffer object to DataBlockFrame object pointer. 
 * 
 */
void AravisDetectorPlugin::process_buffer(ArvBuffer *buffer){

  data_type_ = pixel_format_to_datatype(pixel_format_);

  // Will need to change this at some point 
  image_height_px_ = arv_buffer_get_image_height(buffer);
  image_width_px_ = arv_buffer_get_image_width(buffer);
  if(frame_dimensions_.empty()){
    frame_dimensions_.push_back(image_height_px_);
    frame_dimensions_.push_back(image_width_px_);
  }else{
    frame_dimensions_[0]=image_height_px_;
    frame_dimensions_[1]=image_width_px_;
  }

  
  FrameMetaData metadata(n_frames_made_, "data", data_type_, "", frame_dimensions_, compression_type_);
  boost::shared_ptr<DataBlockFrame> new_frame(new DataBlockFrame(metadata, arv_buffer_get_image_data(buffer, &payload_), payload_, image_data_offset_));

  if (frame_count_ > 0){
    if (n_frames_made_ >= frame_count_){
      // Multi frame mode and we have already processed the correct number of frames
      // Do not push this frame and stop the streaming
      boost::thread *task_ptr = new boost::thread(&AravisDetectorPlugin::auto_stop_stream, this);
      return;
    }
  }
  process_frame(new_frame);
  n_frames_made_++;
}


/** @brief Saves information about the stream_
 * 
 * Saves the number of input and output buffers as well as the number
 * of successful, failed and underrun buffers to their respective class
 * variables.
 */
void AravisDetectorPlugin::get_stream_state(){

  if(stream_==NULL){
    log_error("Stream not initialized, cannot get stream state");
    return;
  }

  arv_stream_get_n_buffers(stream_, &n_input_buff_, &n_output_buff_);
  arv_stream_get_statistics(stream_, &n_completed_buff_, &n_failed_buff_, &n_underrun_buff_);
}


/** @brief Saves the xml file from camera_ into filepath/serial-number.xml
 * 
 */
void AravisDetectorPlugin::save_genicam_xml(std::string filepath){ 
  size_t xml_length;
  std::string filename{temp_file_path_ + camera_model_+".xml"};
  std::ofstream xml_file(filename.c_str());
  xml_file << arv_device_get_genicam_xml(arv_camera_get_device(camera_), &xml_length);
  xml_file.close();
  LOG4CXX_INFO(logger_,"Saving xml config to "<<filename);

}

/** @brief Translates from the available pixel format to datatype
 * 
 * TODO: add support for more pixel formats
 * 
 * @param pixel_form 
 * @return DataType 
 */
DataType AravisDetectorPlugin::pixel_format_to_datatype(std::string pixel_form){
  if(pixel_form == "Mono8")
    return DataType::raw_8bit;
  if(pixel_form == "Mono12")
    log_error("Pixel type unsupported, return unkown");
  if(pixel_form == "RGB8")
    return DataType::raw_8bit;
  

  return DataType::raw_unknown;
}

/********************************
**          Version            **
*********************************/

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