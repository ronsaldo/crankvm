#include <crank-vm/objectmodel.h>

LIB_CRANK_VM_EXPORT size_t
crankvm_object_header_getSmalltalkSize(crankvm_object_header_t *header)
{
    if(!header)
        return 0;

    size_t slotCount = crankvm_object_header_getSlotCount(header);
    crankvm_object_format_t format = crankvm_object_header_getObjectFormat(header);
    if(format < CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        return slotCount;
    }
    else if(format == CRANK_VM_OBJECT_FORMAT_INDEXABLE_64)
    {
        if(sizeof(uint64_t) > sizeof(crankvm_oop_t))
            return slotCount / 2;
        return slotCount;
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_32_1)
    {
        return slotCount * (sizeof(crankvm_oop_t) / 4) - (2 - (format - CRANK_VM_OBJECT_FORMAT_INDEXABLE_32));
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_16_3)
    {
        return slotCount * (sizeof(crankvm_oop_t) / 2) - (4 - (format - CRANK_VM_OBJECT_FORMAT_INDEXABLE_16));
    }
    else if(format <= CRANK_VM_OBJECT_FORMAT_INDEXABLE_8_7)
    {
        return slotCount * sizeof(crankvm_oop_t) - (8 - (format - CRANK_VM_OBJECT_FORMAT_INDEXABLE_8));
    }
    else //if(format <= CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD_7)
    {
        return slotCount * sizeof(crankvm_oop_t) - (8 - (format - CRANK_VM_OBJECT_FORMAT_COMPILED_METHOD));
    }
}
