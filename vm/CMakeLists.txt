set(CrankVM_SOURCES
    arithmetic-primitives.c
    block-primitives.c
    context.c
    error.c
    external-primitives.c
    external-primitives.h
    heap.c
    heap.h
    image.c
    image.h
    interpreter.c
    numbered-primitives.h
    numbered-primitives.c
    object-model.c
    object-primitives.c
    object-primitives.h
    read-memory-stream.h
    special-objects.c
    system-primitives.c
    system-primitives.h

    internal-plugins/file-plugin.c
)

add_definitions(
    -DBUILD_LIB_CRANK_VM
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(LibCrankVM SHARED ${CrankVM_SOURCES})
target_link_libraries(LibCrankVM ${CrankVM_DEP_LIBS})
