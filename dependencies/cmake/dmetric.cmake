cmake_minimum_required(VERSION 3.17)

include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/CPM.cmake )

include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/utils.cmake )

get_git_remote_origin_url( REMOTE_URL ${CMAKE_CURRENT_SOURCE_DIR} )

if(NOT "${REMOTE_URL}" STREQUAL "")
  # using relative url from parent path
  string(REGEX REPLACE "(.*)/.*" "\\1" REMOTE_PARENT_PATH "${REMOTE_URL}")
else()
  # set default repository URL for dmetric
  message(STATUS "using default URL for dmetric repository") 
  set(REMOTE_PARENT_PATH https://git.mpeg.expert/MPEG/3dgh/v-pcc/software)
endif()

check_git_repo(${REMOTE_PARENT_PATH}/mpeg-pcc-dmetric.git IS_VALID)

if (NOT IS_VALID)
  # fallback to default repository URL for dmetric
  message(STATUS "fallback to default URL for dmetric repository") 
  set(REMOTE_PARENT_PATH https://git.mpeg.expert/MPEG/3dgh/v-pcc/software)
endif()

message(STATUS "URL for dmetric repository : ${REMOTE_PARENT_PATH}/mpeg-pcc-dmetric.git")

set( DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/dmetric )
if( NOT EXISTS ${DIR} )
  CPMAddPackage( NAME             dmetric
                GIT_REPOSITORY    ${REMOTE_PARENT_PATH}/mpeg-pcc-dmetric.git
                GIT_TAG           release-v0.14.2
                SOURCE_DIR        ${DIR}
                DOWNLOAD_ONLY     YES )
endif()