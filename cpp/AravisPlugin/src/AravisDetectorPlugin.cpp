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
  const std::string AravisDetectorPlugin::DEFAULT_CAMERA_IP     = "tcp://127.0.0.1:3956";
  const std::string AravisDetectorPlugin::DEFAULT_PIXEL_FORMAT  = "Mono8";
  const double      AravisDetectorPlugin::DEFAULT_EXPOSURE_TIME = 1000.0;
  const double      AravisDetectorPlugin::DEFAULT_FRAME_RATE    = 60;
  const double      AravisDetectorPlugin::DEFAULT_FRAME_COUNT   = 1;
  
  /** Flags*/
  const std::string AravisDetectorPlugin::START_STREAM        = "start";
  const std::string AravisDetectorPlugin::STOP_STREAM         = "stop";
  const std::string AravisDetectorPlugin::LIST_DEVICES        = "list_devices";
  const std::string AravisDetectorPlugin::ACQUIRE_BUFFER      = "photo";

  /** Config names*/
  const std::string AravisDetectorPlugin::READ_CONFIG         = "read_config";
  const std::string AravisDetectorPlugin::CONFIG_CAMERA_IP    = "ip_address";
  const std::string AravisDetectorPlugin::CONFIG_EXPOSURE     = "exposure";
  const std::string AravisDetectorPlugin::CONFIG_FRAME_RATE   = "frame_rate";
  const std::string AravisDetectorPlugin::CONFIG_FRAME_COUNT  = "frame_count";
  const std::string AravisDetectorPlugin::CONFIG_PIXEL_FORMAT = "pixel_format";


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
	g_clear_object (&camera_);
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

/** @brief Implements json configurations
 * 
 * @param[in] config - IpcMessage containing configuration data
 * @param[out] reply - Response IpcMessage
 */
void AravisDetectorPlugin::configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply){
  try{
    /** List all devices*/
    if (config.has_param(START_STREAM)) start_stream();
    if (config.has_param(STOP_STREAM)) stop_stream();
    if (config.has_param(LIST_DEVICES)) display_aravis_cameras();
    if (config.has_param(ACQUIRE_BUFFER)){
        acquire_single_buffer();
        save_frame_pgm();
    }
    if (config.has_param(CONFIG_CAMERA_IP)){
      connect_aravis_camera(config.get_param<std::string>(CONFIG_CAMERA_IP));
    }
    if (config.has_param(CONFIG_EXPOSURE)){
      set_exposure(config.get_param<double>(CONFIG_EXPOSURE));
    }
    if (config.has_param(CONFIG_FRAME_RATE)){
      set_frame_rate(config.get_param<double>(CONFIG_FRAME_RATE));
    }
    if (config.has_param(CONFIG_FRAME_COUNT)){
      set_frame_count(config.get_param<int32_t>(CONFIG_FRAME_COUNT));
    }
    if (config.has_param(CONFIG_PIXEL_FORMAT)){
      set_pixel_format(config.get_param<std::string>(CONFIG_PIXEL_FORMAT));
    }
    if (config.has_param(READ_CONFIG)){
      read_config(config.get_param<int32_t>(READ_CONFIG));
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

/** @brief Queries the Aravis Camera for config info
 * In progress
 * Gets config info and displays it.
 * 
 * In the future it send queries at a preset frequency and stor the info locally.
 * Users will only be able to get the local info
 */
void AravisDetectorPlugin::read_config(int32_t display_option){
    switch(display_option){
      case 1:
        LOG4CXX_INFO(logger_, "The exposure time bounds are min: " << expo_min_ << " and max: "<< expo_max_);
        LOG4CXX_INFO(logger_, "The exposure time is set at " << exposure_time_us_ << " microseconds");
        break;
      case 2:
        LOG4CXX_INFO(logger_, "Frame rate is "<< frame_rate_hz_ << " frames per second");
        break;
      case 3:
        LOG4CXX_INFO(logger_, "Frame count is "<< frame_count_);
        break;
      case 4:
        LOG4CXX_INFO(logger_, "There are "<< n_pixel_formats_ <<" pixel formats: ");
        LOG4CXX_INFO(logger_, available_pixel_formats_);
        LOG4CXX_INFO(logger_, "Currently using "<< pixel_format_ <<" format");
      case 0:
        LOG4CXX_INFO(logger_, "The exposure time bounds are min: " << expo_min_ << " and max: "<< expo_max_);
        LOG4CXX_INFO(logger_, "The exposure time is set at " << exposure_time_us_ << " microseconds");
        LOG4CXX_INFO(logger_, "Frame rate is "<< frame_rate_hz_ << " frames per second");
        LOG4CXX_INFO(logger_, "Frame count is "<< frame_count_);
        LOG4CXX_INFO(logger_, "There are "<< n_pixel_formats_ <<" pixel formats: ");
        LOG4CXX_INFO(logger_, available_pixel_formats_);
        LOG4CXX_INFO(logger_, "Currently using "<< pixel_format_ <<" format");
        LOG4CXX_INFO(logger_, "Frame size: "<< payload_);
        LOG4CXX_INFO(logger_, "Camera acquisition mode is set on: "<< acquisition_mode_);
        break;
      default:
        LOG4CXX_WARN(logger_, "Please check get_frame parameter for spelling mistakes");
    }
}

/** @brief Populates config variables
 * 
 * This function gets called periodically to set config variables that can be
 * accessed by read_config. 
 * 
 * It calls the following:
 * 
 */
void AravisDetectorPlugin::get_config(){

  LOG4CXX_INFO(logger_, "acquiring config");
  get_frame_rate();
  get_exposure();
  get_pixel_format();
  get_acquisition_mode();
  get_frame_size(); // because encoding can change
}

/** @brief Status execution thread for this class.
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
  size_t delay_ms {1000/frame_rate_hz_}; // delay in milliseconds is the time between frames 
  int read_config_delay_seconds = 10; // number of seconds delay between config reads
  int read_config_delay= read_config_delay_seconds*(static_cast<int>(frame_rate_hz_)); // number of loops between config reads
  int i = 0;
  while (working_) {


    boost::this_thread::sleep(boost::posix_time::milliseconds(delay_ms));

    // Lock the camera object if required here
    i++;
    if (ARV_IS_CAMERA(camera_) && i>=read_config_delay) {
      // TODO: Change this example
      // Read out camera status items here and store to member variable cache
      i=0;
      get_config();
    }
    if(streaming_){
        acquire_stream_buffer();
      }
  }
  
}

/** @brief Captures one image buffer from the connected camera
 * 
 * After the buffer is acquired, it resets the acquisition mode to its previous value
 * 
 */
void AravisDetectorPlugin::acquire_single_buffer(){
  if (!ARV_IS_CAMERA (camera_))
    LOG4CXX_ERROR(logger_, "Cannot acquire buffer without connecting to a camera first.");
  GError *error = NULL;
  get_acquisition_mode();
  buffer_ = arv_camera_acquisition(camera_, 0, &error);
  if(error!=NULL)
    LOG4CXX_ERROR(logger_, "When acquiring image buffer from camera, the following error occurred: \n"<<error->message);
      
  set_acquisition_mode(acquisition_mode_); // make sure that the acquisition mode resets back to what it was 
}

/** @brief Captures one frame buffer from the a continuos stream
 * 
 */
void AravisDetectorPlugin::acquire_stream_buffer(){


  if (!ARV_IS_STREAM (stream_)){
    LOG4CXX_ERROR(logger_, "Cannot acquire buffer without initialising a stream first");
    return;}

  // use try_pop_buffer and not pop_buffer because try is thread safe
  buffer_ = arv_stream_try_pop_buffer(stream_);

  // if buffer is empty then it isn't finished.
  // this won't occur if we match frame rate
  if (!ARV_IS_BUFFER (buffer_))return;

  switch(arv_buffer_get_status(buffer_)){
    case ARV_BUFFER_STATUS_SUCCESS:
      save_frame_pgm();
      arv_stream_push_buffer(stream_, buffer_);
      break;
    case ARV_BUFFER_STATUS_UNKNOWN:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: status unknown");
      arv_stream_push_buffer(stream_, buffer_);
      break;
    case ARV_BUFFER_STATUS_TIMEOUT:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: timeout");
      break;
    case ARV_BUFFER_STATUS_MISSING_PACKETS:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: missing packets.");
      arv_stream_push_buffer(stream_, buffer_);
      break;
    case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: wrong packet id");
      arv_stream_push_buffer(stream_, buffer_);
      break;
    case ARV_BUFFER_STATUS_SIZE_MISMATCH:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: size mismatch");
      arv_stream_push_buffer(stream_, buffer_);
      break;
    case ARV_BUFFER_STATUS_FILLING:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: status still filling");
      break;
    case ARV_BUFFER_STATUS_ABORTED:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: aborted");
      arv_stream_push_buffer(stream_, buffer_);
      break;
    case ARV_BUFFER_STATUS_PAYLOAD_NOT_SUPPORTED:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: payload not supported");
      arv_stream_push_buffer(stream_, buffer_);
      break;
    default:
      LOG4CXX_ERROR(logger_, "Error when getting the buffer: unexpected buffer status");
      arv_stream_push_buffer(stream_, buffer_);
  }
 }

/** @brief Set acquisition mode for camera
 * 
 * @param acq_mode can be one of the following: "Continuous", "SingleFrame","MultiFrame"
 */
void AravisDetectorPlugin::set_acquisition_mode(std::string acq_mode){
  GError *error = NULL;
  ArvAcquisitionMode temp= arv_acquisition_mode_from_string(acq_mode.c_str());
  arv_camera_set_acquisition_mode(camera_, temp, &error);
  if(error==NULL){ 
    LOG4CXX_INFO(logger_, "Setting acquisition mode to " << acq_mode);
  }else{
    LOG4CXX_ERROR(logger_, "When getting acquisition mode the following error ocurred: \n" << error->message);
  }
}

void AravisDetectorPlugin::get_acquisition_mode(){
  GError *error = NULL;
  ArvAcquisitionMode temp = arv_camera_get_acquisition_mode(camera_, &error);
  if(error== NULL){
    acquisition_mode_ = arv_acquisition_mode_to_string(temp);
  }else{
    LOG4CXX_ERROR(logger_, "When getting acquisition mode the following error ocurred: \n" << error->message);
  }
  
}

void AravisDetectorPlugin::start_stream(){
  GError *error = NULL;
  
  // check you are connected 
  if (!ARV_IS_CAMERA(camera_)){
    LOG4CXX_ERROR(logger_, "Cannot start stream without connecting to a camera first.");
    return;}

  // delete old stream
  if(stream_ != NULL){
    LOG4CXX_INFO(logger_, "Removing old stream");
    g_object_unref(stream_);
    stream_ = NULL;}

  // set camera on stream mode
  set_acquisition_mode("Continuos"); 
  
  // create the stream object
  stream_ = arv_camera_create_stream (camera_, NULL, NULL, &error);
  
  if(error!=NULL || stream_ == NULL)
    LOG4CXX_ERROR(logger_, "When creating camera stream the following error ocurred: \n" << error->message);

  // and populate it with a few empty buffers (frames)
  for(int i =0; i<50; i++){
    arv_stream_push_buffer(stream_, arv_buffer_new(payload_, NULL));
  }

  // Start the stream
  streaming_= true;
  arv_camera_start_acquisition (camera_, &error);

  if(error!=NULL)
    LOG4CXX_ERROR(logger_, "When starting video stream the following error occurred: \n" << error->message);

}

void AravisDetectorPlugin::stop_stream(){
  GError *error = NULL;
  arv_camera_stop_acquisition (camera_, &error);
  streaming_ = false;
  if(error!=NULL)
    LOG4CXX_ERROR(logger_, "Stream acquisition failed to stop, error : \n" << error->message);
  LOG4CXX_INFO(logger_,"Stopping continuos camera acquisition");
}

/** @brief Set exposure time in microseconds
 * 
 * On success prints:
 *  Setting exposure time to <exposure_time_us>
 * On failure:
 *  When setting exposure time the following error ocurred: <error->message>
 * 
 * @param exposure_time_us
 */
void AravisDetectorPlugin::set_exposure(double exposure_time_us){
  GError *error = NULL;
  if(exposure_time_us <= expo_max_ && exposure_time_us >= expo_min_){
    arv_camera_set_exposure_time(camera_, exposure_time_us, &error);
    // check for errors
    if(error==NULL){ 
    // protect variables from junk data if there are errors
      LOG4CXX_INFO(logger_, "Setting exposure time to " << exposure_time_us);
    }else{
      LOG4CXX_ERROR(logger_, "When setting exposure time the following error ocurred: \n" << error->message);
    }
  }else{
    LOG4CXX_ERROR(logger_, "The exposure value "<< exposure_time_us << " is out of bounds: min="<<expo_min_<<"; max="<< expo_max_);
  }
}

/** @brief Get exposure time in microseconds
 * 
 * On success prints:
 *  Exposure time is <exposure_time_us>
 * On failure:
 *  When reading exposure time the following error ocurred: <error->message>
 */
void AravisDetectorPlugin::get_exposure(){
  GError *error = NULL;
  double temp = arv_camera_get_exposure_time(camera_, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    // check for error before changing cached
    exposure_time_us_ = temp;
  }else{
    LOG4CXX_ERROR(logger_, "When reading exposure time the following error ocurred: \n" << error->message);
  }
}

void AravisDetectorPlugin::get_exposure_bounds(){
  GError *error = NULL;
  double min_expo, max_expo;
  arv_camera_get_exposure_time_bounds(camera_, &min_expo, &max_expo, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    expo_max_ = max_expo;
    expo_min_ = min_expo;
  }else{
    LOG4CXX_ERROR(logger_, "When reading exposure time the following error ocurred: \n" << error->message);
  }
}

/** @brief Set frame rate in hertz
 * 
 * On success prints:
 *  Setting frame rate to <frame_rate_hz>
 * On failure:
 *  When setting frame rate the following error ocurred: <error->message>
 * 
 * @param frame_rate_hz
 */
void AravisDetectorPlugin::set_frame_rate(double frame_rate_hz){
  GError *error = NULL;

  arv_camera_set_frame_rate(camera_, frame_rate_hz, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    LOG4CXX_INFO(logger_, "Setting frame rate to "<< frame_rate_hz);
  }else{
    LOG4CXX_ERROR(logger_, "When setting frame rate the following error ocurred: \n" << error->message);
  }
}

/** @brief Get frame rate in hertz
 * 
 * On success prints:
 *  Frame rate is <frame_rate_hz>
 * On failure:
 *  When reading frame rate the following error ocurred: <error->message>
 */
void AravisDetectorPlugin::get_frame_rate(){
  GError *error = NULL;
  double temp = arv_camera_get_frame_rate(camera_, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    frame_rate_hz_ = temp;
  }else{
    LOG4CXX_ERROR(logger_, "When reading frame rate the following error ocurred: \n" << error->message);
  }
}

/** @brief Set frame count
 * 
 * On success prints:
 *  Setting frame count to <frame_count>
 * On failure:
 *  When setting frame count the following error ocurred: <error->message>
 * 
 * @param frame_count
 */
void AravisDetectorPlugin::set_frame_count(double frame_count){
  GError *error = NULL;
  arv_camera_set_frame_count(camera_, frame_count, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    LOG4CXX_INFO(logger_, "Setting frame count to "<< frame_count);
  }else{
    LOG4CXX_ERROR(logger_, "When setting frame count the following error ocurred: \n" << error->message);
  }
}

/** @brief Get frame count 
 * 
 * WARNING currently doesn't work for some reason 
 * 
 * On failure:
 *  When reading frame count the following error ocurred: <error->message>
 */
void AravisDetectorPlugin::get_frame_count(){
  GError *error = NULL;
  int32_t temp = arv_camera_get_frame_count(camera_, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    frame_count_ = temp;
  }else{
    LOG4CXX_ERROR(logger_, "When reading frame count the following error ocurred: \n" << error->message);
  }
}

/** @brief Set pixel format
 * 
 * use read_config() to see which pixel formats are supported
 * 
 * @param pixel_format string representation of the format (eg, Mono8, Mono12, RGB8)
 */
void AravisDetectorPlugin::set_pixel_format(std::string pixel_format){
  GError *error = NULL;
  arv_camera_set_pixel_format_from_string(camera_, pixel_format.c_str(), &error);
  if(error){
    LOG4CXX_ERROR(logger_, "When setting pixel format the following error ocurred: \n" << error->message);
  }
}

/** @brief Get currently used pixel format from the camera
 *
 * The pixel format is saved in pixel_format_ variable as a string
 */
void AravisDetectorPlugin::get_pixel_format(){
  GError *error = NULL;
  const char* temp = arv_camera_get_pixel_format_as_string(camera_, &error);
  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors
    pixel_format_ = temp;
  }else{
      LOG4CXX_ERROR(logger_, "Error, could not retrieve current pixel format");
    }
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
  GError *error = NULL;
  unsigned int temp; 
  const char** formats_temp = arv_camera_dup_available_pixel_formats_as_strings(camera_,&temp, &error);

  // check for errors
  if(error==NULL){ 
  // protect variables from junk data if there are errors 
    n_pixel_formats_ =  temp;
    available_pixel_formats_ = "\n"; // clean from old strings and start new line
    if(n_pixel_formats_ > 1){
      for(int i=0; i< n_pixel_formats_; i++){
        available_pixel_formats_ += "#" + std::to_string(i+1) + " " + formats_temp[i] + "\n";
      }
    }else{
        available_pixel_formats_.append(formats_temp[0]);
    }
  }else{
      LOG4CXX_ERROR(logger_, "Error, could not retrieve pixel formats");
      g_error_free(error);
    }
  free(formats_temp); // only need to free the container
}

void AravisDetectorPlugin::get_frame_size(){
  GError *error = NULL;
  int temp = arv_camera_get_payload(camera_, &error);
  if(error==NULL){
    payload_ = temp;
  }else{
      LOG4CXX_ERROR(logger_, "Error, could not retrieve frame size");
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
 * @param ip_string std::string of ip address
 */
void AravisDetectorPlugin::connect_aravis_camera(std::string ip_string){
  GError *error = NULL;
  arv_update_device_list();
  unsigned int number_of_cameras = arv_get_n_devices();

  if(number_of_cameras == 0){
    LOG4CXX_ERROR(logger_, "No aravis cameras found");
  }else{
      // there might be a better way to do this
      // arv_camera_new needs a char pointer, so I'm changing the string to a char
      const char *ip_copy = ip_string.c_str();
      camera_ = arv_camera_new(ip_string.c_str(), &error);
    if(error==NULL){ 
     
        /***************************************
        **      Camera init routine
        ****************************************/

        get_frame_size();
        get_pixel_format();
        get_available_pixel_formats();
        get_exposure_bounds();

      }else{
        LOG4CXX_ERROR(logger_, "Error when connecting to camera. Please confirm camera is connected");
      }
     if (ARV_IS_CAMERA (camera_)) {
       LOG4CXX_INFO(logger_,"Connected to camera " << arv_camera_get_model_name (camera_, NULL));
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
  unsigned int number_of_cameras = arv_get_n_devices();
  if(number_of_cameras==0){
    LOG4CXX_WARN(logger_, "No cameras were detected. Please confirm camera is connected");
  }else{
    LOG4CXX_INFO(logger_, number_of_cameras << " cameras were detected: \n ");
    for(int i=0; i<number_of_cameras; i++){
      const char* device_id = arv_get_device_id(i);
      const char* device_address = arv_get_device_address(i);
      LOG4CXX_INFO(logger_,"Device index "<< i <<" has the id "<<device_id<<" and address "<< device_address<<" \n");
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

/** @brief Saves current buffer_ to pgm format. For testing
 * 
 * There are better formats to use in practice
 */
void AravisDetectorPlugin::save_frame_pgm()
{ 
  static size_t bmg_count {1} ; // current number of buffers saved as bmg
  size_t img_size = 512*512;
  const u_char* decoded_frame = reinterpret_cast<u_char*>(const_cast<void*>(arv_buffer_get_image_data(buffer_, &img_size)));
  std::string filename {"/home/gsc/Github/RFI_Odin/_images/test_" + std::to_string(bmg_count)+ ".pgm"};

  FILE *pgmimg = fopen(filename.c_str(), "wb");
  fprintf(pgmimg, "P5\n%d %d\n255\n", 512, 512);
  fwrite(decoded_frame, sizeof(u_char), img_size, pgmimg);
  fclose(pgmimg);
  bmg_count++;

}
}