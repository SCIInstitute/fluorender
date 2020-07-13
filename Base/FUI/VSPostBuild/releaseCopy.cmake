add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${GLEW_DLL_DIR}/glew32.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${Teem_DIR}/teem.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${TIFF_LIBRARY}/tiff.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${Zlib_ROOT_DIRECTORY}/Release/zlib.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${PNG_ROOT_DIR}/Release/libpng16.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${pole_LIBRARY_DIR}/Poledump.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
	)
