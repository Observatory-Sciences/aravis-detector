set(CMAKE_INCLUDE_CURRENT_DIR on)
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

include_directories(${DATA_DIR}/include ${ODINDATA_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${LOG4CXX_INCLUDE_DIRS}/.. ${ZEROMQ_INCLUDE_DIRS})

# Add library for AravisDetector plugin
add_library(AravisDetectorPlugin SHARED AravisDetectorPlugin.cpp AravisDetectorPluginLib.cpp)
target_link_libraries(AravisDetectorPlugin ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES} ${ZEROMQ_LIBRARIES} ${COMMON_LIBRARY} ${ODINDATA_LIBRARIES})
install(TARGETS AravisDetectorPlugin DESTINATION lib)