/**
 * @file AravisDetectorPlugin.h
 * @brief Header file for the Aravis Plugin
 * @date 2024-03-04
 * Adapted from odin-data, LiveViewPlugin.h by Ashley Neaves
 */

#ifndef FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_
#define FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

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
    static const double      DEFAULT_EXPOSURE_TIME; ///< Exposure time in microseconds
    static const double      DEFAULT_FRAME_RATE;    ///< Frame rate in hertz
    static const double      DEFAULT_FRAME_COUNT;   ///< Frame count
    static const bool        DEFAULT_CALLBACK;  ///< default callback value

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
    static const std::string CONFIG_QUERY_FREQ;     ///< miliseconds between querying the camera for config

    /** Names and settings */
    static const std::string DATA_SET_NAME;
    static const std::string FILE_NAME;
    static const std::string COMPRESSION_TYPE;


private:

    /*********************************
    **       Plugin Functions       **
    **********************************/

    void read_config(int32_t display_option);
    void get_config(int32_t get_option);

    /*********************************
    **       Camera Functions       **
    **********************************/

    void connect_aravis_camera(std::string ip); 
    void check_connection();
    void display_aravis_cameras();
    void get_camera_serial();
    void get_camera_id();

    void set_acquisition_mode(std::string acq_mode);
    void get_acquisition_mode();
    void set_aravis_callback(bool arv_callback);

    void set_exposure(double exposure_time_us);
    void get_exposure_bounds();
    void get_exposure();

    void set_frame_rate(double frame_rate_hz);
    void get_frame_rate_bounds();
    void get_frame_rate();

    void set_frame_count(double frame_count);
    void get_frame_count_bounds();
    void get_frame_count();

    void set_pixel_format(std::string pixel_format);
    void get_available_pixel_formats();
    void get_pixel_format();

    void get_frame_size();

    /**********************************
    **    Stream/buffer functions    **
    ***********************************/

    void start_stream();
    void stop_stream();

    void acquire_n_buffer(unsigned int n_buffers);
    void acquire_buffer();
    bool buffer_is_valid();
    void process_buffer();
    
    void get_stream_state();
    
    void save_frame_pgm();
    void save_genicam_xml(std::string filepath);


    DataType pixel_format_to_datatype(std::string pixel_form);


    /*********************************
    **        Plugin states         **
    **********************************/

    LoggerPtr logger_;                      ///< Pointer to logger object for displaying info in terminal
    boost::thread *thread_;                 ///< Pointer to status thread
    bool working_;                          ///< Is the status thread working?
    bool streaming_;                        ///< Is the camera streaming data?
    bool aravis_callback_;                  ///< Is the camera emitting signals when a buffer is finished?
    bool camera_connected_;                 ///< is the camera connected?
    
    size_t delay_ms_ {1000};                /// delay between config queries in milliseconds  

    /*********************************
    **       Camera parameters      **
    **********************************/

    ArvCamera *camera_;                     ///< Pointer to ArvCamera object
    std::string camera_id_ {"None"};        ///< camera device id
    std::string camera_serial_ {"None"};    ///< camera serial number
    std::string camera_address_ {"None"};   ///< camera address

    double exposure_time_us_;               ///< current exposure time in microseconds
    double min_exposure_time_;              ///< minimum exposure time in microseconds
    double max_exposure_time_;              ///< maximum exposure time in microseconds

    double frame_rate_hz_ {5};              ///< current frame rate in hertz, default to 5
    double min_frame_rate_;                 ///< minimum frame rate in hertz
    double max_frame_rate_;                 ///< maximum frame rate in hertz

    unsigned int n_pixel_formats_;          ///< total number of pixel formats
    std::string available_pixel_formats_;   ///< a set of available pixel formats in string form
    std::string pixel_format_;              ///< current pixel format

    std::string acquisition_mode_;          ///< string describing current camera mode: "Continuous", "SingleFrame","MultiFrame"

    size_t payload_;                        ///< frame size in bytes

    unsigned int frame_count_;              ///< current frame count in MultiFrame mode
    unsigned int min_frame_count_;          ///< current frame count in MultiFrame mode
    unsigned int max_frame_count_;          ///< current frame count in MultiFrame mode


    /**********************************
    **   Stream/buffer parameters    **
    ***********************************/

	ArvBuffer *buffer_;                     ///< Pointer to ArvBuffer object. It holds frames/packets from the camera
    ArvStream *stream_;                     ///< Pointer to ArvStream object. For continuos frame acquisition

    DataType data_type_;                    ///< currently used data_type
    CompressionType compression_type_;      ///< currently used compression type

    std::string data_set_name_;             ///< name of the data set the plugin is writing to
    std::string file_id_;                   ///< name of the file the plugin is writing to 
    int image_data_offset_{0};              ///< size of buffer metadata (usually none)

    long long n_frames_made_ {0};           ///< Number of frames created from buffers

    int n_empty_buffers_{500};              ///< number of empty buffers to initialise the current stream with. Defaults to 50
    int n_input_buff_;                      ///< n of input buffers in the current stream
    int n_output_buff_;                     ///< n of output buffers in the current stream
    long unsigned int n_completed_buff_ {0};///< n of successful buffers
    long unsigned int n_failed_buff_ {0};   ///< n of failed buffers
    long unsigned int n_underrun_buff_ {0}; ///< n of buffers overwritten (stream ran out of empty buffers)

    unsigned long long image_height_px_{0}; ///< image height in pixels
    unsigned long long image_width_px_{0};  ///< image width in pixels
    
    std::vector<unsigned long long> frame_dimensions_;

};

} // namespace 
#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/