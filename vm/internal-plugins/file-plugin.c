#include "crank-vm/interpreter.h"
#include <unistd.h>

static void
crankvm_FilePlugin_primitiveConnectToFile(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveConnectToFileDescriptor(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectoryCreate(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectoryDelete(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectoryDelimitor(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectoryEntry(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectoryGetMacTypeAndCreator(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectoryLookup(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDirectorySetMacTypeAndCreator(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveDisableFileAccess(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileAtEnd(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileClose(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileDelete(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileDescriptorType(crankvm_primitive_context_t *primitiveContext)
{
    // TODO: Implement this properly.
    return crankvm_primitive_returnInteger(primitiveContext, 3);
}

static void
crankvm_FilePlugin_primitiveFileFlush(crankvm_primitive_context_t *primitiveContext)
{
    int fd = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 0));
    if(crankvm_primitive_hasFailed(primitiveContext)) return;

    (void)fd;

    return crankvm_primitive_returnOop(primitiveContext, crankvm_primitive_getReceiver(primitiveContext));
}

static void
crankvm_FilePlugin_primitiveFileGetPosition(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileOpen(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileOpenNew(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileRead(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileRename(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileSetPosition(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileSize(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileStdioHandles(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_Array_t *array = crankvm_Array_create(primitiveContext->context, 3);
    array->slots[0] = crankvm_oop_encodeSmallInteger(STDIN_FILENO);
    array->slots[1] = crankvm_oop_encodeSmallInteger(STDOUT_FILENO);
    array->slots[2] = crankvm_oop_encodeSmallInteger(STDERR_FILENO);
    return crankvm_primitive_returnOop(primitiveContext, (crankvm_oop_t)array);
}

static void
crankvm_FilePlugin_primitiveFileSync(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileTruncate(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

static void
crankvm_FilePlugin_primitiveFileWrite(crankvm_primitive_context_t *primitiveContext)
{
    size_t count = crankvm_primitive_getSizeValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 0));
    size_t startIndex = crankvm_primitive_getSizeValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 1));
    uint8_t *bytes = crankvm_primitive_getBytesPointer(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 2));
    int fd = crankvm_primitive_getSmallIntegerValue(primitiveContext, crankvm_primitive_getStackAt(primitiveContext, 3));
    printf("fd: %d\n", fd);
    if(crankvm_primitive_hasFailed(primitiveContext)) return;

    printf("write bytes %zu into %d\n", count, fd);
    ssize_t written = write(fd, bytes + startIndex - 1, count);
    if(written < 0)
        return crankvm_primitive_fail(primitiveContext);

    return crankvm_primitive_returnInteger(primitiveContext, written);
}

static void
crankvm_FilePlugin_primitiveHasFileAccess(crankvm_primitive_context_t *primitiveContext)
{
    CRANK_UNIMPLEMENTED();
}

crankvm_plugin_t crankvm_FilePlugin = {
    .name = "FilePlugin",
    .primitives = {
        {.name = "primitiveConnectToFile", .function = crankvm_FilePlugin_primitiveConnectToFile},
        {.name = "primitiveConnectToFileDescriptor", .function = crankvm_FilePlugin_primitiveConnectToFileDescriptor},
        {.name = "primitiveDirectoryCreate", .function = crankvm_FilePlugin_primitiveDirectoryCreate},
        {.name = "primitiveDirectoryDelete", .function = crankvm_FilePlugin_primitiveDirectoryDelete},
        {.name = "primitiveDirectoryDelimitor", .function = crankvm_FilePlugin_primitiveDirectoryDelimitor},
        {.name = "primitiveDirectoryEntry", .function = crankvm_FilePlugin_primitiveDirectoryEntry},
        {.name = "primitiveDirectoryGetMacTypeAndCreator", .function = crankvm_FilePlugin_primitiveDirectoryGetMacTypeAndCreator},
        {.name = "primitiveDirectoryLookup", .function = crankvm_FilePlugin_primitiveDirectoryLookup},
        {.name = "primitiveDirectorySetMacTypeAndCreator", .function = crankvm_FilePlugin_primitiveDirectorySetMacTypeAndCreator},
        {.name = "primitiveDisableFileAccess", .function = crankvm_FilePlugin_primitiveDisableFileAccess},
        {.name = "primitiveFileAtEnd", .function = crankvm_FilePlugin_primitiveFileAtEnd},
        {.name = "primitiveFileClose", .function = crankvm_FilePlugin_primitiveFileClose},
        {.name = "primitiveFileDelete", .function = crankvm_FilePlugin_primitiveFileDelete},
        {.name = "primitiveFileDescriptorType", .function = crankvm_FilePlugin_primitiveFileDescriptorType},
        {.name = "primitiveFileFlush", .function = crankvm_FilePlugin_primitiveFileFlush},
        {.name = "primitiveFileGetPosition", .function = crankvm_FilePlugin_primitiveFileGetPosition},
        {.name = "primitiveFileOpen", .function = crankvm_FilePlugin_primitiveFileOpen},
        {.name = "primitiveFileOpenNew", .function = crankvm_FilePlugin_primitiveFileOpenNew},
        {.name = "primitiveFileRead", .function = crankvm_FilePlugin_primitiveFileRead},
        {.name = "primitiveFileRename", .function = crankvm_FilePlugin_primitiveFileRename},
        {.name = "primitiveFileSetPosition", .function = crankvm_FilePlugin_primitiveFileSetPosition},
        {.name = "primitiveFileSize", .function = crankvm_FilePlugin_primitiveFileSize},
        {.name = "primitiveFileStdioHandles", .function = crankvm_FilePlugin_primitiveFileStdioHandles},
        {.name = "primitiveFileSync", .function = crankvm_FilePlugin_primitiveFileSync},
        {.name = "primitiveFileTruncate", .function = crankvm_FilePlugin_primitiveFileTruncate},
        {.name = "primitiveFileWrite", .function = crankvm_FilePlugin_primitiveFileWrite},
        {.name = "primitiveHasFileAccess", .function = crankvm_FilePlugin_primitiveHasFileAccess},
        {NULL, NULL}
    }
};
