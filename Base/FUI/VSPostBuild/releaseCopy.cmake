add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${GLEW_DLL_DIR}/glew32.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${Teem_DIR}/teem.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${TIFF_LIBRARY}/tiff.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${Zlib_ROOT_DIRECTORY}/Debug/zlib.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${PNG_ROOT_DIR}/Debug/libpng16.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)
