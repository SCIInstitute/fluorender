# Use the qtadvanceddocking_DIR

if(DEFINED qtadvanceddocking_DIR)

  if(EXISTS ${qtadvanceddocking_DIR})

    if(EXISTS ${qtadvanceddocking_DIR}/include)
      set(qtadvanceddocking_INCLUDE_DIR ${qtadvanceddocking_DIR}/include)
    endif(EXISTS ${qtadvanceddocking_DIR}/include)

    if(EXISTS ${qtadvanceddocking_DIR}/lib)
      set(qtadvanceddocking_LIBRARY_DIR ${qtadvanceddocking_DIR}/lib)

      set(qtadvanceddocking_LIBRARIES qtadvanceddocking)

      set(qtadvanceddocking_FOUND true CACHE BOOL "Found Qt ADS lib")

      foreach(LIB ${qtadvanceddocking_LIBRARIES})
        find_library(LIB ${LIB} PATHS ${qtadvanceddocking_LIBRARY_DIR} NO_DEFAULT_PATH)

        if(NOT LIB)
          unset(qtadvanceddocking_LIBRARIES)
          set(qtadvanceddocking_FOUND false)

          if(qtadvanceddocking_FIND_REQUIRED)
            message(FATAL_ERROR "ADS library ${LIB} not found in ${qtadvanceddocking_LIBRARY_DIR}")
          else(qtadvanceddocking_FIND_REQUIRED)
            message(WARNING "ADS library ${LIB} not found in ${qtadvanceddocking_LIBRARY_DIR}")
	    unset(qtadvanceddocking_FOUND)
          endif(qtadvanceddocking_FIND_REQUIRED)
        endif(NOT LIB)
      endforeach(LIB ${qtadvanceddocking_LIBRARIES})

    else(EXISTS ${qtadvanceddocking_DIR}/lib)
      unset(qtadvanceddocking_FOUND)

      if(qtadvanceddocking_FIND_REQUIRED)
        message(FATAL_ERROR "ADS library path ${qtadvanceddocking_DIR}/lib does not exist.")
      else(qtadvanceddocking_FIND_REQUIRED)
        message(WARNING "ADS library path ${qtadvanceddocking_DIR}/lib does not exist.")
      endif(qtadvanceddocking_FIND_REQUIRED)
    endif(EXISTS ${qtadvanceddocking_DIR}/lib)

  else(EXISTS ${qtadvanceddocking_DIR})
    unset(qtadvanceddocking_FOUND)

    if(qtadvanceddocking_FIND_REQUIRED)
      message(FATAL_ERROR "ADS library path ${qtadvanceddocking_DIR} does not exist.")
    else(qtadvanceddocking_FIND_REQUIRED)
      message(WARNING "ADS library path ${qtadvanceddocking_DIR} does not exist.")
    endif(qtadvanceddocking_FIND_REQUIRED)

  endif(EXISTS ${qtadvanceddocking_DIR})

else(DEFINED qtadvanceddocking_DIR)

  unset(qtadvanceddocking_FOUND)

  if(qtadvanceddocking_FIND_REQUIRED)
    message(FATAL_ERROR "qtadvanceddocking_DIR is not defined but the package is required.")
  endif(qtadvanceddocking_FIND_REQUIRED)

endif(DEFINED qtadvanceddocking_DIR)

if(qtadvanceddocking_FOUND)
  set(HAVE_ADS true CACHE BOOL "Have Qt ADS lib")
  message(STATUS "Found QtAdvancedDocking: TRUE")
else(qtadvanceddocking_FOUND)
  message(STATUS "Found QtAdvancedDocking: FALSE")
endif(qtadvanceddocking_FOUND)
