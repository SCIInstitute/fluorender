#This File will copy over all Dll's needed for Fluorender.

if(Debug_Mode)
	include(${CMAKE_CURRENT_LIST_DIR}/debugCopy.cmake)
else()
	include(releaseCopy.cmake)
endif()
