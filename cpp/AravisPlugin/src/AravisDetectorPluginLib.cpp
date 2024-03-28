/**
 * @file AravisDetectorPluginLib.cpp
 * @brief Registers the class 
 * @date 2024-03-04
 * 
 * Adapted from odin-data, LiveViewPluginLib.cpp by Ashley Neaves
 */

#include "AravisDetectorPlugin.h"
#include "ClassLoader.h"

namespace FrameProcessor
{
  /**
   * Registration of this plugin through the ClassLoader.  This macro
   * registers the class without needing to worry about name mangling
   */
  REGISTER(FrameProcessorPlugin, AravisDetectorPlugin, "AravisDetectorPlugin");

} // namespace FrameProcessor



