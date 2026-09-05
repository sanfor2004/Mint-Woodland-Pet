cmake_minimum_required(VERSION 3.20)
project(MintWoodlandPet LANGUAGES CXX)
add_executable(MintWoodlandPet WIN32 src/main.cpp)
target_compile_features(MintWoodlandPet PRIVATE cxx_std_17)
target_compile_definitions(MintWoodlandPet PRIVATE UNICODE _UNICODE NOMINMAX)
if(MSVC)
  target_compile_options(MintWoodlandPet PRIVATE /EHsc /utf-8 /W4)
endif()
target_link_libraries(MintWoodlandPet PRIVATE gdiplus sapi ole32 winmm)
add_custom_target(MintWoodlandPetRuntimeAssets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:MintWoodlandPet>/data"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/mint-woodland-pet/moods" "$<TARGET_FILE_DIR:MintWoodlandPet>/assets/mint-woodland-pet/moods"
  DEPENDS MintWoodlandPet)
