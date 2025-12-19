#include "engine_api.h"

/*
 * JNI bridge rewritten from the decompiled monolith (libgroovemachinemobile.so.c).
 * The original file mixed together rendering, input, asset IO and engine lifecycle.
 * Here we isolate the Android entry points so they can be replaced or reused
 * by a WebAssembly/WebGL front-end.
 *
 * NOTE: This file assumes the existence of the Engine_* functions declared in
 * include/engine_api.h. Those are still provided by the legacy binary blob and
 * will be incrementally ported to C/C++ in later steps.
 */

int Java_com_imageline_GrooveMachineMobile_ILRenderer_nativeInitGL(int viewport0,
                                                                   int viewport1) {
  GL_LoadWrapTexture();
  Engine_SetScreenSize(viewport0, viewport1, gScreenWidth, gScreenHeight);
  Engine_Create();
  return Engine_StartAudioHard();
}

int Java_com_imageline_GrooveMachineMobile_ILRenderer_nativeResize(int viewport0,
                                                                   int viewport1,
                                                                   int /*unused_env*/,
                                                                   int /*unused_obj*/,
                                                                   int width,
                                                                   int height) {
  gScreenHeight = height;
  gScreenWidth = width;
  return Engine_SetScreenSize(viewport0, viewport1, width, height);
}

GBoxUI *Java_com_imageline_GrooveMachineMobile_GrooveMachineMobileActivity_nativeUIThreadIdle(
    void) {
  return Engine_UIThreadIdle();
}

void Java_com_imageline_GrooveMachineMobile_GrooveMachineMobileActivity_nativeDone(
    void) {
  Engine_StopAudioHard();
  Engine_Destroy();
}

void *Java_com_imageline_GrooveMachineMobile_GrooveMachineMobileActivity_nativePause(
    void) {
  Engine_StopAudioHard();
  return Engine_Suspend();
}

int Java_com_imageline_GrooveMachineMobile_GrooveMachineMobileActivity_nativeResume(
    void) {
  return Engine_StartAudioHard();
}

int Java_com_imageline_GrooveMachineMobile_GrooveMachineMobileActivity_nativeAssetEnumerate(
    int env, int asset_manager, int asset_index) {
  char *path =
      (char *)(*(int(__cdecl **)(int, int, int))(*(int *)env + 676))(
          env, asset_index, 0);
  Engine_FileFound(path, 0);
  return (*(int(__cdecl **)(int, int, char *))(*(int *)env + 680))(
      env, asset_index, path);
}

int Java_com_imageline_GrooveMachineMobile_ILGLSurfaceView_nativeTouchBegan(
    int /*env*/, int /*obj*/, int pointer_index, long long packed_xy) {
  float *xy = (float *)&packed_xy;
  return Engine_Touch(0, xy[0], xy[1], pointer_index + 1);
}

int Java_com_imageline_GrooveMachineMobile_ILGLSurfaceView_nativeTouchMove(
    int /*env*/, int /*obj*/, int pointer_index, long long packed_xy) {
  float *xy = (float *)&packed_xy;
  return Engine_Touch(1, xy[0], xy[1], pointer_index + 1);
}

int Java_com_imageline_GrooveMachineMobile_ILGLSurfaceView_nativeTouchEnd(
    int /*env*/, int /*obj*/, int pointer_index, long long packed_xy) {
  float *xy = (float *)&packed_xy;
  return Engine_Touch(2, xy[0], xy[1], pointer_index + 1);
}

int Java_com_imageline_GrooveMachineMobile_ILRenderer_nativeRender(void) {
  GL_PrepareFrame(gScreenWidth, gScreenHeight);
  return Engine_RenderGraphics();
}

int Java_com_imageline_GrooveMachineMobile_GrooveMachineMobileActivity_nativeReadAssets(
    int env, int /*obj*/, int asset_manager) {
  // This mirrors the original behaviour: read the first 1000 bytes of
  // readme.txt from the asset manager as a heartbeat during startup.
  int mgr = (int)AAssetManager_fromJava(env, asset_manager);
  if (!mgr) {
    return 0;
  }

  int asset = AAssetManager_open((void *)mgr, "readme.txt", 0);
  if (!asset) {
    return 0;
  }

  char buffer[1000];
  AAsset_read(asset, buffer, sizeof(buffer));
  AAsset_close(asset);
  return 0;
}
