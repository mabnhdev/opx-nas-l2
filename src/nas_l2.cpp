/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * filename: nas_l2.cpp
 */


#ifndef ORIGINAL_DELL_CODE
#include <dlfcn.h>
#endif
#include "nas_l2_init.h"
#include "nas_switch_mac.h"
#include "nas_mirror_api.h"
#include "cps_api_operation.h"
#include "nas_sflow_api.h"
#include "nas_stg_api.h"
#include "nas_mac_api.h"
#include "nas_hash_cps.h"
#include "nas_switch_cps.h"
#include "nas_switch_log.h"

#define NUM_L2_CPS_API_THREAD 1

static cps_api_operation_handle_t handle;


t_std_error (*nas_l2_init_functions[])(cps_api_operation_handle_t handle) = {
        nas_switch_mac_init,
        nas_mirroring_init,
        nas_sflow_init,
        nas_stg_init,
        nas_mac_init,
        nas_hash_init,
        nas_switch_cps_init,
        nas_switch_log_init,
};

t_std_error nas_l2_init(void) {

    if (cps_api_operation_subsystem_init(&handle,NUM_L2_CPS_API_THREAD)!=cps_api_ret_code_OK) {
        return STD_ERR(CPSNAS,FAIL,0);
    }
    size_t ix = 0;
    size_t mx = sizeof(nas_l2_init_functions)/sizeof(*nas_l2_init_functions);
    for ( ; ix < mx ; ++ix ) {
#ifndef ORIGINAL_DELL_CODE
        /*
         * The Broadcom SAI doesn't implenment STG functions yet
         * therefore if we are linked against the Broadcom SAI don't
         * attempt to call nas_stg_init() because it will SEGV due to
         * the SAI not having a a dispatch table for STG functions.
         */
        if ((nas_l2_init_functions[ix] = nas_stg_init) &&
            (dlsym(RTLD_DEFAULT, "opennsl_driver_init") != NULL))
            continue;
#endif
        t_std_error rc = nas_l2_init_functions[ix](handle);
        if (rc!=STD_ERR_OK) return rc;
    }
    return STD_ERR_OK;
}
