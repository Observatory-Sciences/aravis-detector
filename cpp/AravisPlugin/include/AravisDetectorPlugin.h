/**
 * @file AravisDetectorPlugin.h
 * @brief An Odin plugin for Aravis cameras
 * @date 2024-03-04
 * 
 * Currently a very simple version.
 * 
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
private:

    /** Pointer to logger */
    LoggerPtr logger_;
    /** Pointer to status thread */
    boost::thread *thread_;
    /** Is the status thread working */
    bool working_;

    /**Pointer to GError object defined as NULL so that it can save error when making a new camera object*/
    GError *error = NULL;
    /**Pointer to ArvCamera object that will hold the connection*/
    ArvCamera *camera;
	/**Pointer to ArvBuffer object. It holds frames/packets from the camera*/
	ArvBuffer *buffer;
    /**Image size information, irrelevant for now*/
    size_t img_size = 512*512;
};

} // namespace 

#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/