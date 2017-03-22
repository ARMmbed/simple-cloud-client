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

#ifndef MBED_CLOUD_CLIENT_USER_CONFIG_H
#define MBED_CLOUD_CLIENT_USER_CONFIG_H

#define MBED_CLOUD_CLIENT_SUPPORT_CLOUD
#define MBED_CLOUD_CLIENT_CLOUD_ADDRESS          "identity.mbedcloud.com"
#define MBED_CLOUD_CLIENT_CLOUD_PORT             5683
#define MBED_CLOUD_CLIENT_ENDPOINT_TYPE          "default"
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
#define MBED_CLOUD_CLIENT_LIFETIME               600

#define MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#define MBED_CLOUD_CLIENT_UPDATE_ID
#define MBED_CLOUD_CLIENT_UPDATE_CERT
#define MBED_CLOUD_CLIENT_UPDATE_BUFFER          2048

#endif /* MBED_CLOUD_CLIENT_USER_CONFIG_H */
