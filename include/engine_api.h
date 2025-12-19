#ifndef ENGINE_API_H
#define ENGINE_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GBoxUI GBoxUI;

extern int gScreenWidth;
extern int gScreenHeight;

/* OpenGL bridge */
void GL_LoadWrapTexture(void);
void GL_PrepareFrame(int width, int height);

/* Core engine hooks extracted from the original JNI layer */
int Engine_SetScreenSize(int viewport0, int viewport1, int width, int height);
void Engine_Create(void);
int Engine_StartAudioHard(void);
void Engine_StopAudioHard(void);
void Engine_Destroy(void);
void *Engine_Suspend(void);
GBoxUI *Engine_UIThreadIdle(void);
void Engine_FileFound(const char *path, int user_flag);
int Engine_Touch(int phase, float x, float y, int touch_id_plus_one);
int Engine_RenderGraphics(void);

/* Android asset accessors (JNI indirections) */
void *AAssetManager_fromJava(int env, int asset_manager);
int AAssetManager_open(void *mgr, const char *name, int mode);
int AAsset_read(int asset, void *buf, int len);
void AAsset_close(int asset);

#ifdef __cplusplus
}
#endif

#endif /* ENGINE_API_H */
