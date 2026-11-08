cmake_minimum_required(VERSION 3.14)
project(OwnGame C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

include_directories(include)

file(GLOB SOURCES "src/*.cpp" "src/*.c")

add_library(OwnGameCore STATIC ${SOURCES})
if(WIN32)
    target_link_libraries(OwnGameCore PUBLIC winmm shell32)
endif()

add_executable(OwnGame main.cpp)
target_link_libraries(OwnGame PRIVATE OwnGameCore)

add_custom_command(
    TARGET OwnGame PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_CURRENT_BINARY_DIR}/game_data.db"
    COMMENT "Cleaning up the old game_data.db database file before a new build..."
)

add_custom_command(
    TARGET OwnGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:OwnGame>/assets"
    COMMENT "Copying 'assets' next to the executable file..."
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(UnitTests tests/tests.cpp tests/gui_core_tests.cpp)
target_link_libraries(UnitTests PRIVATE OwnGameCore GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(UnitTests)
option(OWNGAME_BUILD_GUI "Build the Qt 6 retro desktop interface" OFF)
if(OWNGAME_BUILD_GUI)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Multimedia)
    set(CMAKE_AUTOMOC ON)
    qt_add_executable(OwnGameGui gui/main.cpp gui/GameController.cpp gui/GameController.h)
    target_link_libraries(OwnGameGui PRIVATE OwnGameCore Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia)
    qt_add_resources(OwnGameGui "gui_resources" PREFIX "/" FILES
        gui/shaders/curved.frag.qsb gui/qml/Main.qml gui/qml/RetroButton.qml gui/qml/RetroSlider.qml gui/qml/BroadcastNoise.qml gui/qml/IconArtwork.qml gui/qml/RetroCheckBox.qml gui/qml/Television.qml gui/qml/Cat.qml
        gui/assets/background.mp3 gui/assets/cat.wav gui/assets/fanfare.wav gui/assets/noanswer.wav gui/assets/roundEnd.wav gui/assets/meow.wav gui/assets/open.wav gui/assets/right.wav gui/assets/wrong.wav gui/assets/cat.png gui/assets/Oswald.ttf gui/assets/OFL-Oswald.txt)
    add_custom_command(TARGET OwnGameGui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:OwnGameGui>/assets")
endif()

if(OWNGAME_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(GuiTests tests/gui_integration_tests.cpp gui/GameController.cpp gui/GameController.h)
    target_include_directories(GuiTests PRIVATE gui)
    target_link_libraries(GuiTests PRIVATE OwnGameCore GTest::gtest Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia Qt6::Test)
    qt_add_resources(GuiTests "test_gui_resources" PREFIX "/" FILES
        gui/shaders/curved.frag.qsb gui/qml/Main.qml gui/qml/RetroButton.qml gui/qml/RetroSlider.qml gui/qml/BroadcastNoise.qml gui/qml/IconArtwork.qml gui/qml/RetroCheckBox.qml gui/qml/Television.qml gui/qml/Cat.qml
        gui/assets/background.mp3 gui/assets/cat.wav gui/assets/fanfare.wav gui/assets/noanswer.wav gui/assets/roundEnd.wav gui/assets/meow.wav gui/assets/open.wav gui/assets/right.wav gui/assets/wrong.wav gui/assets/cat.png gui/assets/Oswald.ttf)
    add_custom_command(TARGET GuiTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:GuiTests>/assets")
    add_test(NAME GuiIntegration COMMAND GuiTests)
    set_tests_properties(GuiIntegration PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
endif()

if(OWNGAME_BUILD_GUI AND WIN32)
    enable_language(RC)
    set_target_properties(OwnGameGui PROPERTIES WIN32_EXECUTABLE TRUE)
    target_sources(OwnGameGui PRIVATE gui/app.rc)
    target_include_directories(OwnGameGui PRIVATE gui)
endif()
