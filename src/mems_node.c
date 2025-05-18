#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "mems.h"

// Initialize MeMS
static napi_value Init(napi_env env, napi_callback_info info) {
  mems_init();
  return NULL;
}

// Allocate memory
static napi_value Malloc(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  napi_get_cb_info(env, info, &argc, args, NULL, NULL);

  if (argc != 1) {
    napi_throw_error(env, NULL, "Expected 1 argument: size");
    return NULL;
  }

  uint32_t size;
  napi_get_value_uint32(env, args[0], &size);

  void *ptr = mems_malloc(size);
  if (!ptr) {
    napi_throw_error(env, NULL, "Memory allocation failed");
    return NULL;
  }

  napi_value result;
  napi_create_bigint_uint64(env, (uint64_t)ptr, &result);
  return result;
}

// Get physical address
static napi_value Get(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  napi_get_cb_info(env, info, &argc, args, NULL, NULL);

  if (argc != 1) {
    napi_throw_error(env, NULL, "Expected 1 argument: virtual address");
    return NULL;
  }

  uint64_t v_ptr;
  bool lossless;
  napi_get_value_bigint_uint64(env, args[0], &v_ptr, &lossless);
  printf("Get: v_ptr=%lu, lossless=%d\n", v_ptr, lossless);

  void *phy_ptr = mems_get((void *)v_ptr);
  if (!phy_ptr) {
    napi_throw_error(env, NULL, "Invalid virtual address");
    return NULL;
  }

  napi_value result;
  napi_create_bigint_uint64(env, (uint64_t)phy_ptr, &result);
  return result;
}

// Free memory
static napi_value Free(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  napi_get_cb_info(env, info, &argc, args, NULL, NULL);

  if (argc != 1) {
    napi_throw_error(env, NULL, "Expected 1 argument: virtual address");
    return NULL;
  }

  uint64_t ptr;
  bool lossless;
  napi_get_value_bigint_uint64(env, args[0], &ptr, &lossless);
  printf("Free: ptr=%lu, lossless=%d\n", ptr, lossless);

  mems_free((void *)ptr);
  return NULL;
}

// Print stats
static napi_value PrintStats(napi_env env, napi_callback_info info) {
  mems_print_stats();
  return NULL;
}

// Finish MeMS
static napi_value Finish(napi_env env, napi_callback_info info) {
  mems_finish();
  return NULL;
}

// Module initialization
static napi_value InitModule(napi_env env, napi_value exports) {
  napi_value fn;

  napi_create_function(env, NULL, 0, Init, NULL, &fn);
  napi_set_named_property(env, exports, "init", fn);

  napi_create_function(env, NULL, 0, Malloc, NULL, &fn);
  napi_set_named_property(env, exports, "malloc", fn);

  napi_create_function(env, NULL, 0, Get, NULL, &fn);
  napi_set_named_property(env, exports, "get", fn);

  napi_create_function(env, NULL, 0, Free, NULL, &fn);
  napi_set_named_property(env, exports, "free", fn);

  napi_create_function(env, NULL, 0, PrintStats, NULL, &fn);
  napi_set_named_property(env, exports, "printStats", fn);

  napi_create_function(env, NULL, 0, Finish, NULL, &fn);
  napi_set_named_property(env, exports, "finish", fn);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, InitModule)
