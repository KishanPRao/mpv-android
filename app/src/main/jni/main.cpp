#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>

#include <mpv/client.h>
#include <mpv/opengl_cb.h>

#include <EGL/egl.h>

#include "main.h"

#define jfun(name) JNIEXPORT void JNICALL Java_is_xyz_mpv_MPVLib_##name

extern "C" {
    jfun(init) (JNIEnv* env, jobject obj);
    jfun(loadfile) (JNIEnv* env, jobject obj, jstring path);
    jfun(resize) (JNIEnv* env, jobject obj, jint width, jint height);
    jfun(step) (JNIEnv* env, jobject obj);
    jfun(play) (JNIEnv *env, jobject obj);
    jfun(pause) (JNIEnv *env, jobject obj);
    jfun(touch_1down) (JNIEnv* env, jobject obj, jint x, jint y);
    jfun(touch_1move) (JNIEnv* env, jobject obj, jint x, jint y);
    jfun(touch_1up) (JNIEnv* env, jobject obj, jint x, jint y);
    jfun(setconfigdir) (JNIEnv* env, jobject obj, jstring path);
};

static void die(const char *msg)
{
    ALOGE("%s", msg);
    exit(1);
}

static void *get_proc_address_mpv(void *fn_ctx, const char *name)
{
    return (void*)eglGetProcAddress(name);
}

mpv_handle *mpv;
mpv_opengl_cb_context *mpv_gl;

int g_width, g_height;
char g_delayed_load[2048];
char g_config_dir[2048];

jfun(init) (JNIEnv* env, jobject obj) {
    if (mpv)
        return;

    setlocale(LC_NUMERIC, "C");

    mpv = mpv_create();
    if (!mpv)
        die("context init failed");

    int terminal = 1;
    mpv_set_option(mpv, "terminal", MPV_FORMAT_FLAG, &terminal);
    mpv_set_option_string(mpv, "msg-level", "all=v");
    int osc = 1;
    mpv_set_option(mpv, "osc", MPV_FORMAT_FLAG, &osc);
    mpv_set_option_string(mpv, "script-opts", "osc-scalewindowed=1.5");

    mpv_set_option_string(mpv, "config", "yes");
    mpv_set_option_string(mpv, "config-dir", g_config_dir);

    if (mpv_initialize(mpv) < 0)
        die("mpv init failed");

    mpv_gl = (mpv_opengl_cb_context*)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
        die("failed to create mpv GL API handle");

    if (mpv_opengl_cb_init_gl(mpv_gl, NULL, get_proc_address_mpv, NULL) < 0)
        die("failed to initialize mpv GL context");

    if (mpv_set_option_string(mpv, "vo", "opengl-cb") < 0)
        die("failed to set VO");
    if (mpv_set_option_string(mpv, "ao", "openal") < 0)
        die("failed to set AO");

    const char *cmd[] = {"loadfile", g_delayed_load, NULL};
    mpv_command(mpv, cmd);
}

#define CHKVALID() if (!mpv) return;

jfun(loadfile) (JNIEnv* env, jobject obj, jstring jpath) {
    // TODO: We should have a direct way for java to run mpv commands instead of this
    const char *path = env->GetStringUTFChars(jpath, NULL);
    if (mpv) {
        const char *cmd[] = {"loadfile", path, NULL};
        mpv_command(mpv, cmd);
    } else {
        strncpy(g_delayed_load, path, sizeof(g_delayed_load) - 1);
    }
    env->ReleaseStringUTFChars(jpath, path);
}

static void mouse_pos(int x, int y) {
    char sx[5], sy[5];
    const char *cmd[] = {"mouse", sx, sy, NULL};
    snprintf(sx, sizeof(sx), "%d", x);
    snprintf(sy, sizeof(sy), "%d", y);
    mpv_command(mpv, cmd);
}

static void mouse_trigger(int down, int btn) {
    // "mouse" doesn't actually send keydown events so we need to do it manually
    char k[16];
    const char *cmd[] = {down?"keydown":"keyup", k, NULL};
    snprintf(k, sizeof(k), "MOUSE_BTN%d", btn);
    mpv_command(mpv, cmd);
}

jfun(touch_1down) (JNIEnv* env, jobject obj, jint x, jint y) {
    CHKVALID();
    mouse_pos(x, y);
    mouse_trigger(1, 0);
}

jfun(touch_1move) (JNIEnv* env, jobject obj, jint x, jint y) {
    CHKVALID();
    mouse_pos(x, y);
}

jfun(touch_1up) (JNIEnv* env, jobject obj, jint x, jint y) {
    CHKVALID();
    mouse_trigger(0, 0);
    // move the cursor to the top left corner where it doesn't trigger the OSC
    // FIXME: this causes the OSC to receive a mouse_btn0 up event with x and y == 0
    //        but sometimes it gets the correct coords (threading/async?)
    //mouse_pos(0, 0);
}

jfun(resize) (JNIEnv* env, jobject obj, jint width, jint height) {
    g_width = width;
    g_height = height;
}

jfun(step) (JNIEnv* env, jobject obj) {
    mpv_opengl_cb_draw(mpv_gl, 0, g_width, -g_height);
}

jfun(play) (JNIEnv* env, jobject obj) {
    CHKVALID();
    int paused = 0;
    mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

jfun(pause) (JNIEnv* env, jobject obj) {
    CHKVALID();
    int paused = 1;
    mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

jfun(setconfigdir) (JNIEnv* env, jobject obj, jstring jpath) {
    const char *path = env->GetStringUTFChars(jpath, NULL);
    strncpy(g_config_dir, path, sizeof(g_config_dir) - 1);
    env->ReleaseStringUTFChars(jpath, path);
}
