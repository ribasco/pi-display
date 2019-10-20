if (CMAKE_CROSSCOMPILING)
    if (NOT EXISTS ${JAVA_HOME})
        message(FATAL_ERROR "Invalid JAVA_HOME = ${JAVA_HOME}")
    endif ()

    set(JNI_INCLUDE_DIRS "")
    set(JNI_LIBRARIES "")

    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^arm")
        if (TARGET_OS STREQUAL "linux")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include/linux")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/arm/libjawt.so")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/jre/lib/arm/server/libjvm.so")
        else ()
            message(FATAL_ERROR "[FIND-JNI] Target OS not supported for the '${CMAKE_SYSTEM_PROCESSOR}' architecture (TARGET OS = ${TARGET_OS}, PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR})")
        endif ()
    elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if (TARGET_OS STREQUAL "windows")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include/win32")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/jawt.lib")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/jvm.lib")
        elseif (TARGET_OS STREQUAL "linux")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include/win32")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/jawt.lib")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/jvm.lib")
        elseif (TARGET_OS STREQUAL "darwin")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include")
            list(APPEND JNI_INCLUDE_DIRS "${JAVA_HOME}/include/darwin")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/jawt.lib")
            #list(APPEND JNI_LIBRARIES "${JAVA_HOME}/lib/jvm.lib")
        else ()
            message(FATAL_ERROR "[FIND-JNI] Target OS not supported for the '${CMAKE_SYSTEM_PROCESSOR}' architecture (TARGET OS = ${TARGET_OS}, PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR})")
        endif ()
    else ()
        message(FATAL_ERROR "[FIND-JNI] Target system processor not supported by this build (CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR})")
    endif ()

    foreach (item ${JNI_INCLUDE_DIRS})
        if (NOT EXISTS ${item})
            message(FATAL_ERROR "Could not find entry ${item}")
        endif ()
    endforeach (item)

    set(JNI_FOUND true)
else ()
    # Use default find jni package if we are not cross-compiling
    # message(STATUS "[FIND-JNI] Using default FindJNI package")
    include(FindJNI)
endif ()

get_filename_component(JNI_LIBRARIES "" REALPATH BASE_DIR "${JNI_LIBRARIES}")
get_filename_component(JNI_INCLUDE_DIRS "" REALPATH BASE_DIR "${JNI_INCLUDE_DIRS}")

# message(STATUS "[FIND-JNI] JNI_LIBRARIES = ${JNI_LIBRARIES}")
# message(STATUS "[FIND-JNI] JNI_INCLUDE_DIRS = ${JNI_INCLUDE_DIRS}")