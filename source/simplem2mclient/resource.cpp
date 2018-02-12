//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2017 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#include "mbed.h"
#include "mbed-cloud-client/MbedCloudClient.h"
#include "m2mresource.h"
#include "mbed-client/m2minterface.h"
#include <stdio.h>
#include <string.h>

M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, const char *value, bool observable, Callback<void(void*)> cb)
{
    M2MObject *object = NULL;
    M2MObjectInstance* object_instance = NULL;
    M2MResource* resource = NULL;
    char name[6];

    //check if object already exists.
    if (!list->empty()) {
        M2MObjectList::const_iterator it;
        it = list->begin();
        for ( ; it != list->end(); it++ ) {
            if ((*it)->name_id() == object_id) {
                object = (*it);
                break;
            }
        }
    }
    //Create new object if needed.
    if (!object) {
        snprintf(name, 6, "%d", object_id);
        object = M2MInterfaceFactory::create_object(name);
        list->push_back(object);
    } else {
        //check if instance already exists.
        object_instance = object->object_instance(instance_id);
    }
    //Create new instance if needed.
    if (!object_instance) {
        object_instance = object->create_object_instance(instance_id);
    }
    //create the recource.
    snprintf(name, 6, "%d", resource_id);
    resource = object_instance->create_dynamic_resource(name, resource_type, data_type, observable);
    //Set value if available.
    if (value) {
        resource->set_value((const unsigned char*)value, strlen(value));
    }
    //Set allowed operations for accessing the resource.
    resource->set_operation(allowed);
    //Set callback of PUT or POST operation is enabled.
    //NOTE: This function does not support setting them both.
    if ((allowed & M2MResourceInstance::POST_ALLOWED) && cb){
        // We need a copy of the callback. The original callback might go out of scope.
        // @todo, do we need to clear this? It's actually meant to live until the end of the program... But it's not nice to alloc and never free.
        Callback<void(void*)>* copy = new Callback<void(void*)>(cb);

        // Callback::call is const, which FP1 does not like. Cast it to non-const.
        FP1<void, void*> fp(copy, (void (Callback<void(void*)>::*)(void*))&Callback<void(void*)>::call);

        resource->set_execute_function(fp);
    }

    return resource;
}

M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, const char *value, bool observable, Callback<void(const char*)> cb)
{
    M2MObject *object = NULL;
    M2MObjectInstance* object_instance = NULL;
    M2MResource* resource = NULL;
    char name[6];

    //check if object already exists.
    if (!list->empty()) {
        M2MObjectList::const_iterator it;
        it = list->begin();
        for ( ; it != list->end(); it++ ) {
            if ((*it)->name_id() == object_id) {
                object = (*it);
                break;
            }
        }
    }
    //Create new object if needed.
    if (!object) {
        snprintf(name, 6, "%d", object_id);
        object = M2MInterfaceFactory::create_object(name);
        list->push_back(object);
    } else {
        //check if instance already exists.
        object_instance = object->object_instance(instance_id);
    }
    //Create new instance if needed.
    if (!object_instance) {
        object_instance = object->create_object_instance(instance_id);
    }
    //create the recource.
    snprintf(name, 6, "%d", resource_id);
    resource = object_instance->create_dynamic_resource(name, resource_type, data_type, observable);
    //Set value if available.
    if (value) {
        resource->set_value((const unsigned char*)value, strlen(value));
    }
    //Set allowed operations for accessing the resource.
    resource->set_operation(allowed);
    //Set callback of PUT or POST operation is enabled.
    //NOTE: This function does not support setting them both.
    if((allowed & M2MResourceInstance::PUT_ALLOWED) && cb) {
        // We need a copy of the callback. The original callback might go out of scope.
        // @todo, do we need to clear this? It's actually meant to live until the end of the program... But it's not nice to alloc and never free.
        Callback<void(const char*)>* copy = new Callback<void(const char*)>(cb);

        // Callback::call is const, which FP1 does not like. Cast it to non-const.
        FP1<void, const char*> fp(copy, (void (Callback<void(const char*)>::*)(const char*))&Callback<void(const char*)>::call);

        resource->set_value_updated_function(fp);
    }

    return resource;
}


M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, const char *value, bool observable)
{
    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, value, observable, (Callback<void(void*)>)NULL);
}

// Integer
M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, int value, bool observable, Callback<void(void*)> cb)
{
    char str[13];
    sprintf(str, "%d", value);

    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, str, observable, cb);
}

M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, int value, bool observable, Callback<void(const char*)> cb)
{
    char str[13];
    sprintf(str, "%d", value);

    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, str, observable, cb);
}

M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, int value, bool observable)
{
    char str[13];
    sprintf(str, "%d", value);

    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, str, observable);
}

// Float
M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, float value, bool observable, Callback<void(void*)> cb)
{
    char str[25];
    sprintf(str, "%g", value);

    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, str, observable, cb);
}

M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, float value, bool observable, Callback<void(const char*)> cb)
{
    char str[25];
    sprintf(str, "%g", value);

    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, str, observable, cb);
}

M2MResource* add_resource(M2MObjectList *list, uint16_t object_id, uint16_t instance_id,
                          uint16_t resource_id, const char *resource_type, M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed, float value, bool observable)
{
    char str[25];
    sprintf(str, "%g", value);

    return add_resource(list, object_id, instance_id, resource_id, resource_type, data_type, allowed, str, observable);
}


