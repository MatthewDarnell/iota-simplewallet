//
// Created by matth on 3/10/2020.
//

#include <cclient/api/core/core_api.h>
#include <sodium.h>
#include <cclient/api/extended/extended_api.h>
#include "../api.h"
#include "../../iota-simplewallet.h"

cJSON* get_node_info() {
  iota_client_service_t* serv = get_iota_client();
  get_node_info_res_t *node_res = get_node_info_res_new();
  if (!serv || !node_res) {
    log_wallet_error("%s: OOM\n", __func__);
    return NULL;
  }

  retcode_t ret;
  if ((ret = iota_client_get_node_info(serv, node_res)) != RC_OK) {
    log_wallet_error("Get Node Info Returned: %s", error_2_string(ret));
    iota_client_core_destroy(&serv);
    return NULL;
  }
  iota_client_core_destroy(&serv);



  char milestone[NUM_TRYTES_HASH + 1] = { 0 };
  char solid_subtangle[NUM_TRYTES_HASH + 1] = { 0 };

  flex_trits_to_trytes(
    (tryte_t*)milestone, NUM_TRYTES_HASH,
    node_res->latest_milestone,
    NUM_TRITS_HASH, NUM_TRITS_HASH);


  flex_trits_to_trytes(
    (tryte_t*)solid_subtangle, NUM_TRYTES_HASH,
    node_res->latest_solid_subtangle_milestone,
    NUM_TRITS_HASH, NUM_TRITS_HASH);


  cJSON* ret_val = cJSON_CreateObject();

  cJSON_AddStringToObject(ret_val, "appName",  get_node_info_res_app_name(node_res));
  cJSON_AddStringToObject(ret_val, "appVersion", get_node_info_res_app_version(node_res));
  cJSON_AddStringToObject(ret_val, "latestMilestone", milestone);
  cJSON_AddNumberToObject(ret_val, "latestMilestoneIndex", node_res->latest_milestone_index);
  cJSON_AddStringToObject(ret_val, "latestSolidSubtangleMilestone", solid_subtangle);
  cJSON_AddNumberToObject(ret_val, "latestSolidSubtangleMilestoneIndex", node_res->latest_solid_subtangle_milestone_index);
  cJSON_AddNumberToObject(ret_val, "neighbors", node_res->neighbors);

  get_node_info_res_free(&node_res);
  return ret_val;
}