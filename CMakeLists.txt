cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "Toolchain file")
  else()
    set(VITASDK "C:/msys64/usr/local/vitasdk")
    set(ENV{VITASDK} "C:/msys64/usr/local/vitasdk")
    set(CMAKE_TOOLCHAIN_FILE "${VITASDK}/share/vita.toolchain.cmake" CACHE PATH "Toolchain file")
  endif()
endif()

project(VitaDoctor C CXX)

if(EXISTS "$ENV{VITASDK}/share/vita.cmake")
  include("$ENV{VITASDK}/share/vita.cmake")
elseif(EXISTS "C:/msys64/usr/local/vitasdk/share/vita.cmake")
  include("C:/msys64/usr/local/vitasdk/share/vita.cmake")
endif()

set(VITA_APP_NAME "VitaDoctor")
set(VITA_TITLEID "VITADOC00")
set(VITA_VERSION "01.01")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-q -O2")

add_executable(VitaDoctor
  src/main.c
  src/gui.c
  src/config.c
  src/tab_analog.c
  src/tab_battery.c
  src/tab_display.c
  src/tab_motion.c
  src/tab_audio.c
  src/tab_camera.c
  src/tab_system.c
  src/tab_report.c
)

target_include_directories(VitaDoctor PRIVATE src)

target_link_libraries(VitaDoctor PRIVATE
  vita2d
  pib
  SceDisplay_stub
  SceGxm_stub
  SceCtrl_stub
  SceTouch_stub
  SceMotion_stub
  ScePower_stub
  SceAudio_stub
  SceAudioIn_stub
  SceCamera_stub
  SceNet_stub
  SceNetCtl_stub
  ScePgf_stub
  SceCommonDialog_stub
  SceSysmodule_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceVshBridge_stub
  m
)

vita_create_self(eboot.bin VitaDoctor UNSAFE)

vita_create_vpk(VitaDoctor.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}

    FILE sce_sys/icon0.png sce_sys/icon0.png
    FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml

    FILE audio/sound_left.wav audio/sound_left.wav
    FILE audio/sound_right.wav audio/sound_right.wav
)
