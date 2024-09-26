####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
####

# Find Boost with the required components if any; since span is header only, no components are needed
find_package(Boost 1.78.0 REQUIRED)  # Ensure the version matches what's installed

# If your entire project can benefit from Boost, include the headers globally
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please ensure it is installed and the BOOST_ROOT is set.")
else()
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/instances.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/topology.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/FlightComputerTopologyDefs.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/FlightComputerTopology.cpp"
)

set(MOD_DEPS
  Fw/Logger
  Fw/Sm
  Svc/PosixTime
  # Communication Implementations
  Drv/Udp
  Drv/TcpClient
)

register_fprime_module()
