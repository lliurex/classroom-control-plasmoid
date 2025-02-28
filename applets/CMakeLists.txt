if(NOT WIN32)
    # uses the command line tool 'classroom-control', does not make sense on Windows
    add_subdirectory(classroomcontrol)
endif(NOT WIN32)
