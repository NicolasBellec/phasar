cmake_minimum_required(VERSION 3.10)

project(DFIPhasar)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build a small test tool to show how phasar may be used
add_executable(dfi_binding
  dfi_binding.cpp
)

find_package(Boost COMPONENTS log filesystem program_options graph ${BOOST_THREAD} REQUIRED)
target_link_libraries(dfi_binding
  LINK_PUBLIC
  phasar_config
  phasar_controller
  phasar_db
  phasar_experimental
  phasar_controlflow
  phasar_ifdside
  phasar_mono
  phasar_passes
  ${PHASAR_PLUGINS_LIB}
  phasar_pointer
  phasar_typehierarchy
  phasar_phasarllvm_utils
  phasar_utils
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
)

if(USE_LLVM_FAT_LIB)
  llvm_config(dfi_binding USE_SHARED ${LLVM_LINK_COMPONENTS})
else()
  llvm_config(dfi_binding ${LLVM_LINK_COMPONENTS})
endif()

set(LLVM_LINK_COMPONENTS
)

find_package(phasar COMPONENTS mono REQUIRED)
include_directories(${PHASAR_INCLUDE_DIR})
link_directories(${PHASAR_LIBRARY_DIR})


install(TARGETS dfi_binding
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
