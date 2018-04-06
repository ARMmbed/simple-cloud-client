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

#ifndef RESOURCE_H
#define RESOURCE_H

#include "mbed.h"
#include "m2mresource.h"

/**
 * \brief Helper function for creating different kind of resources.
 *        The path of the resource will be "object_id/instance_id/resource_id"
 *        For example if object_id = 1, instance_id = 2, resource_id = 3
 *        the path would be 1/2/3
 *
 * \param list Pointer to the object list,
 *             contains objects to be registered to the server.
 * \param object_id Name of the object in integer format.
 * \param instance_id Name of the instance in integer format.
 * \param resource_id Name of the resource in integer format.
 * \param resource_type Resource type name.
 * \param data_type Data type of the resource value.
 * \param allowed Methods allowed for accessing this resource.
 * \param value Resource value as a null terminated string.
 *              May be set as NULL.
 * \param observable Resource set observable if true.
 * \param cb Function pointer to either:
 *           value_updated_callback2 if allowed & GET_PUT_ALLOWED
 *           OR
 *           execute_callback_2 in if allowed & POST_ALLOWED.
 *           In other cases this parameter is ignored.
 *
 *        NOTE: This function is not designed to support setting both
 *              GET_PUT_ALLOWED and POST_ALLOWED for parameter allowed
 *              at the same time.
 */
M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          const char *value,
                          bool observable);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          const char *value,
                          bool observable,
                          Callback<void(void*)> cb);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          const char *value,
                          bool observable,
                          Callback<void(const char*)> cb);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          int value,
                          bool observable);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          int value,
                          bool observable,
                          Callback<void(void*)> cb);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          int value,
                          bool observable,
                          Callback<void(const char*)> cb);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          float value,
                          bool observable);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          float value,
                          bool observable,
                          Callback<void(void*)> cb);

M2MResource* add_resource(M2MObjectList *list,
                          uint16_t object_id,
                          uint16_t instance_id,
                          uint16_t resource_id,
                          const char *resource_type,
                          M2MResourceInstance::ResourceType data_type,
                          M2MBase::Operation allowed,
                          float value,
                          bool observable,
                          Callback<void(const char*)> cb);



/**
 * Note that this function allocates memory on the heap! Don't forget to free it...
 *
 * c_str() on m2m::String can return the wrong pointer (which I've seen in the blink resource... Workaround)
 */
char* malloc_c_string_from_m2m_string(m2m::String str) {
    char *target = (char*)calloc(str.length() + 1, 1);
    for (size_t ix = 0; ix < str.length(); ix++) {
        target[ix] = str.at(ix);
    }
    return target;
}

#endif //RESOURCE_H
