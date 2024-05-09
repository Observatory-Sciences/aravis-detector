/**
 * @file AravisDetectorPlugin.h
 * @brief Header file for the Aravis Plugin
 * @date 2024-03-04
 * Adapted from odin-data, LiveViewPlugin.h by Ashley Neaves
 */

#ifndef FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_
#define FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_

#define GET_CONFIG_CAMERA_INIT 1
#define GET_CONFIG_CAMERA_PARAMS 2
#define GET_CONFIG_STREAM_STAT 3
#define GET_CONFIG_ALL 4

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <map>
#include <sys/stat.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

#include "FrameProcessorPlugin.h"
#include "DataBlockFrame.h"
#include "ClassLoader.h"
#include <fstream>

extern "C" {
    #include "arv.h"
}

namespace FrameProcessor
{

class AravisDetectorPlugin : public FrameProcessorPlugin{

public:

    AravisDetectorPlugin();
    virtual ~AravisDetectorPlugin();
    void process_frame(boost::shared_ptr<Frame> frame);
    void configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply);
    void requestConfiguration(OdinData::IpcMessage& reply);
    void status(OdinData::IpcMessage& status);
    bool reset_statistics();
    void status_task();
    void callback_access(ArvStream *stream_temp); 

    int get_version_major();
    int get_version_minor();
    int get_version_patch();
    std::string get_version_short();
    std::string get_version_long();

    /** Default Config Values*/
    static const std::string DEFAULT_CAMERA_IP;     ///< Ip address of the current camera
    static const std::string DEFAULT_PIXEL_FORMAT;  ///< Default pixel encoding
    static const std::string DEFAULT_CAMERA_ID;     ///< Default camera ID
    static const std::string DEFAULT_CAMERA_SERIAL; ///< Default serial number of camera 
    static const std::string DEFAULT_CAMERA_MODEL;  ///< Default camera model
    static const std::string DEFAULT_FILE_PATH;     ///< Default temporary file path
    static const std::string DEFAULT_DATASET;       ///< Default data set name
    static const std::string DEFAULT_FILE_NAME;     ///< Default data file name 
    static const std::string DEFAULT_AQUISIT_MODE;  ///< Default acquisition mode 
    static const double      DEFAULT_EXPOSURE_TIME; ///< Exposure time in microseconds
    static const size_t      DEFAULT_STATUS_FREQ;   ///< Time between status checks in miliseconds
    static const double      DEFAULT_FRAME_RATE;    ///< Frame rate in hertz
    static const unsigned int DEFAULT_FRAME_COUNT;   ///< Frame count
    static const int         DEFAULT_EMPTY_BUFF;    ///< Number of empty buffers used to initialize the stream 

    /** Flags*/
    static const std::string START_STREAM;          ///< starts continuos mode acquisition   
    static const std::string STOP_STREAM;           ///< stops continuos mode acquisition
    static const std::string LIST_DEVICES;          ///< list available devices
    static const std::string ACQUIRE_BUFFER;        ///< acquire an image buffer from the camera

    /** Config names*/
    static const std::string READ_CONFIG;           ///< returns config values for the current connected camera
    static const std::string CONFIG_CAMERA_IP;      ///< set camera IP
    static const std::string CONFIG_EXPOSURE;       ///< set exposure time in microseconds
    static const std::string CONFIG_FRAME_RATE;     ///< set frame rate in hz
    static const std::string CONFIG_FRAME_COUNT;    ///< set frame count
    static const std::string CONFIG_PIXEL_FORMAT;   ///< set pixel encoding Mono8/ 12bit/ etc
    static const std::string CONFIG_ACQUISITION_MODE;///< set the camera acquisition mode: "Continuous", "SingleFrame","MultiFrame"
    static const std::string CONFIG_CALLBACK;       ///< Choose weather to activate the Aravis callback mechanism for frame acquisition
    static const std::string CONFIG_STATUS_FREQ;    ///< set the status polling frequency in miliseconds
    static const std::string CONFIG_EMPTY_BUFF;     ///< number of empty buffers in a stream object 
    static const std::string CONFIG_CAMERA_ID;      ///< camera's manufacturer id
    static const std::string CONFIG_CAMERA_SERIAL;  ///< camera's serial number
    static const std::string CONFIG_CAMERA_MODEL;   ///< camera's model

    /** Names and settings */
    static const std::string DATA_SET_NAME;         ///< name of data set used in frame creation
    static const std::string FILE_NAME;             ///< file name used in frame creation
    static const std::string COMPRESSION_TYPE;      ///< compression type used
    static const std::string TEMP_FILES_PATH;       ///< a location to store temporary files like camera xml


private:

    /*********************************
    **       Plugin Functions       **
    **********************************/

    void log_error(std::string msg, OdinData::IpcMessage& reply);
    void log_error(std::string msg);
    void log_warning(std::string msg, OdinData::IpcMessage& reply);
    void log_warning(std::string msg);

    void get_config(int32_t get_option);

    void set_file_name(std::string file_id,  OdinData::IpcMessage& reply);
    void set_file_path(std::string new_file_path, OdinData::IpcMessage& reply);
    void set_dataset_name(std::string data_set_name,  OdinData::IpcMessage& reply);
    void set_compression_type(std::string compression_type,  OdinData::IpcMessage& reply);
    void set_status_poll_frequency(size_t new_frequency,  OdinData::IpcMessage& reply);
    
    /*********************************
    **       Camera Functions       **
    **********************************/

    void connect_aravis_camera(std::string ip, OdinData::IpcMessage& reply); 
    void check_connection();
    void find_aravis_cameras(OdinData::IpcMessage& reply);
    void get_camera_serial();
    void get_camera_id();

    void set_acquisition_mode(std::string acq_mode, OdinData::IpcMessage& reply);
    void get_acquisition_mode();

    void set_exposure(double exposure_time_us, OdinData::IpcMessage& reply);
    void get_exposure_bounds();
    void get_exposure();

    void set_frame_rate(double frame_rate_hz, OdinData::IpcMessage& reply);
    void get_frame_rate_bounds();
    void get_frame_rate();

    void set_pixel_format(std::string pixel_format,OdinData::IpcMessage& reply);
    void get_available_pixel_formats();
    void get_pixel_format();

    void get_frame_size();

    /**********************************
    **    Stream/buffer functions    **
    ***********************************/

    void start_stream(OdinData::IpcMessage& reply);
    void stop_stream(OdinData::IpcMessage& reply);
    void auto_stop_stream();

    void set_frame_count(unsigned int frame_count, OdinData::IpcMessage& reply);
    void set_empty_buffers(int n_empty_buffers, OdinData::IpcMessage& reply);


    void acquire_n_buffer(unsigned int n_buffers, OdinData::IpcMessage& reply);
    void acquire_buffer();
    bool buffer_is_valid(ArvBuffer *buffer);
    void process_buffer(ArvBuffer *buffer);
    
    void get_stream_state();
    
    void save_genicam_xml(std::string filepath);


    DataType pixel_format_to_datatype(std::string pixel_form);


    /*********************************
    **        Plugin states         **
    **********************************/

    LoggerPtr logger_;                                  ///< Pointer to logger object for displaying info in terminal
    boost::thread *thread_;                             ///< Pointer to status thread
    bool working_;                                      ///< Is the status thread working?
    bool streaming_;                                    ///< Is the camera streaming data?
    bool camera_connected_;                             ///< is the camera connected?
    
    size_t status_freq_ms_ {DEFAULT_STATUS_FREQ};        ///< delay between config queries in milliseconds  
    std::string temp_file_path_{DEFAULT_FILE_PATH};     ///< temporary file path for  


    /*********************************
    **       Camera parameters      **
    **********************************/

    ArvCamera *camera_;                                 ///< Pointer to ArvCamera object
    int connected_devices_ {0};
    std::map<std::string, std::pair<std::string, std::string>> available_cameras_;  ///< camera index (camera id, ip address)
    std::string camera_id_ {DEFAULT_CAMERA_ID};         ///< camera device id
    std::string camera_serial_ {DEFAULT_CAMERA_SERIAL}; ///< camera serial number
    std::string camera_address_ {DEFAULT_CAMERA_IP};    ///< camera address
    std::string camera_model_{DEFAULT_CAMERA_MODEL};    ///< camera model

    double exposure_time_us_ {DEFAULT_EXPOSURE_TIME};   ///< current exposure time in microseconds
    double min_exposure_time_ {};                       ///< minimum exposure time in microseconds
    double max_exposure_time_ {};                       ///< maximum exposure time in microseconds

    double frame_rate_hz_ {DEFAULT_FRAME_RATE};         ///< current frame rate in hertz, default to 5
    double min_frame_rate_ {};                          ///< minimum frame rate in hertz
    double max_frame_rate_ {};                          ///< maximum frame rate in hertz

    unsigned int n_pixel_formats_ {};                   ///< total number of pixel formats
    std::string available_pixel_formats_ {};            ///< a set of available pixel formats in string form
    std::string pixel_format_ {DEFAULT_PIXEL_FORMAT};   ///< current pixel format

    std::string acquisition_mode_ {DEFAULT_AQUISIT_MODE};///< string describing current camera mode: "Continuous", "SingleFrame","MultiFrame"

    size_t payload_ {};                                 ///< frame size in bytes

    unsigned int frame_count_ {DEFAULT_FRAME_COUNT};    ///< current frame count in MultiFrame mode



    /**********************************
    **   Stream/buffer parameters    **
    ***********************************/

    ArvStream *stream_;                                 ///< Pointer to ArvStream object. For continuos frame acquisition

    DataType data_type_;                                ///< currently used data_type
    CompressionType compression_type_;                  ///< currently used compression type

    std::string data_set_name_ {DEFAULT_DATASET};       ///< name of the data set the plugin is writing to
    std::string file_id_  {DEFAULT_FILE_NAME};          ///< name of the file the plugin is writing to 
    int image_data_offset_{0};                          ///< size of buffer metadata (usually none)

    long long n_frames_made_ {0};                       ///< Number of frames created from buffers

    int n_empty_buffers_ {DEFAULT_EMPTY_BUFF};           ///< number of empty buffers to initialise the current stream with. Defaults to 50
    int n_input_buff_ {0};                              ///< n of input buffers in the current stream
    int n_output_buff_ {0};                             ///< n of output buffers in the current stream
    long unsigned int n_completed_buff_ {0};            ///< n of successful buffers
    long unsigned int n_failed_buff_ {0};               ///< n of failed buffers
    long unsigned int n_underrun_buff_ {0};             ///< n of buffers overwritten (stream ran out of empty buffers)

    unsigned long long image_height_px_{0};             ///< image height in pixels
    unsigned long long image_width_px_{0};              ///< image width in pixels
    std::vector<unsigned long long> frame_dimensions_;  ///< image dimensions for frame creation

};

} // namespace 
#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/