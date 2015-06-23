# Copyright (C) 2015 Istituto Italiano di Tecnologia - iCub Facility
# Author: Marco Randazzo
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

SET(PROJECTNAME torqueObserver)

PROJECT(${PROJECTNAME})

SET(folder_source main.cpp)

SET(folder_header)

SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})

INCLUDE_DIRECTORIES(${YARP_INCLUDE_DIRS})

ADD_EXECUTABLE(${PROJECTNAME} ${folder_source} ${folder_header})

TARGET_LINK_LIBRARIES(${PROJECTNAME} ${YARP_LIBRARIES})

INSTALL(TARGETS ${PROJECTNAME} DESTINATION bin)



