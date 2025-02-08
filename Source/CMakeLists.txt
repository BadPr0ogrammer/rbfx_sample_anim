set (SOURCE_FILES 
	Sample.cpp
        Sample.h
        SkeletalAnimation.cpp
        SkeletalAnimation.h
        SamplesManager.cpp
        SamplesManager.h
        Rotator.cpp
        Rotator.h
        Mover.cpp
        Mover.h
)

# Windows only: URHO3D_WIN32_CONSOLE controls whether to keep console or not
if (NOT URHO3D_WIN32_CONSOLE)
    set (PLATFORM WIN32)
endif ()

set (TARGET_NAME ${CMAKE_PROJECT_NAME})
add_executable(${TARGET_NAME} ${PLATFORM} ${SOURCE_FILES})

target_link_libraries (${TARGET_NAME} PRIVATE
    Urho3D
)

