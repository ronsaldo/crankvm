set(CrankVM_SOURCES
    main.c
)

add_executable(crankvm ${CrankVM_SOURCES})
target_link_libraries(crankvm LibCrankVM)
