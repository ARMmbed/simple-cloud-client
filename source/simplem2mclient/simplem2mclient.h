//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifndef SIMPLEM2MCLIENT_H
#define SIMPLEM2MCLIENT_H
#include <stdio.h>

#include "factory_configurator_client.h"
#include "key_config_manager.h"
#include "m2mdevice.h"
#include "m2mresource.h"
#include "mbed-client/m2minterface.h"
#include "mbed-cloud-client/MbedCloudClient.h"
#include "memory_tests.h"
#include "pal.h"
#include "resource.h"
#include "update_client_hub.h"

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#include "update_ui_example.h"
#endif

#ifndef DEFAULT_FIRMWARE_PATH
#define DEFAULT_FIRMWARE_PATH       "/sd/firmware"
#endif

class SimpleM2MClient {

public:

    SimpleM2MClient(NetworkInterface *network) :
        _registered(false),
        _register_called(false),
        _network(network)
    {}

    int init() {
        extern const uint8_t arm_uc_vendor_id[];
        extern const uint16_t arm_uc_vendor_id_size;
        extern const uint8_t arm_uc_class_id[];
        extern const uint16_t arm_uc_class_id_size;

        ARM_UC_SetVendorId(arm_uc_vendor_id, arm_uc_vendor_id_size);
        ARM_UC_SetClassId(arm_uc_class_id, arm_uc_class_id_size);


        // Print some statistics of the object sizes and heap memory consumption
        // if the MBED_HEAP_STATS_ENABLED is defined.
        m2mobject_stats();
        heap_stats();

        fcc_status_e status = fcc_init();
        if (status != FCC_STATUS_SUCCESS)
        {
            printf("Factory client failed - %d", status);
            return 1;
        }

    // Resets storage to an empty state.
    // Use this function when you want to clear storage from all the factory-tool generated data and user data.
    // After this operation device must be injected again by using factory tool or developer certificate.
    #ifdef RESET_STORAGE
        printf("Resets storage to an empty state\n");
        fcc_status_e delete_status = fcc_storage_delete();
        if (delete_status != FCC_STATUS_SUCCESS)
        {
            printf("Failed to delete storage - %d\n", delete_status);
        }
    #endif

    // Deletes existing firmware images from storage.
    // This deletes any existing firmware images during application startup.
    // This compilation flag is currently implemented only for mbed OS.
    #ifdef RESET_FIRMWARE
        bool status_erase = rmFirmwareImages();
        if (status_erase == false)
        {
            return 1;
        }
    #endif

    #ifdef MBED_CONF_APP_DEVELOPER_MODE
        printf("Start developer flow\n");
        status = fcc_developer_flow();
        if (status == FCC_STATUS_KCM_FILE_EXIST_ERROR)
        {
            printf("Developer credentials already exists\n");
        }
        else if (status != FCC_STATUS_SUCCESS)
        {
            printf("Failed to load developer credentials - is the storage layer active and accessible? If you're using an SD card formatting should help - see also https://os.mbed.com/users/janjongboom/code/format-sd-card/\n");
            return 1;
        }
    #endif
        status = fcc_verify_device_configured_4mbed_cloud();
        if (status != FCC_STATUS_SUCCESS)
        {
            printf("Device not configured for mbed Cloud - is the storage layer active and accessible? If you're using an SD card formatting should help - see also https://os.mbed.com/users/janjongboom/code/format-sd-card/\n");
            return 1;
        }

        return 0;
    }

    bool call_register() {

        _cloud_client.on_registered(this, &SimpleM2MClient::client_registered);
        _cloud_client.on_unregistered(this, &SimpleM2MClient::client_unregistered);
        _cloud_client.on_error(this, &SimpleM2MClient::error);

        bool setup = _cloud_client.setup(_network);
        _register_called = true;
        if (!setup) {
            printf("Client setup failed\n");
            return false;
        }

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
        /* Set callback functions for authorizing updates and monitoring progress.
           Code is implemented in update_ui_example.cpp
           Both callbacks are completely optional. If no authorization callback
           is set, the update process will procede immediately in each step.
        */
        update_ui_set_cloud_client(&_cloud_client);
        _cloud_client.set_update_authorize_handler(update_authorize);
        _cloud_client.set_update_progress_handler(update_progress);
#endif
        return true;
    }

    void close() {
        _cloud_client.close();
    }

    void client_registered() {
        _registered = true;
        static const ConnectorClientEndpointInfo* endpoint = NULL;
        if (endpoint == NULL) {
            endpoint = _cloud_client.endpoint_info();
            if (endpoint) {
                _registered_cb(endpoint);
            }
        }
    }

    void client_unregistered() {
        _registered = false;
        _register_called = false;
        printf("Client unregistered\n");

    }

    void error(int error_code) {
        const char *error;
        switch(error_code) {
            case MbedCloudClient::ConnectErrorNone:
                error = "MbedCloudClient::ConnectErrorNone";
                break;
            case MbedCloudClient::ConnectAlreadyExists:
                error = "MbedCloudClient::ConnectAlreadyExists";
                break;
            case MbedCloudClient::ConnectBootstrapFailed:
                error = "MbedCloudClient::ConnectBootstrapFailed";
                break;
            case MbedCloudClient::ConnectInvalidParameters:
                error = "MbedCloudClient::ConnectInvalidParameters";
                break;
            case MbedCloudClient::ConnectNotRegistered:
                error = "MbedCloudClient::ConnectNotRegistered";
                break;
            case MbedCloudClient::ConnectTimeout:
                error = "MbedCloudClient::ConnectTimeout";
                break;
            case MbedCloudClient::ConnectNetworkError:
                error = "MbedCloudClient::ConnectNetworkError";
                break;
            case MbedCloudClient::ConnectResponseParseFailed:
                error = "MbedCloudClient::ConnectResponseParseFailed";
                break;
            case MbedCloudClient::ConnectUnknownError:
                error = "MbedCloudClient::ConnectUnknownError";
                break;
            case MbedCloudClient::ConnectMemoryConnectFail:
                error = "MbedCloudClient::ConnectMemoryConnectFail";
                break;
            case MbedCloudClient::ConnectNotAllowed:
                error = "MbedCloudClient::ConnectNotAllowed";
                break;
            case MbedCloudClient::ConnectSecureConnectionFailed:
                error = "MbedCloudClient::ConnectSecureConnectionFailed";
                break;
            case MbedCloudClient::ConnectDnsResolvingFailed:
                error = "MbedCloudClient::ConnectDnsResolvingFailed";
                break;
#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
            case MbedCloudClient::UpdateWarningCertificateNotFound:
                error = "MbedCloudClient::UpdateWarningCertificateNotFound";
                break;
            case MbedCloudClient::UpdateWarningIdentityNotFound:
                error = "MbedCloudClient::UpdateWarningIdentityNotFound";
                break;
            case MbedCloudClient::UpdateWarningCertificateInvalid:
                error = "MbedCloudClient::UpdateWarningCertificateInvalid";
                break;
            case MbedCloudClient::UpdateWarningSignatureInvalid:
                error = "MbedCloudClient::UpdateWarningSignatureInvalid";
                break;
            case MbedCloudClient::UpdateWarningVendorMismatch:
                error = "MbedCloudClient::UpdateWarningVendorMismatch";
                break;
            case MbedCloudClient::UpdateWarningClassMismatch:
                error = "MbedCloudClient::UpdateWarningClassMismatch";
                break;
            case MbedCloudClient::UpdateWarningDeviceMismatch:
                error = "MbedCloudClient::UpdateWarningDeviceMismatch";
                break;
            case MbedCloudClient::UpdateWarningURINotFound:
                error = "MbedCloudClient::UpdateWarningURINotFound";
                break;
            case MbedCloudClient::UpdateWarningRollbackProtection:
                error = "MbedCloudClient::UpdateWarningRollbackProtection";
                break;
            case MbedCloudClient::UpdateWarningUnknown:
                error = "MbedCloudClient::UpdateWarningUnknown";
                break;
            case MbedCloudClient::UpdateErrorWriteToStorage:
                error = "MbedCloudClient::UpdateErrorWriteToStorage";
                break;
#endif
            default:
                error = "UNKNOWN";
        }
        printf("\nError occurred : %s\r\n", error);
        printf("Error code : %d\r\n\n", error_code);
        printf("Error details : %s\r\n\n",_cloud_client.error_description());
    }

    bool is_client_registered() {
        return _registered;
    }

    bool is_register_called() {
        return _register_called;
    }

    void add_objects(M2MObjectList& object_list) {
        // Create resource for unregistering the device. Path of this resource will be: 5000/0/1.
        add_resource(&object_list, 5000, 0, 1, "unregister", M2MResourceInstance::STRING,
                    M2MBase::POST_ALLOWED, "", false, callback(this, &SimpleM2MClient::unregister));

        // Create resource for  running factory reset for the device. Path of this resource will be: 5000/0/2.
        add_resource(&object_list, 5000, 0, 2, "factory_reset", M2MResourceInstance::STRING,
                    M2MBase::POST_ALLOWED, "", false, callback(this, &SimpleM2MClient::factory_reset));

        _cloud_client.add_objects(object_list);
    }

    MbedCloudClient& get_cloud_client() {
        return _cloud_client;
    }

    void on_registered(Callback<void(const ConnectorClientEndpointInfo*)> cb) {
        _registered_cb = cb;
    }

    void factory_reset(void*) {
        printf("Factory reset resource executed\n");
        close();
        kcm_status_e kcm_status = kcm_factory_reset();
        if (kcm_status != KCM_STATUS_SUCCESS)
        {
            printf("Failed to do factory reset - %d\n", kcm_status);
        }
        else
        {
            printf("Factory reset completed. Restarting device...\n");
            NVIC_SystemReset();
        }
    }

    void unregister(void*) {
        printf("Unregister resource executed\n");
        close();
    }

private:
    bool rmFirmwareImages() {
        palStatus_t status = PAL_SUCCESS;
        status = pal_fsRmFiles(DEFAULT_FIRMWARE_PATH);
        if (status == PAL_SUCCESS)
        {
            printf("Firmware storage erased.\n");
        }
        else if (status == PAL_ERR_FS_NO_PATH)
        {
            printf("Firmware path not found/does not exist.\n");
        }
        else
        {
            printf("Firmware storage erasing failed with %ld", status);
            return false;
        }
        return true;
    }

    M2MObjectList       _obj_list;
    MbedCloudClient     _cloud_client;
    bool                _registered;
    bool                _register_called;
    NetworkInterface*   _network;

    Callback<void(const ConnectorClientEndpointInfo*)> _registered_cb;

};

#endif // SIMPLEM2MCLIENT_H
