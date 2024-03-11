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
    void set_exposure(float exposure_time_us);
    void display_aravis_cameras();
    void connect_aravis_camera(std::string ip);
    int get_version_major();
    int get_version_minor();
    int get_version_patch();
    std::string get_version_short();
    std::string get_version_long();

    /**Default Config Values*/
    static const std::string DEFAULT_CAMERA_IP; ///< Ip address of the current camera
    static const float    DEFAULT_EXPOSURE_TIME;  ///< Exposure time in microseconds

    /*Config names*/
    static const std::string LIST_DEVICES;      ///< list available devices
    static const std::string CONFIG_CAMERA_IP;  ///< set camera IP
    static const std::string CONFIG_EXPOSURE;   ///< set exposure time


private:

    LoggerPtr logger_;      ///< Pointer to logger object for displaying info in terminal

    boost::thread *thread_; ///< Pointer to status thread
    bool working_;          ///< Is the status thread working?

    GError *error = NULL;   ///< Pointer to GError object for aravis camera errors
    ArvCamera *camera;      ///< Pointer to ArvCamera object
	ArvBuffer *buffer;      ///< Pointer to ArvBuffer object. It holds frames/packets from the camera

    unsigned int number_of_cameras; ///< total number of cameras connected.

};

} // namespace 
#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/