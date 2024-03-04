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

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;


#include "FrameProcessorPlugin.h"
#include "ClassLoader.h"

namespace FrameProcessor
{

class AravisDetectorPlugin : public FrameProcessorPlugin{

public:

    AravisDetectorPlugin();
    virtual ~AravisDetectorPlugin();
    void process_frame(boost::shared_ptr<Frame> frame);
    void configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply);
    // int get_version_major();
    // int get_version_minor();
    // int get_version_patch();
    // std::string get_version_short();
    // std::string get_version_long();

    // Default values and names TBA

private:

    /** Pointer to logger */
    LoggerPtr logger_;
    OdinData::IpcChannel publish_socket_;
    /**Boolean that shows if the plugin has a successfully bound ZMQ endpoint*/
    bool is_bound_;

};

} // namespace 

#endif /* FRAMEPROCESSOR_ARAVISDETECTORPLUGIN_H_*/