project(Formats)

include_directories(
  ${Teem_INCLUDE_DIRS}
  ${FLIVR_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${Qt5Xml_INCLUDE_DIRS}
  ${pugi_INCLUDE_DIRS}
  ${TIFF_INCLUDE_DIR}
)

                  #imageJ_reader.hpp
                  #JVMInitializer.hpp
set(HEADER_FILES  base_reader.hpp
                  base_writer.hpp
                  brkxml_reader.hpp
                  lbl_reader.hpp
                  lsm_reader.hpp
                  msk_reader.hpp
                  msk_writer.hpp
                  nrrd_reader.hpp
                  nrrd_writer.hpp
                  oib_reader.hpp
                  oif_reader.hpp
                  png_resource.hpp
                  pvxml_reader.hpp
                  tif_reader.hpp
                  tif_writer.hpp
                  tinyxml2.hpp
)

             #imageJ_reader.cpp
             #JVMInitializer.cpp
set(SOURCES  base_reader.cpp
             brkxml_reader.cpp
             lbl_reader.cpp
             lsm_reader.cpp
             msk_reader.cpp
             msk_writer.cpp
             nrrd_reader.cpp
             nrrd_writer.cpp
             oib_reader.cpp
             oif_reader.cpp
             pvxml_reader.cpp
             tif_reader.cpp
             tif_writer.cpp
             tinyxml2.cpp
)

set(Formats_INCLUDE_DIR ${HEADER_FILES} CACHE INTERNAL "")
set(Formats_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")


add_library(Formats STATIC ${HEADER_FILES} ${SOURCES})

target_link_libraries(Formats FLIVR ${teem_LIBRARIES} ${Boost_LIBRARIES} ${pugi_LIBRARIES} ${TIFF_LIBRARIES})

add_dependencies(Formats FLIVR Teem_external_download Boost_external_download pugi_external_download libtiff_external_download)
qt5_use_modules(Formats Xml)
