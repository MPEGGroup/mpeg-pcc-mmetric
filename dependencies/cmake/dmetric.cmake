cmake_minimum_required(VERSION 3.17)

include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/CPM.cmake )

set( DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/dmetric )
if( NOT EXISTS ${DIR} )
  CPMAddPackage( NAME             dmetric
                GIT_REPOSITORY    https://git.mpeg.expert/MPEG/3dgh/v-pcc/software/mpeg-pcc-dmetric.git
                GIT_TAG           release-v0.14.1
                SOURCE_DIR        ${DIR}
                DOWNLOAD_ONLY     YES )
endif()
