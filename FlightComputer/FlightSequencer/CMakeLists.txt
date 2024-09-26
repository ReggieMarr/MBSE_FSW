# Set the source files, including the generated file
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/FlightSequencer.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/FlightSequencer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/FlightSM.plantuml"
  "${CMAKE_CURRENT_LIST_DIR}/FlightSM.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sendEvent.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/SignalGen.cpp"
)

set(MOD_DEPS Fw/Sm)

# Register the F Prime module
register_fprime_module()
