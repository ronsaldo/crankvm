set(CrankVM_SOURCES
    context.c
    error.c
    interpreter.c
)

add_definitions(
    -DBUILD_LIB_CRANK_VM
)

add_library(LibCrankVM SHARED ${CrankVM_SOURCES})
target_link_libraries(LibCrankVM ${CrankVM_DEP_LIBS})
