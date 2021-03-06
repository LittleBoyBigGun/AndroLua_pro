/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_handle.h"
#include "lluv_prepare.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <assert.h>

#define LLUV_PREPARE_NAME LLUV_PREFIX" Prepare"
static const char *LLUV_PREPARE = LLUV_PREPARE_NAME;

LLUV_INTERNAL int lluv_prepare_index(lua_State *L){
  return lluv__index(L, LLUV_PREPARE, lluv_handle_index);
}

LLUV_IMPL_SAFE(lluv_prepare_create){
  lluv_loop_t   *loop   = lluv_opt_loop_ex(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *handle = lluv_handle_create(L, UV_PREPARE, safe_flag | INHERITE_FLAGS(loop));
  int err = uv_prepare_init(loop->handle, LLUV_H(handle, uv_prepare_t));
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_prepare(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_PREPARE, idx, LLUV_PREPARE_NAME" expected");

  return handle;
}

static void lluv_on_prepare_start(uv_prepare_t *arg){
  lluv_on_handle_start((uv_handle_t*)arg);
}

static int lluv_prepare_start(lua_State *L){
  lluv_handle_t *handle = lluv_check_prepare(L, 1, LLUV_FLAG_OPEN);
  int err;

  lluv_check_args_with_cb(L, 2);
  LLUV_START_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_prepare_start(LLUV_H(handle, uv_prepare_t), lluv_on_prepare_start);

  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_START);

  return lluv_return(L, handle, LLUV_START_CB(handle), err);
}

static int lluv_prepare_stop(lua_State *L){
  lluv_handle_t *handle = lluv_check_prepare(L, 1, LLUV_FLAG_OPEN);
  int err = uv_prepare_stop(LLUV_H(handle, uv_prepare_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lluv_handle_unlock(L, handle, LLUV_LOCK_START);

  lua_settop(L, 1);
  return 1;
}

static const struct luaL_Reg lluv_prepare_methods[] = {
  { "start",      lluv_prepare_start      },
  { "stop",       lluv_prepare_stop       },

  {NULL,NULL}
};

#define LLUV_FUNCTIONS(F)               \
  {"prepare", lluv_prepare_create_##F}, \

static const struct luaL_Reg lluv_functions[][2] = {
  {
    LLUV_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_prepare_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_PREPARE, lluv_prepare_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
}
