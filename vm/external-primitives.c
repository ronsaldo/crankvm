#include "external-primitives.h"

extern crankvm_plugin_t crankvm_FilePlugin;
const crankvm_plugin_t *crankvm_internalPlugins[] = {
    &crankvm_FilePlugin,
    NULL
};

CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER(crankvm_primitive_callExternalPrimitive, CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_EXTERNAL_PRIMITIVE_CALL)

const crankvm_plugin_t*
crankvm_plugin_findNamed(crankvm_context_t *context, const char *name)
{
    const crankvm_plugin_t **pos = crankvm_internalPlugins;
    while(*pos)
    {
        const crankvm_plugin_t *plugin = *pos;
        if(!strcmp(plugin->name, name))
            return plugin;
        ++pos;
    }
    return NULL;
}

const crankvm_plugin_primitive_t*
crankvm_plugin_findPrimitiveNamed(crankvm_context_t *context, const crankvm_plugin_t *plugin, const char *name)
{
    const crankvm_plugin_primitive_t *currentPrimitive = plugin->primitives;
    while(currentPrimitive->name)
    {
        if(!strcmp(currentPrimitive->name, name))
            return currentPrimitive;
        ++currentPrimitive;
    }
    return NULL;
}

void
crankvm_primitive_callExternalPrimitive(crankvm_primitive_context_t *primitiveContext)
{
    crankvm_context_t *context = primitiveContext->context;

    // Fetch the literal.
    crankvm_oop_t externalPrimitiveSpec = crankvm_primitive_getLiteral(primitiveContext, 0);
    if(crankvm_primitive_hasFailed(primitiveContext)) return;

    if(!crankvm_oop_isPointer(externalPrimitiveSpec) ||
        crankvm_object_header_getSlotCount((crankvm_object_header_t *)externalPrimitiveSpec) < 2)
        return crankvm_primitive_failWithCode(primitiveContext, CRANK_VM_PRIMITIVE_ERROR_BAD_ARGUMENT);

    crankvm_Array_t *primitiveSpec = (crankvm_Array_t*)externalPrimitiveSpec;
    crankvm_oop_t pluginNameOop = primitiveSpec->slots[0];
    crankvm_oop_t primitiveNameOop = primitiveSpec->slots[1];

    char *pluginName = crankvm_primitive_stringToCString(primitiveContext, pluginNameOop);
    char *primitiveName = crankvm_primitive_stringToCString(primitiveContext, primitiveNameOop);
    if(crankvm_primitive_hasFailed(primitiveContext))
    {
        crankvm_context_free(context, pluginName);
        crankvm_context_free(context, primitiveName);
        return;
    }

    // Find the plugin.
    const crankvm_plugin_t *foundPlugin = crankvm_plugin_findNamed(primitiveContext->context, pluginName);
    crankvm_context_free(context, pluginName);
    if(!foundPlugin)
    {
        crankvm_context_free(context, primitiveName);
        return crankvm_primitive_fail(primitiveContext);
    }

    // Find the primitive.
    const crankvm_plugin_primitive_t *foundPrimitive = crankvm_plugin_findPrimitiveNamed(primitiveContext->context, foundPlugin, primitiveName);
    crankvm_context_free(context, primitiveName);
    if(!foundPrimitive)
        return crankvm_primitive_fail(primitiveContext);

    // Invoke the primitive function.
    return foundPrimitive->function(primitiveContext);
}
