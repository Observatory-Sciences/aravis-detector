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
#include "arv.h"

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

    /**Default Config Values*/
    static const std::string DEFAULT_CAMERA_IP;     ///< Ip address of the current camera
    static const std::string DEFAULT_PIXEL_FORMAT;  ///< Default pixel encoding
    static const float    DEFAULT_EXPOSURE_TIME;    ///< Exposure time in microseconds
    static const float    DEFAULT_FRAME_RATE;       ///< Frame rate in hertz
    static const float    DEFAULT_FRAME_COUNT;      ///< Frame count

    /*Config names*/
    static const std::string LIST_DEVICES;          ///< list available devices
    static const std::string GET_CONFIG;            ///< returns config values for the current connected camera

    static const std::string CONFIG_CAMERA_IP;      ///< set camera IP
    static const std::string CONFIG_EXPOSURE;       ///< set exposure time in microseconds
    static const std::string CONFIG_FRAME_RATE;     ///< set frame rate in hz
    static const std::string CONFIG_FRAME_COUNT;    ///< set frame count
    static const std::string CONFIG_PIXEL_FORMAT;   ///< set pixel encoding Mono8/ 12bit/ etc


private:

    void get_config(int32_t display_option);
    void display_aravis_cameras();
    void connect_aravis_camera(std::string ip);

    void set_exposure(double exposure_time_us);
    void get_exposure();

    void set_frame_rate(float frame_rate_hz);
    void get_frame_rate();

    void set_frame_count(float frame_count);
    void get_frame_count();

    void set_pixel_format(std::string pixel_format);
    void get_pixel_format();


    LoggerPtr logger_;      ///< Pointer to logger object for displaying info in terminal

    boost::thread *thread_; ///< Pointer to status thread
    bool working_;          ///< Is the status thread working?

    ArvCamera *camera_;      ///< Pointer to ArvCamera object
	ArvBuffer *buffer_;      ///< Pointer to ArvBuffer object. It holds frames/packets from the camera

};

} // namespace 
#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/