include(ExternalProject)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install/libevent)
set(LIBNAME "event")
set(PROJ_PREFIX "libevent")

ExternalProject_Add(
        project_libevent
        GIT_REPOSITORY "https://github.com/libevent/libevent.git"
        GIT_TAG release-2.1.8-stable
        PREFIX ${PROJ_PREFIX}
        INSTALL_COMMAND $(MAKE) install
        BUILD_COMMAND $(MAKE)
        UPDATE_COMMAND ""
        # CONFIGURE_COMMAND ./autogen.sh COMMAND ./configure --host=${CMAKE_SYSTEM_PROCESSOR} --disable-openssl --prefix=<INSTALL_DIR> CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_SYSROOT=${CMAKE_SYSROOT} -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DEVENT__DISABLE_OPENSSL=ON -DEVENT__BUILD_SHARED_LIBRARIES=ON -DEVENT__DISABLE_THREAD_SUPPORT=OFF -DEVENT__DISABLE_BENCHMARK=ON -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_SAMPLES=ON -DEVENT__DISABLE_REGRESS=ON
)

ExternalProject_Get_Property(project_libevent SOURCE_DIR INSTALL_DIR)

file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
add_library(libevent SHARED IMPORTED)

set(LE_INCLUDE_DIR "${INSTALL_DIR}/include")
set(LE_LIB_DIR "${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}event${CMAKE_SHARED_LIBRARY_SUFFIX}")

set_target_properties(libevent PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_target_properties(libevent PROPERTIES IMPORTED_LOCATION ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

add_dependencies(libevent project_libevent)

message(STATUS "[LIBEVENT] INCLUDE = ${LE_INCLUDE_DIR}")
message(STATUS "[LIBEVENT] LIBRARIES = ${LE_LIB_DIR}")