/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SIMPLE_CLOUD_CLIENT_H__
#define __SIMPLE_CLOUD_CLIENT_H__

#define smc_debug_msg(...) if (debug) printf(__VA_ARGS__)

#include <map>
#include <string>
#include <vector>
#include "mbed-cloud-client/MbedCloudClient.h"
#include "mbed-cloud-client/SimpleM2MResource.h"
#include "mbed-client/m2minterface.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed_cloud_client_user_config.h"
#include "factory_configurator_client.h"

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#include "update_ui_example.h"
#endif

#define MBED_CONF_SIMPLE_MBED_CLIENT_UPDATE_INTERVAL       25000

using namespace std;

struct MbedClientOptions {
    const char* Manufacturer;
    const char* Type;
    const char* ModelNumber;
    const char* SerialNumber;
};


static void mbed_cloud_error(int error_code) {
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
        case MbedCloudClient::UpdateWarningUnknown:
            error = "MbedCloudClient::UpdateWarningUnknown";
            break;
        case MbedCloudClient::UpdateErrorWriteToStorage:
            error = "MbedCloudClient::UpdateErrorWriteToStorage";
            break;
#endif
        default:
            error = "UNKNOWN";
            break;
    }

    // @todo, should I keep registry state somewhere?

    printf("\n[SMC] Error occured: %s\n", error);
    printf("[SMC] Error code: %d\n", error_code);
}

class SimpleResourceBase {
public:
    virtual void update(string v) {}
    virtual void clear_pending_value() {}
};

class SimpleMbedClientBase : public MbedCloudClientCallback {
public:

    SimpleMbedClientBase(bool aDebug = true)
        : evQueue(new EventQueue()), evThread(new Thread()), debug(aDebug)
    {
        evThread->start(callback(evQueue, &EventQueue::dispatch_forever));
    }

    SimpleMbedClientBase(EventQueue* aQueue, bool aDebug = true)
        : evQueue(aQueue), debug(aDebug)
    {

    }

    ~SimpleMbedClientBase() {
        if (evThread) {
            delete evQueue;
            delete evThread;
        }
        // if no evThread then evQueue is not owned by us
    }

    struct MbedClientOptions get_default_options() {
        struct MbedClientOptions options;
        options.Manufacturer = "Manufacturer_String";
        options.Type = "Type_String";
        options.ModelNumber = "ModelNumber_String";
        options.SerialNumber = "SerialNumber_String";

        return options;
    }

    bool init(NetworkInterface* iface, struct MbedClientOptions options) {
        smc_debug_msg("[SMC] Initializing...\n");

        fcc_status_e status = fcc_init();
        if(status != FCC_STATUS_SUCCESS) {
            smc_debug_msg("[SMC] fcc_init failed with status %d! - exit\n", status);
            return false;
        }

        // Resets storage to an empty state.
        // Use this function when you want to clear storage from all the factory-tool generated data and user data.
        // After this operation device must be injected again by using factory tool or developer certificate.
    #ifdef RESET_STORAGE
        smc_debug_msg("[SMC] Resets storage to an empty state\n");
        fcc_status_e delete_status = fcc_storage_delete();
        if (delete_status != FCC_STATUS_SUCCESS) {
            smc_debug_msg("[SMC] Failed to delete storage - %d\n", delete_status);
        }
    #endif

    #ifdef MBED_CONF_APP_DEVELOPER_MODE
        smc_debug_msg("[SMC] Start developer flow\n");
        status = fcc_developer_flow();
        if (status == FCC_STATUS_KCM_FILE_EXIST_ERROR) {
            smc_debug_msg("[SMC] Developer credentials already exists\n");
        } else if (status != FCC_STATUS_SUCCESS) {
            smc_debug_msg("[SMC] Failed to load developer credentials - exit\n");
            return false;
        }
    #endif
        status = fcc_verify_device_configured_4mbed_cloud();
        if (status != FCC_STATUS_SUCCESS) {
            smc_debug_msg("[SMC] Device not configured for mbed Cloud - exit\n");
            return false;
        }

        smc_debug_msg("[SMC] Registering...\n");

        // Create list of Objects to register
        M2MObjectList object_list;

#ifdef MBED_CLOUD_CLIENT_UPDATE_ID
        /* When this configuration flag is set, the manufacturer, model number
           and serial number is taken from update_default_resources.c
        */
#else
        M2MDevice*   device_object   = M2MInterfaceFactory::create_device();
        device_object->create_resource(M2MDevice::Manufacturer, options.Manufacturer);
        device_object->create_resource(M2MDevice::DeviceType, options.Type);
        device_object->create_resource(M2MDevice::ModelNumber, options.ModelNumber);
        device_object->create_resource(M2MDevice::SerialNumber, options.SerialNumber);
        object_list.push_back(device_object);
#endif

        map<string, M2MObject*>::iterator it;
        for (it = objects.begin(); it != objects.end(); it++)
        {
            object_list.push_back(it->second);
        }

        // Set endpoint registration object
        client->add_objects(object_list);

        // @todo make member function
        client->on_error(&mbed_cloud_error);

        client->set_update_callback(this);

        // @todo: shouldn't alloc this... But I need a copy...
        Callback<void()>* internal_register_cb = new Callback<void()>(this, &SimpleMbedClientBase::client_registered);
        client->on_registered(internal_register_cb, (void (Callback<void()>::*)(void))&Callback<void()>::call);

        Callback<void()>* internal_unregister_cb = new Callback<void()>(this, &SimpleMbedClientBase::client_unregistered);
        client->on_unregistered(internal_unregister_cb, (void (Callback<void()>::*)(void))&Callback<void()>::call);

        // Keep alive ticker (every 25 seconds)
        evQueue->call_every(MBED_CONF_SIMPLE_MBED_CLIENT_UPDATE_INTERVAL,
            callback(this, &SimpleMbedClientBase::keep_alive));

        // Connect to mbed Cloud
        bool cloud_setup = client->setup(iface);
        if (!cloud_setup) {
            smc_debug_msg("[SMC] mbed Cloud Client setup failed\n");
            return false;
        }

        // So this apparently only works if you set it after setup() was called...
#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
        /* Set callback functions for authorizing updates and monitoring progress.
        Code is implemented in update_ui_example.cpp

        Both callbacks are completely optional. If no authorization callback
        is set, the update process will procede immediately in each step.
        */
#ifdef ARM_UPDATE_CLIENT_VERSION_VALUE
#if ARM_UPDATE_CLIENT_VERSION_VALUE > 101000
        client->set_update_authorize_handler(&update_authorize);
#endif
#endif
        client->set_update_progress_handler(&update_progress);
#endif

        return true;
    }

    bool setup(NetworkInterface* iface) {
        if (client) {
            smc_debug_msg("[SMC] [ERROR] mbed_client_setup called, but mbed_client is already instantiated\n");
            return false;
        }

        client = new MbedCloudClient();

        return init(iface, get_default_options());
    }

    bool setup(MbedClientOptions options, NetworkInterface* iface) {
        if (client) {
            smc_debug_msg("[SMC] [ERROR] mbed_client_setup called, but mbed_client is already instantiated\n");
            return false;
        }

        client = new MbedCloudClient();

        return init(iface, options);
    }

    void on_registered(Callback<void()> fp) {
        registeredCallback = fp;
    }

    void on_unregistered(Callback<void()> fp) {
        unregisteredCallback = fp;
    }

    bool define_function(const char* route, Callback<void(void*)> ev) {
        if (!define_resource_internal(route, string(), M2MBase::POST_ALLOWED, false)) {
            return false;
        }

        string route_str(route);
        if (!resources.count(route_str)) {
            smc_debug_msg("[SMC] [ERROR] Should be created, but no such route (%s)\n", route);
            return false;
        }

        // We need a copy of the callback. The original callback might go out of scope.
        // @todo, do we need to clear this? It's actually meant to live until the end of the program... But it's not nice to alloc and never free.
        Callback<void(void*)>* copy = new Callback<void(void*)>(ev);

        // Callback::call is const, which FP1 does not like. Cast it to non-const.
        FP1<void, void*> fp(copy, (void (Callback<void(void*)>::*)(void*))&Callback<void(void*)>::call);

        resources[route_str]->set_execute_function(fp);
        return true;
    }

    bool define_function(const char* route, void(*fn)(void*)) {
        if (!define_resource_internal(route, string(), M2MBase::POST_ALLOWED, false)) {
            return false;
        }

        string route_str(route);
        if (!resources.count(route_str)) {
            smc_debug_msg("[SMC] [ERROR] Should be created, but no such route (%s)\n", route);
            return false;
        }

        resources[route_str]->set_execute_function(execute_callback_2(fn));
        return true;
    }

    bool define_function(const char* route, execute_callback fn) {
        if (!define_resource_internal(route, string(), M2MBase::POST_ALLOWED, false)) {
            return false;
        }

        string route_str(route);
        if (!resources.count(route_str)) {
            smc_debug_msg("[SMC] [ERROR] Should be created, but no such route (%s)\n", route);
            return false;
        }
        // No clue why this is not working?! It works with class member, but not with static function...
        resources[route_str]->set_execute_function(fn);
        return true;
    }

    string get(string route_str) {
        if (!resources.count(route_str)) {
            smc_debug_msg("[SMC] [ERROR] No such route (%s)\n", route_str.c_str());
            return string();
        }

        // otherwise ask mbed Client...
        uint8_t* buffIn = NULL;
        uint32_t sizeIn;
        resources[route_str]->get_value(buffIn, sizeIn);

        string s((char*)buffIn, sizeIn);
        return s;
    }

    // Note: these `set` calls are async.
    // SimpleResource* buffers the value, so when using it through operators you'll get the right value back.
    void set(string route_str, string v) {
        evQueue->call(callback(this, &SimpleMbedClientBase::internal_set_str), route_str, v);
    }

    void set(string route_str, const int& v) {
        evQueue->call(callback(this, &SimpleMbedClientBase::internal_set_int), route_str, v);
    }

    void set(string route_str, const float& v) {
        evQueue->call(callback(this, &SimpleMbedClientBase::internal_set_float), route_str, v);
    }

    bool define_resource_internal(const char* route, string v, M2MBase::Operation opr, bool observable) {
        if (client) {
            smc_debug_msg("[SMC] [ERROR] mbed_client_define_resource, Can only define resources before mbed_client_setup is called!\n");
            return false;
        }

        vector<string> segments = parse_route(route);
        if (segments.size() != 3) {
            smc_debug_msg("[SMC] [ERROR] mbed_client_define_resource, Route needs to have three segments, split by '/' (%s)\n", route);
            return false;
        }

        // segments[1] should be one digit and numeric
        char n = segments.at(1).c_str()[0];
        if (n < '0' || n > '9') {
            smc_debug_msg("[SMC] [ERROR] mbed_client_define_resource, second route segment should be numeric, but was not (%s)\n", route);
            return false;
        }

        int inst_id = atoi(segments.at(1).c_str());

        M2MObjectInstance* inst;
        if (objectInstances.count(segments.at(0))) {
            inst = objectInstances[segments.at(0)];
        }
        else {
            M2MObject* obj = M2MInterfaceFactory::create_object(segments.at(0).c_str());
            inst = obj->create_object_instance(inst_id);
            objects.insert(std::pair<string, M2MObject*>(segments.at(0), obj));
            objectInstances.insert(std::pair<string, M2MObjectInstance*>(segments.at(0), inst));
        }

        // @todo check if the resource exists yet
        M2MResource* res = inst->create_dynamic_resource(segments.at(2).c_str(), "",
            M2MResourceInstance::STRING, observable);
        res->set_operation(opr);
        res->set_value((uint8_t*)v.c_str(), v.length());

        string route_str(route);
        resources.insert(pair<string, M2MResource*>(route_str, res));

        return true;
    }

    void register_update_callback(string route, SimpleResourceBase* simpleResource) {
        updateValues[route] = simpleResource;
    }

    M2MResource* get_resource(string route) {
        if (!resources.count(route)) {
            smc_debug_msg("[SMC] [ERROR] No such route (%s)\n", route.c_str());
            return NULL;
        }

        return resources[route];
    }

    EventQueue* eventQueue() const {
        return evQueue;
    }

private:
    vector<string> parse_route(const char* route) {
        const string s(route);
        vector<string> v;

        split(s, '/', v);

        return v;
    }

    void split(const string& s, char delim, vector<string>& v) {
        size_t i = 0;
        size_t pos = s.find(delim);
        while (pos != string::npos) {
            v.push_back(s.substr(i, pos - i));
            i = ++pos;
            pos = s.find(delim, pos);

            if (pos == string::npos) {
                v.push_back(s.substr(i, s.length()));
            }
        }
    }

    virtual void value_updated(M2MBase *base, M2MBase::BaseType type) {
        if (strcmp(base->uri_path(), "") != 0) {
            resource_updated(string(base->uri_path()));
        }
    }

    void resource_updated(string uri) {
        if (updateValues.count(uri) == 0) {
            smc_debug_msg("[SMC] PUT came in for %s\n", uri.c_str());
            return;
        }

        string v = get(uri);

        // mbed Cloud 1.2 portal *always* puts "" around new values from a PUT... strip them off
        // @todo: remove this check when portal works properly
        if (v.size() > 1 && v.substr(0, 1) == "\"" && v.substr(v.size() - 1, 1) == "\"") {
            v = v.substr(1, v.size() - 2);
        }

        smc_debug_msg("[SMC] PUT came in for %s, new value is %s\n", uri.c_str(), v.c_str());

        if (v.empty()) return;

        // Schedule this on the other thread, to avoid blocking this thread
        evQueue->call(callback(updateValues[uri], &SimpleResourceBase::update), v);
    }

    // These operations have side effects, they should not be called immediately,
    // but always through the eventqueue
    void internal_set_str(string route_str, string v) {
        if (!resources.count(route_str)) {
            smc_debug_msg("[SMC] [ERROR] No such route (%s)\n", route_str.c_str());
            return;
        }

        if (v.length() == 0) {
            resources[route_str]->clear_value();
        }
        else {
            resources[route_str]->set_value((uint8_t*)v.c_str(), v.length());
        }

        updateValues[route_str]->clear_pending_value();
    }

    void internal_set_int(string route, const int& v) {
        char str[13];
        sprintf(str, "%d", v);

        internal_set_str(route, string(str));
    }

    void internal_set_float(string route, const float& v) {
        char str[25];
        sprintf(str, "%g", v);

        internal_set_str(route, string(str));
    }

    void keep_alive() {
// In case you are using UDP mode, to overcome NAT firewall issue , this example
// application sends keep alive pings every 25 seconds so that the application doesnt
// lose network connection over UDP. In case of TCP, this is not required as long as
// TCP keepalive is properly configured to a reasonable value, default for mbed Cloud
// Client is 300 seconds
#if defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) || \
    defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE)
        client->keep_alive();
#endif
    }

    void client_registered() {
        static const ConnectorClientEndpointInfo* endpoint = NULL;
        if (endpoint == NULL) {
            endpoint = client->endpoint_info();
        }

        smc_debug_msg("[SMC] Registered:\n");
        if (endpoint) {
#ifdef MBED_CONF_APP_DEVELOPER_MODE
            smc_debug_msg("\tEndpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
#else
            smc_debug_msg("\tEndpoint Name: %s\n", endpoint->endpoint_name.c_str());
#endif
            smc_debug_msg("\tDevice Id: %s\n", endpoint->internal_endpoint_name.c_str());
        }
        else {
            smc_debug_msg("\tCould not load endpoint info");
        }

        if (registeredCallback) {
            registeredCallback();
        }
    }

    void client_unregistered() {
        smc_debug_msg("[SMC] Unregistered\n");

        if (unregisteredCallback) {
            unregisteredCallback();
        }
    }

    MbedCloudClient* client;
    map<string, M2MObject*> objects;
    map<string, M2MObjectInstance*> objectInstances;
    map<string, M2MResource*> resources;

    EventQueue*     evQueue;
    Thread*         evThread;

    bool debug;

    map<string, SimpleResourceBase*> updateValues;

    Callback<void()> registeredCallback;
    Callback<void()> unregisteredCallback;
};

class SimpleResourceString : public SimpleResourceBase {
public:
    SimpleResourceString(SimpleMbedClientBase* aSimpleClient, string aRoute, Callback<void(string)> aOnUpdate) :
        simpleClient(aSimpleClient), route(aRoute), onUpdate(aOnUpdate), hasPendingValue(false) {}

    string operator=(const string& newValue) {
        pendingValue = newValue;
        hasPendingValue = true;

        simpleClient->set(route, newValue);
        return newValue;
    };

    operator string() const {
        if (hasPendingValue) {
            return pendingValue;
        }

        return simpleClient->get(route);
    };

    virtual void update(string v) {
        if (onUpdate) onUpdate(v);
    }

    M2MResource* get_resource() {
        return simpleClient->get_resource(route);
    }

    virtual void clear_pending_value() {
        hasPendingValue = false;
    }

private:
    SimpleMbedClientBase* simpleClient;
    string route;
    Callback<void(string)> onUpdate;

    // set() is async (because on the event queue, so store the pending value here...)
    bool hasPendingValue;
    string pendingValue;
};

class SimpleResourceInt : public SimpleResourceBase {
public:
    SimpleResourceInt(SimpleMbedClientBase* aSimpleClient, string aRoute, Callback<void(int)> aOnUpdate) :
        simpleClient(aSimpleClient), route(aRoute), onUpdate(aOnUpdate), hasPendingValue(false), pendingValue(0) {}

    int operator=(int newValue) {
        pendingValue = newValue;
        hasPendingValue = true;

        simpleClient->set(route, newValue);
        return newValue;
    };
    operator int() const {
        if (hasPendingValue) {
            return pendingValue;
        }

        string v = simpleClient->get(route);
        if (v.empty()) return 0;

        return atoi((const char*)v.c_str());
    };

    virtual void update(string v) {
        if (!onUpdate) return;

        onUpdate(atoi((const char*)v.c_str()));
    }

    M2MResource* get_resource() {
        return simpleClient->get_resource(route);
    }

    virtual void clear_pending_value() {
        hasPendingValue = false;
    }

private:
    SimpleMbedClientBase* simpleClient;
    string route;
    Callback<void(int)> onUpdate;

    // set() is async (because on the event queue, so store the pending value here...)
    bool hasPendingValue;
    int pendingValue;
};

class SimpleResourceFloat : public SimpleResourceBase {
public:
    SimpleResourceFloat(SimpleMbedClientBase* aSimpleClient, string aRoute, Callback<void(float)> aOnUpdate) :
        simpleClient(aSimpleClient), route(aRoute), onUpdate(aOnUpdate), hasPendingValue(false), pendingValue(0) {}

    float operator=(float newValue) {
        pendingValue = newValue;
        hasPendingValue = true;

        simpleClient->set(route, newValue);
        return newValue;
    };
    operator float() const {
        if (hasPendingValue) {
            return pendingValue;
        }

        string v = simpleClient->get(route);
        if (v.empty()) return 0;

        return atof((const char*)v.c_str());
    };

    virtual void update(string v) {
        if (!onUpdate) return;

        onUpdate(atof((const char*)v.c_str()));
    }

    M2MResource* get_resource() {
        return simpleClient->get_resource(route);
    }

    virtual void clear_pending_value() {
        hasPendingValue = false;
    }

private:
    SimpleMbedClientBase* simpleClient;
    string route;
    Callback<void(float)> onUpdate;

    // set() is async (because on the event queue, so store the pending value here...)
    bool hasPendingValue;
    float pendingValue;
};

class SimpleMbedClient : public SimpleMbedClientBase {
public:

    SimpleMbedClient(bool aDebug = true)
        : SimpleMbedClientBase(aDebug)
    {

    }

    SimpleMbedClient(EventQueue* aQueue, bool aDebug = true)
        : SimpleMbedClientBase(aQueue, aDebug)
    {

    }

    // @todo: macro this up

    // String
    SimpleResourceString define_resource(
        const char* route,
        string v,
        M2MBase::Operation opr = M2MBase::GET_PUT_ALLOWED,
        bool observable = true,
        Callback<void(string)> onUpdate = NULL)
    {
        SimpleResourceString* simpleResource = new SimpleResourceString(this, route, onUpdate);
        bool res = define_resource_internal(route, v, opr, observable);
        if (!res) {
            printf("Error while creating %s\n", route);
        }
        else {
            register_update_callback(route, simpleResource);
        }
        return *simpleResource;
    }

    SimpleResourceString define_resource(
        const char* route,
        string v,
        M2MBase::Operation opr,
        bool observable,
        void(*onUpdate)(string))
    {
        Callback<void(string)> fp = onUpdate;
        return define_resource(route, v, opr, observable, fp);
    }

    SimpleResourceString define_resource(
        const char* route,
        string v,
        Callback<void(string)> onUpdate)
    {
        return define_resource(route, v, M2MBase::GET_PUT_ALLOWED, true, onUpdate);
    }

    SimpleResourceString define_resource(
        const char* route,
        string v,
        void(*onUpdate)(string))
    {
        Callback<void(string)> fp = onUpdate;
        return define_resource(route, v, M2MBase::GET_PUT_ALLOWED, true, fp);
    }

    // Int
    SimpleResourceInt define_resource(
        const char* route,
        int v,
        M2MBase::Operation opr = M2MBase::GET_PUT_ALLOWED,
        bool observable = true,
        Callback<void(int)> onUpdate = NULL)
    {
        SimpleResourceInt* simpleResource = new SimpleResourceInt(this, route, onUpdate);

        char str[13];
        sprintf(str, "%d", v);

        bool res = define_resource_internal(route, string(str), opr, observable);
        if (!res) {
            printf("Error while creating %s\n", route);
        }
        else {
            register_update_callback(route, simpleResource);
        }
        return *simpleResource;
    }

    SimpleResourceInt define_resource(
        const char* route,
        int v,
        M2MBase::Operation opr,
        bool observable,
        void(*onUpdate)(int))
    {
        Callback<void(int)> fp = onUpdate;
        return define_resource(route, v, opr, observable, fp);
    }

    SimpleResourceInt define_resource(
        const char* route,
        int v,
        Callback<void(int)> onUpdate)
    {
        return define_resource(route, v, M2MBase::GET_PUT_ALLOWED, true, onUpdate);
    }

    SimpleResourceInt define_resource(
        const char* route,
        int v,
        void(*onUpdate)(int))
    {
        Callback<void(int)> fp = onUpdate;
        return define_resource(route, v, M2MBase::GET_PUT_ALLOWED, true, fp);
    }

    // Float
    SimpleResourceFloat define_resource(
        const char* route,
        float v,
        M2MBase::Operation opr = M2MBase::GET_PUT_ALLOWED,
        bool observable = true,
        Callback<void(float)> onUpdate = NULL)
    {
        SimpleResourceFloat* simpleResource = new SimpleResourceFloat(this, route, onUpdate);

        char str[25];
        sprintf(str, "%g", v);

        bool res = define_resource_internal(route, string(str), opr, observable);
        if (!res) {
            printf("Error while creating %s\n", route);
        }
        else {
            register_update_callback(route, simpleResource);
        }
        return *simpleResource;
    }

    SimpleResourceFloat define_resource(
        const char* route,
        float v,
        M2MBase::Operation opr,
        bool observable,
        void(*onUpdate)(float))
    {
        Callback<void(float)> fp = onUpdate;
        return define_resource(route, v, opr, observable, fp);
    }

    SimpleResourceFloat define_resource(
        const char* route,
        float v,
        Callback<void(float)> onUpdate)
    {
        return define_resource(route, v, M2MBase::GET_PUT_ALLOWED, true, onUpdate);
    }

    SimpleResourceFloat define_resource(
        const char* route,
        float v,
        void(*onUpdate)(float))
    {
        Callback<void(float)> fp = onUpdate;
        return define_resource(route, v, M2MBase::GET_PUT_ALLOWED, true, fp);
    }
};

#endif // __SIMPLE_CLOUD_CLIENT_H__
