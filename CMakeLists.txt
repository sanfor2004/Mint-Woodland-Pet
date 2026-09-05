cmake_minimum_required(VERSION 3.20)
project(PikachuPet LANGUAGES CXX)
add_executable(PikachuPet WIN32 src/main.cpp)
target_compile_features(PikachuPet PRIVATE cxx_std_17)
target_compile_definitions(PikachuPet PRIVATE UNICODE _UNICODE NOMINMAX)
if(MSVC)
  target_compile_options(PikachuPet PRIVATE /EHsc /utf-8 /W4)
endif()
target_link_libraries(PikachuPet PRIVATE gdiplus sapi ole32 winmm)
add_custom_target(PikaRuntimeAssets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:PikachuPet>/data"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/pika/moods" "$<TARGET_FILE_DIR:PikachuPet>/assets/pika/moods"
  DEPENDS PikachuPet)
