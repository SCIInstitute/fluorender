## Internal. Section: /nuget/nuspec in .autopkg file (NUSPEC as section identifier CMake argument).
function(nuget_internal_autopkg_process_nuspec_args AUTOPKG_INDENT_SIZE AUTOPKG_CONTENT OUT_AUTOPKG_CONTENT OUT_PACKAGE_ID)
    # Inputs
    set(options REQUIRE_LICENSE_ACCEPTANCE)
    set(oneValueArgs PACKAGE VERSION TITLE DESCRIPTION LICENSE_URL PROJECT_URL ICON
        SUMMARY RELEASE_NOTES COPYRIGHT
    )
    set(multiValueArgs AUTHORS OWNERS TAGS)
    cmake_parse_arguments(NUARG
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
    )
    nuget_internal_helper_error_if_unparsed_args(
        "${NUARG_UNPARSED_ARGUMENTS}"
        "${NUARG_KEYWORDS_MISSING_VALUES}"
    )
    # See http://coapp.org/developers/autopackage.html and http://coapp.org/reference/autopackage-ref.html for below requirements
    nuget_internal_helper_error_if_empty("${NUARG_PACKAGE}" "PACKAGE must not be empty: it is a required element (/nuget/nuspec/id) of an .autopkg file.")
    nuget_internal_helper_error_if_empty("${NUARG_VERSION}" "VERSION must not be empty: it is a required element (/nuget/nuspec/version) of an .autopkg file.")
    nuget_internal_helper_error_if_empty("${NUARG_TITLE}" "TITLE must not be empty: it is a required element (/nuget/nuspec/title) of an .autopkg file.")
    nuget_internal_helper_error_if_empty("${NUARG_DESCRIPTION}" "DESCRIPTION must not be empty: it is a required element (/nuget/nuspec/description) of an .autopkg file.")
    nuget_internal_helper_error_if_empty("${NUARG_AUTHORS}" "AUTHORS must not be empty: it is a required element (/nuget/nuspec/authors) of an .autopkg file.")
    nuget_internal_helper_error_if_empty("${NUARG_OWNERS}" "OWNERS must not be empty: it is a required element (/nuget/nuspec/owners) of an .autopkg file.")
    # Actual functionality
    # Begin /nuget/nuspec
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_INDENT_SIZE}nuspec {")
    set(AUTOPKG_SUBELEMENT_INDENT_SIZE "${AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}")
    # Required nuspec subelements
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}id = ${NUARG_PACKAGE};")
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}version: ${NUARG_VERSION};")
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}title: ${NUARG_TITLE};")
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}description: \"${NUARG_DESCRIPTION}\";")
    string(REPLACE ";" "," AUTHORS "${NUARG_AUTHORS}")
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}authors: {${AUTHORS}};")
    string(REPLACE ";" "," OWNERS "${NUARG_OWNERS}")
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}owners: {${OWNERS}};")

    # Optional nuspec subelements
    if(NOT "${NUARG_LICENSE_URL}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}licenseUrl: \"${NUARG_LICENSE_URL}\";")
    endif()

    if(NOT "${NUARG_PROJECT_URL}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}projectUrl: \"${NUARG_PROJECT_URL}\";")
    endif()

    if(NOT "${NUARG_ICON}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}iconUrl: \"${NUARG_ICON}\";")
    endif()

    if(NUARG_REQUIRE_LICENSE_ACCEPTANCE)
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}requireLicenseAcceptance: true;")
    else()
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}requireLicenseAcceptance: false;")
    endif()

    if(NOT "${NUARG_SUMMARY}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}summary: \"${NUARG_SUMMARY}\";")
    endif()

    if(NOT "${NUARG_RELEASE_NOTES}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}releaseNotes: \"${NUARG_RELEASE_NOTES}\";")
    endif()

    if(NOT "${NUARG_COPYRIGHT}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}copyright: \"${NUARG_COPYRIGHT}\";")
    endif()

    if(NOT "${NUARG_TAGS}" STREQUAL "")
        string(REPLACE ";" "," TAGS "${NUARG_TAGS}")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}tags: {${TAGS}};")
    endif()

    # End /nuget/nuspec
    string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_INDENT_SIZE}};")
    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
    set(${OUT_PACKAGE_ID} "${NUARG_PACKAGE}" PARENT_SCOPE)
endfunction()

## Internal. Section: /nuget/dependencies in .autopkg file.
## Automatically generated based on previous nuget_add_dependencies() calls.
## Only dependencies marked as PUBLIC or INTERFACE are added (ie. PRIVATE dependencies are omitted).
function(nuget_internal_autopkg_add_dependencies AUTOPKG_INDENT_SIZE AUTOPKG_CONTENT OUT_AUTOPKG_CONTENT)
    # Begin /nuget/dependencies
    set(AUTOPKG_DEPENDENCIES_CONTENT_BEGIN "\n${AUTOPKG_INDENT_SIZE}dependencies {\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}packages: {")
    set(AUTOPKG_DEPENDENCIES_CONTENT_END "\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}};\n${AUTOPKG_INDENT_SIZE}};")
    set(AUTOPKG_DEPENDENCIES_CONTENT "${AUTOPKG_DEPENDENCIES_CONTENT_BEGIN}")
    # For each dependency that should be in /nuget/dependencies
    nuget_get_dependencies(DEPENDENCIES)
    set(AUTOPKG_SUBELEMENT_INDENT_SIZE "${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}")

    foreach(DEPENDENCY IN LISTS DEPENDENCIES)
        nuget_get_dependency_usage("${DEPENDENCY}" USAGE)

        if("${USAGE}" STREQUAL "PRIVATE")
            continue()
        endif()

        nuget_get_dependency_version("${DEPENDENCY}" VERSION)
        string(APPEND AUTOPKG_DEPENDENCIES_CONTENT "${DEPENDENCY_SEPARATOR}\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}${DEPENDENCY}/${VERSION}")
        set(DEPENDENCY_SEPARATOR ",")
    endforeach()

    # End /nuget/dependencies
    string(APPEND AUTOPKG_DEPENDENCIES_CONTENT "${AUTOPKG_DEPENDENCIES_CONTENT_END}")

    if(NOT "${AUTOPKG_DEPENDENCIES_CONTENT}" STREQUAL "${AUTOPKG_DEPENDENCIES_CONTENT_BEGIN}${AUTOPKG_DEPENDENCIES_CONTENT_END}")
        string(APPEND AUTOPKG_CONTENT "${AUTOPKG_DEPENDENCIES_CONTENT}")
    endif()

    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
endfunction()

## Internal. Section: /nuget/files in .autopkg file (FILES as section identifier CMake argument).
function(nuget_internal_autopkg_process_files_args
    AUTOPKG_INDENT_SIZE
    AUTOPKG_CONTENT
    CMAKE_ARCHITECTURE
    CMAKE_PLATFORMTOOLSET
    CMAKE_OUTPUT_DIR
    RELATIVE_OUTPUT_DIR
    OUT_AUTOPKG_CONTENT
)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs CMAKE_CONFIGURATIONS INCLUDES OUTPUTS)
    cmake_parse_arguments(NUARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    nuget_internal_helper_error_if_unparsed_args("${NUARG_UNPARSED_ARGUMENTS}" "${NUARG_KEYWORDS_MISSING_VALUES}")
    # Begin /nuget/files
    set(AUTOPKG_FILES_CONTENT_BEGIN "\n${AUTOPKG_INDENT_SIZE}files {")
    set(AUTOPKG_FILES_CONTENT_END "\n${AUTOPKG_INDENT_SIZE}};")
    set(AUTOPKG_FILES_CONTENT "${AUTOPKG_FILES_CONTENT_BEGIN}")

    if (NOT "${NUARG_INCLUDES}" STREQUAL "")
        nuget_internal_autopkg_process_files_includes_args(
            "${AUTOPKG_INDENT_SIZE}"
            "${AUTOPKG_FILES_CONTENT}"
            "${CMAKE_OUTPUT_DIR}"
            AUTOPKG_FILES_CONTENT
            ${NUARG_INCLUDES}
        )
    endif()

    if (NOT "${NUARG_OUTPUTS}" STREQUAL "")
        nuget_internal_autopkg_process_files_outputs_args(
            "${AUTOPKG_INDENT_SIZE}"
            "${AUTOPKG_FILES_CONTENT}"
            ${CMAKE_ARCHITECTURE}
            "${CMAKE_PLATFORMTOOLSET}"
            "${RELATIVE_OUTPUT_DIR}"
            AUTOPKG_FILES_CONTENT
            ${NUARG_OUTPUTS}
        )
    endif()

    # End /nuget/files
    string(APPEND AUTOPKG_FILES_CONTENT "${AUTOPKG_FILES_CONTENT_END}")

    if ("${AUTOPKG_FILES_CONTENT}" STREQUAL "${AUTOPKG_FILES_CONTENT_BEGIN}${AUTOPKG_FILES_CONTENT_END}")
        message(FATAL_ERROR "Assembled expression for generating /nuget/files node(s) for .autopkg file(s) is empty.")
    endif()

    string(APPEND AUTOPKG_CONTENT "${AUTOPKG_FILES_CONTENT}")
    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
endfunction()

## Internal.
function(nuget_internal_autopkg_process_files_includes_args
    AUTOPKG_INDENT_SIZE
    AUTOPKG_CONTENT
    CMAKE_OUTPUT_DIR
    OUT_AUTOPKG_CONTENT
)
    # Inputs
    # See http://coapp.org/developers/autopackage.html and http://coapp.org/reference/autopackage-ref.html
    set(options "")
    set(oneValueArgs FILE_INCLUDE_DEST)
    set(multiValueArgs FILE_INCLUDE_SRC)
    cmake_parse_arguments(NUARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    nuget_internal_helper_error_if_unparsed_args("${NUARG_UNPARSED_ARGUMENTS}" "${NUARG_KEYWORDS_MISSING_VALUES}")
    nuget_internal_helper_error_if_empty("${NUARG_FILE_INCLUDE_SRC}" "FILE_INCLUDE_SRC must not be empty: it is a required " +
        "attribute of an .autopkg file's /nuget/files/include element."
    )

    if (NOT "${NUARG_FILE_INCLUDE_DEST}" STREQUAL "")
        set(AUTOPKG_FILES_CONTENT_BEGIN "\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}nestedInclude: {")
    else()
        set(AUTOPKG_FILES_CONTENT_BEGIN "\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}include: {")
    endif()
    set(AUTOPKG_FILES_CONTENT_END "\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}};")
    set(AUTOPKG_FILES_CONTENT "${AUTOPKG_FILES_CONTENT_BEGIN}")
    set(AUTOPKG_SUBELEMENT_INDENT_SIZE "${AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}")
    cmake_path(RELATIVE_PATH CMAKE_SOURCE_DIR BASE_DIRECTORY ${CMAKE_OUTPUT_DIR} OUTPUT_VARIABLE RELATIVE_OUTPUT_DIR)

    if (NOT "${NUARG_FILE_INCLUDE_DEST}" STREQUAL "")
        string(APPEND AUTOPKG_FILES_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}#destination = ${NUARG_FILE_INCLUDE_DEST};")
    endif()

    foreach(FILE_INCLUDE_SRC ${NUARG_FILE_INCLUDE_SRC})
        string(APPEND AUTOPKG_FILES_CONTENT "${SEPARATOR}")
        string(APPEND AUTOPKG_FILES_CONTENT "\n${AUTOPKG_SUBELEMENT_INDENT_SIZE}${RELATIVE_OUTPUT_DIR}/${FILE_INCLUDE_SRC}")
        set(SEPARATOR ",")
    endforeach()

    string(APPEND AUTOPKG_FILES_CONTENT "${AUTOPKG_FILES_CONTENT_END}")
    string(APPEND AUTOPKG_CONTENT "${AUTOPKG_FILES_CONTENT}")
    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
endfunction()

## Internal.
function(nuget_internal_autopkg_process_files_outputs_args
    AUTOPKG_INDENT_SIZE
    AUTOPKG_CONTENT
    CMAKE_ARCHITECTURE
    CMAKE_PLATFORMTOOLSET
    RELATIVE_OUTPUT_DIR
    OUT_AUTOPKG_CONTENT
)
    # Begin /nuget/files
    set(AUTOPKG_FILES_CONTENT_BEGIN "\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}[$<LOWER_CASE:$<CONFIG>>")
    if (NOT CMAKE_ARCHITECTURE STREQUAL "")
        string(APPEND AUTOPKG_FILES_CONTENT_BEGIN ",${CMAKE_ARCHITECTURE}")
    endif()
    if (NOT CMAKE_PLATFORMTOOLSET STREQUAL "")
        string(APPEND AUTOPKG_FILES_CONTENT_BEGIN ",${CMAKE_PLATFORMTOOLSET}")
    endif()
    string(APPEND AUTOPKG_FILES_CONTENT_BEGIN "] {")
    set(AUTOPKG_FILES_CONTENT_END "\n${AUTOPKG_INDENT_SIZE}${AUTOPKG_INDENT_SIZE}}")
    set(AUTOPKG_FILES_CONTENT "${AUTOPKG_FILES_CONTENT_BEGIN}")
    set(ARGS_HEAD "")
    set(ARGS_TAIL ${ARGN})
    set(AUTOPKG_SUBELEMENT_INDENT_SIZE "${AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}")

    while(NOT "${ARGS_TAIL}" STREQUAL "")
        nuget_internal_helper_cut_arg_list(CMAKE_CONDITIONAL_SECTION "${ARGS_TAIL}" ARGS_HEAD ARGS_TAIL)
        list(LENGTH ARGS_HEAD ARGS_HEAD_LENGTH)

        if(ARGS_HEAD_LENGTH GREATER_EQUAL 2)
            list(GET ARGS_HEAD 0 MAYBE_CMAKE_INCLUDE_CONDITION_IDENTIFIER)

            if("${MAYBE_CMAKE_INCLUDE_CONDITION_IDENTIFIER}" STREQUAL "CMAKE_CONDITIONAL_SECTION")
                list(GET ARGS_HEAD 1 CMAKE_CONDITIONAL_SECTION)
                nuget_internal_helper_list_sublist("${ARGS_HEAD}" 2 -1 ARGS_HEAD)
            endif()
        endif()

        nuget_internal_autopkg_add_files_outputs_conditionally("${AUTOPKG_SUBELEMENT_INDENT_SIZE}" "${AUTOPKG_FILES_CONTENT}"
            "${RELATIVE_OUTPUT_DIR}" AUTOPKG_FILES_CONTENT "${CMAKE_CONDITIONAL_SECTION}" ${ARGS_HEAD}
        )
    endwhile()

    # End /nuget/files
    string(APPEND AUTOPKG_FILES_CONTENT "${AUTOPKG_FILES_CONTENT_END}")

    if("${AUTOPKG_FILES_CONTENT}" STREQUAL "${AUTOPKG_FILES_CONTENT_BEGIN}${AUTOPKG_FILES_CONTENT_END}")
        message(FATAL_ERROR "Assembled expression for generating /nuget/files node(s) for .autopkg file(s) is empty.")
    endif()

    string(APPEND AUTOPKG_CONTENT "${AUTOPKG_FILES_CONTENT}")
    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
endfunction()

## Internal.
function(nuget_internal_autopkg_add_files_outputs_conditionally AUTOPKG_INDENT_SIZE AUTOPKG_CONTENT RELATIVE_OUTPUT_DIR OUT_AUTOPKG_CONTENT CMAKE_CONDITIONAL_SECTION)
    # Input: check for a CMAKE_CONDITIONAL_SECTION parameter pack
    if(NOT "${CMAKE_CONDITIONAL_SECTION}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "$<${CMAKE_CONDITIONAL_SECTION}:")
    endif()

    nuget_internal_helper_error_if_empty("${ARGN}" "Input expression for generating elements under /nuget/files node(s) for .autopkg file(s) is empty: no FILE_SRC and FILE_TARGET arguments were provided.")
    # Loop over parameter pack
    set(ARGS_HEAD "")
    set(ARGS_TAIL ${ARGN})

    while(NOT "${ARGS_TAIL}" STREQUAL "")
        nuget_internal_helper_cut_arg_list(FILE_SRC "${ARGS_TAIL}" ARGS_HEAD ARGS_TAIL)
        nuget_internal_autopkg_add_output_file_conditionally("${AUTOPKG_INDENT_SIZE}" "${AUTOPKG_CONTENT}" "${RELATIVE_OUTPUT_DIR}"
            AUTOPKG_CONTENT ${ARGS_HEAD}
        )
    endwhile()

    # Close generator expression if this was a CMAKE_CONDITIONAL_SECTION parameter pack
    if(NOT "${CMAKE_CONDITIONAL_SECTION}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT ">")
    endif()

    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
endfunction()

## Internal.
function(nuget_internal_autopkg_add_output_file_conditionally AUTOPKG_INDENT_SIZE AUTOPKG_CONTENT RELATIVE_OUTPUT_DIR OUT_AUTOPKG_CONTENT)
    # Inputs
    # See http://coapp.org/developers/autopackage.html and http://coapp.org/reference/autopackage-ref.html
    set(options "")
    set(oneValueArgs FILE_BIN_SRC FILE_LIB_SRC FILE_SYMBOLS_SRC)
    set(multiValueArgs FILE_EXCLUDE)
    cmake_parse_arguments(NUARG
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
    )
    nuget_internal_helper_error_if_unparsed_args(
        "${NUARG_UNPARSED_ARGUMENTS}"
        "${NUARG_KEYWORDS_MISSING_VALUES}"
    )
    nuget_internal_helper_error_if_empty("${NUARG_FILE_BIN_SRC}${NUARG_FILE_LIB_SRC}${NUARG_FILE_SYMBOLS_SRC}"
        "FILE_BIN_SRC, FILE_LIB_SRC and FILE_SYMBOLS_SRC must not both be empty: one of them "
        "is a required attribute (bin or symbols) of an .autopkg file's /nuget/files element."
    )

    # Actual functionality
    if(NOT "${NUARG_FILE_BIN_SRC}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_INDENT_SIZE}bin: \"${RELATIVE_OUTPUT_DIR}/${NUARG_FILE_BIN_SRC}\";")
    endif()

    if(NOT "${NUARG_FILE_LIB_SRC}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_INDENT_SIZE}lib: \"${RELATIVE_OUTPUT_DIR}/${NUARG_FILE_LIB_SRC}\";")
    endif()

    if(NOT "${NUARG_FILE_SYMBOLS_SRC}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT "\n${AUTOPKG_INDENT_SIZE}symbols: \"${RELATIVE_OUTPUT_DIR}/${NUARG_FILE_SYMBOLS_SRC}\";")
    endif()

    if(NOT "${NUARG_FILE_EXCLUDE}" STREQUAL "")
        string(APPEND AUTOPKG_CONTENT " exclude=\"${NUARG_FILE_EXCLUDE}\"")
    endif()

    set(${OUT_AUTOPKG_CONTENT} "${AUTOPKG_CONTENT}" PARENT_SCOPE)
endfunction()

## Internal. Section: CMake-specific (without special section identifier CMake argument).
## Write output .autopkg file(s) conditionally for provided configurations in CMAKE_CONFIGURATIONS intersected with
## the available configurations in the current build system this function is actually called from. No error is raised if
## a given configuration is not available -- the output file is simply not generated for that in the current build system.
## Not raising an error if a given configuration is unavailable makes it possible to reuse the same nuget_generate_autopkg_files()
## calls across different build systems without adjustments or writing additional code for generating the values of the
## CMAKE_CONFIGURATIONS argument.
function(nuget_internal_autopkg_generate_output AUTOPKG_CONTENT PACKAGE_ID CMAKE_OUTPUT_DIR CMAKE_CONFIGURATIONS)
    # Inputs
    nuget_internal_helper_error_if_empty("${AUTOPKG_CONTENT}" "AUTOPKG_CONTENT to be written is empty: cannot generate .autopkg file's content.")
    nuget_internal_helper_error_if_empty("${PACKAGE_ID}" "PACKAGE_ID to be written is empty: cannot generate .autopkg filename.")

    if("${CMAKE_OUTPUT_DIR}" STREQUAL "")
        set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/CMakeNuGetTools/autopkg/${PACKAGE_ID}.$<CONFIG>.autopkg")
    else()
        set(OUTPUT_FILE "${CMAKE_OUTPUT_DIR}/${PACKAGE_ID}.$<CONFIG>.autopkg")
    endif()

    # Actual functionality
    if("${CMAKE_CONFIGURATIONS}" STREQUAL "")
        file(GENERATE OUTPUT "${OUTPUT_FILE}" CONTENT "${AUTOPKG_CONTENT}")
        message(STATUS "Written \"${OUTPUT_FILE}\" file(s).")
    else()
        set(CONDITIONS "$<OR:")

        foreach(CONFIGURATION IN LISTS CMAKE_CONFIGURATIONS)
            string(APPEND CONDITIONS "${CONDITIONS_SEPARATOR}$<CONFIG:${CONFIGURATION}>")
            set(CONDITIONS_SEPARATOR ",")
        endforeach()

        string(APPEND CONDITIONS ">")
        file(GENERATE OUTPUT "${OUTPUT_FILE}" CONTENT "${AUTOPKG_CONTENT}" CONDITION "${CONDITIONS}")
        message(STATUS "Written \"${OUTPUT_FILE}\" file(s) for \"${CMAKE_CONFIGURATIONS}\" configuration(s).")
    endif()
endfunction()

## Internal.
function(nuget_internal_merge_second_autopkg_file_into_first FILEPATH_ACC FILEPATH_IN)
    set(FILES_NODE_BEGIN_STR "files {")
    set(FILES_NODE_END_STR "}\\;")
    string(LENGTH "${FILES_NODE_BEGIN_STR}" FILES_NODE_BEGIN_LEN)
    string(LENGTH "${FILES_NODE_END_STR}" FILES_NODE_END_LEN)
    # Inputs: FILEPATH_IN
    file(STRINGS "${FILEPATH_IN}" LINES_IN NEWLINE_CONSUME ENCODING UTF-8)
    string(FIND "${LINES_IN}" "${FILES_NODE_BEGIN_STR}" LINES_IN_FILES_NODE_BEGIN_POS)

    if(${LINES_IN_FILES_NODE_BEGIN_POS} EQUAL -1)
        message(FATAL_ERROR "Cannot merge: did not find the \"${FILES_NODE_BEGIN_STR}\" part of the .autopkg files node in \"${FILEPATH_IN}\".")
    endif()

    string(FIND "${LINES_IN}" "${FILES_NODE_END_STR}" LINES_IN_FILES_NODE_END_POS REVERSE)

    if(${LINES_IN_FILES_NODE_END_POS} EQUAL -1)
        message(FATAL_ERROR "Cannot merge: did not find the \"${FILES_NODE_END_STR}\" part of the .autopkg files node in \"${FILEPATH_IN}\".")
    endif()

    # Inputs: FILEPATH_ACC
    file(STRINGS "${FILEPATH_ACC}" LINES_ACC NEWLINE_CONSUME ENCODING UTF-8)
    string(FIND "${LINES_ACC}" "${FILES_NODE_BEGIN_STR}" LINES_ACC_FILES_NODE_BEGIN_POS)

    if(${LINES_ACC_FILES_NODE_BEGIN_POS} EQUAL -1)
        message(FATAL_ERROR "Cannot merge: did not find the \"${FILES_NODE_BEGIN_STR}\" part of the .autopkg files node in \"${FILEPATH_ACC}\".")
    endif()

    string(FIND "${LINES_ACC}" "${FILES_NODE_END_STR}" LINES_ACC_FILES_NODE_END_POS REVERSE)

    if(${LINES_ACC_FILES_NODE_END_POS} EQUAL -1)
        message(FATAL_ERROR "Cannot merge: did not find the \"${FILES_NODE_END_STR}\" part of the .autopkg files node in \"${FILEPATH_ACC}\".")
    endif()

    # Check: FILEPATH_ACC and FILEPATH_IN contents outside the files node should not differ
    string(SUBSTRING "${LINES_IN}" 0 ${LINES_IN_FILES_NODE_BEGIN_POS} LINES_IN_UNTIL_FILES_NODE_BEGIN)
    string(SUBSTRING "${LINES_ACC}" 0 ${LINES_ACC_FILES_NODE_BEGIN_POS} LINES_ACC_UNTIL_FILES_NODE_BEGIN)

    if(NOT "${LINES_IN_UNTIL_FILES_NODE_BEGIN}" STREQUAL "${LINES_ACC_UNTIL_FILES_NODE_BEGIN}")
        message(FATAL_ERROR "Cannot merge: file content before the .autopkg files node of \"${FILEPATH_ACC}\" and \"${FILEPATH_IN}\" differs.")
    endif()

    string(SUBSTRING "${LINES_IN}" ${LINES_IN_FILES_NODE_END_POS} -1 LINES_IN_AFTER_FILES_NODE_END)
    string(SUBSTRING "${LINES_ACC}" ${LINES_ACC_FILES_NODE_END_POS} -1 LINES_ACC_AFTER_FILES_NODE_END)

    if(NOT "${LINES_IN_AFTER_FILES_NODE_END}" STREQUAL "${LINES_ACC_AFTER_FILES_NODE_END}")
        message(FATAL_ERROR "Cannot merge: file content after the .autopkg files node of \"${FILEPATH_ACC}\" and \"${FILEPATH_IN}\" differs.")
    endif()

    # Create merged content
    # NOTE: no need to check for duplicate <file> element entries when merging FILEPATH_ACC and FILEPATH_IN: nuget pack does not
    # seem to complain when file elements with the same src and target attributes are present in the files node of a .autopkg file.
    string(SUBSTRING "${LINES_ACC}" 0 ${LINES_ACC_FILES_NODE_END_POS} NEW_LINES_ACC)
    string(APPEND NEW_LINES_ACC "${NUGET_AUTOPKG_INDENT_SIZE}// Below merged from: \"${FILEPATH_IN}\"")
    math(EXPR LINES_IN_AFTER_FILES_NODE_BEGIN_POS "${LINES_IN_FILES_NODE_BEGIN_POS} + ${FILES_NODE_BEGIN_LEN}")
    string(SUBSTRING "${LINES_IN}" ${LINES_IN_AFTER_FILES_NODE_BEGIN_POS} -1 LINES_IN_AFTER_FILES_NODE_BEGIN)
    string(APPEND NEW_LINES_ACC "${LINES_IN_AFTER_FILES_NODE_BEGIN}")
    # Output: overwrite FILEPATH_ACC with merged content
    set(NEW_FILE_CONTENT_ACC "${NEW_LINES_ACC}")
    string(REPLACE "\\;" ";" NEW_FILE_CONTENT_ACC "${NEW_FILE_CONTENT_ACC}")
    file(WRITE "${FILEPATH_ACC}" "${NEW_FILE_CONTENT_ACC}")
endfunction()

## Internal.
function(nuget_internal_merge_n_autopkg_files FILEPATH_ACC)
    nuget_internal_helper_error_if_empty("${FILEPATH_ACC}" "No filepath to a .autopkg file provided as a basis for merge operation.")
    nuget_internal_helper_error_if_empty("${ARGN}" "No .autopkg filepaths provided for merge operation.")
    # Read first input file (FILEPATH_BASE)
    list(GET ARGN 0 FILEPATH_BASE)
    file(STRINGS "${FILEPATH_BASE}" LINES_BASE NEWLINE_CONSUME ENCODING UTF-8)
    # Inputs: FILEPATH_BASE
    set(FILES_NODE_BEGIN_STR "files {")
    string(LENGTH "${FILES_NODE_BEGIN_STR}" FILES_NODE_BEGIN_LEN)
    string(FIND "${LINES_BASE}" "${FILES_NODE_BEGIN_STR}" LINES_BASE_FILES_NODE_BEGIN_POS)

    if(${LINES_BASE_FILES_NODE_BEGIN_POS} EQUAL -1)
        message(FATAL_ERROR "Cannot merge: did not find the \"${FILES_NODE_BEGIN_STR}\" part of the .autopkg files node in \"${FILEPATH_BASE}\".")
    endif()
    
    # Initialize content of FILEPATH_ACC with FILEPATH_BASE adding comment referring to original source file
    math(EXPR LINES_BASE_AFTER_FILES_NODE_BEGIN_POS "${LINES_BASE_FILES_NODE_BEGIN_POS} + ${FILES_NODE_BEGIN_LEN}")
    string(SUBSTRING "${LINES_BASE}" 0 ${LINES_BASE_AFTER_FILES_NODE_BEGIN_POS} NEW_LINES_BASE)
    string(APPEND NEW_LINES_BASE "\n${NUGET_AUTOPKG_INDENT_SIZE}${NUGET_AUTOPKG_INDENT_SIZE}// Below loaded from: \"${FILEPATH_BASE}\"")
    string(SUBSTRING "${LINES_BASE}" ${LINES_BASE_AFTER_FILES_NODE_BEGIN_POS} -1 LINES_BASE_AFTER_FILES_NODE_BEGIN)
    string(APPEND NEW_LINES_BASE "${LINES_BASE_AFTER_FILES_NODE_BEGIN}")
    set(NEW_FILE_CONTENT_BASE "${NEW_LINES_BASE}")
    string(REPLACE "\\;" ";" NEW_FILE_CONTENT_BASE "${NEW_FILE_CONTENT_BASE}")
    file(WRITE "${FILEPATH_ACC}" "${NEW_FILE_CONTENT_BASE}")
    # Merge rest of the input files into FILEPATH_ACC
    nuget_internal_helper_list_sublist("${ARGN}" 1 -1 FILEPATHS_IN_TAIL)

    foreach(FILEPATH_IN IN LISTS FILEPATHS_IN_TAIL)
        nuget_internal_merge_second_autopkg_file_into_first("${FILEPATH_ACC}" "${FILEPATH_IN}")
    endforeach()
endfunction()
