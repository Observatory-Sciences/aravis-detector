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
#include "ClassLoader.h"
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
    void status_task();


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


private:

    /*********************************
    **       Plugin Functions       **
    **********************************/

    void read_config(int32_t display_option);
    void get_config();

    /*********************************
    **       Camera Functions       **
    **********************************/

    void connect_aravis_camera(std::string ip); 
    void display_aravis_cameras();

    void set_acquisition_mode(std::string acq_mode);
    void get_acquisition_mode();

    void set_exposure(double exposure_time_us);
    void get_exposure_bounds();
    void get_exposure();

    void set_frame_rate(double frame_rate_hz);
    void get_frame_rate_bounds();
    void get_frame_rate();

    void set_frame_count(double frame_count);
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

    void acquire_single_buffer();
    void acquire_stream_buffer();
    
    void get_stream_state();
    
    void save_frame_pgm();


    /*********************************
    **        Plugin states         **
    **********************************/

    LoggerPtr logger_;                      ///< Pointer to logger object for displaying info in terminal
    boost::thread *thread_;                 ///< Pointer to status thread
    bool working_;                          ///< Is the status thread working?
    bool streaming_;                        ///< Is the camera streaming data?

    /*********************************
    **       Camera parameters      **
    **********************************/

    ArvCamera *camera_;                     ///< Pointer to ArvCamera object

    double exposure_time_us_;               ///< current exposure time in microseconds
    double expo_min_;                       ///< minimum exposure time in microseconds
    double expo_max_;                       ///< maximum exposure time in microseconds

    double frame_rate_hz_ {5};              ///< current frame rate in hertz, default to 5
    double min_frame_rate_;                 ///< minimum frame rate in hertz
    double max_frame_rate_;                 ///< maximum frame rate in hertz

    unsigned int n_pixel_formats_;          ///< total number of pixel formats
    std::string available_pixel_formats_;   ///< a set of available pixel formats in string form
    std::string pixel_format_;              ///< current pixel format

    std::string acquisition_mode_;          ///< string describing current camera mode: "Continuous", "SingleFrame","MultiFrame"

    unsigned int payload_;                  ///< frame size in bytes

    double frame_count_;                    ///< current frame count in MultiFrame mode

    /**********************************
    **   Stream/buffer parameters    **
    ***********************************/

	ArvBuffer *buffer_;                     ///< Pointer to ArvBuffer object. It holds frames/packets from the camera
    ArvStream *stream_;                     ///< Pointer to ArvStream object. For continuos frame acquisition

    int n_empty_buffers_{50};               ///< number of empty buffers to initialise the current stream with. Defaults to 50
    int n_input_buff_;                      ///< n of input buffers in the current stream
    int n_output_buff_;                     ///< n of output buffers in the current stream
    long unsigned int n_completed_buff_;    ///< n of successful buffers
    long unsigned int n_failed_buff_;       ///< n of failed buffers
    long unsigned int n_underrun_buff_;     ///< n of buffers overwritten (stream ran out of empty buffers)

};

} // namespace 
#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/