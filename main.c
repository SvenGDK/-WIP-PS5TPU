#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ps5/kernel.h>

// System module IDs
#define SCE_SYSMODULE_NP_TROPHY2                 0x00A7
#define SCE_SYSMODULE_NP_UNIVERSAL_DATA_SYSTEM   0x00C5

// Constants
#define SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_CONTEXT  0
#define SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_HANDLE   0

// Types
typedef int32_t SceUserServiceUserId;
typedef int32_t SceNpUniversalDataSystemContext;
typedef int32_t SceNpUniversalDataSystemHandle;
typedef struct SceNpUniversalDataSystemEvent SceNpUniversalDataSystemEvent;
typedef struct SceNpUniversalDataSystemEventPropertyObject SceNpUniversalDataSystemEventPropertyObject;

typedef struct SceNpUniversalDataSystemInitParam {
    uint64_t size;
    uint64_t poolSize;
} SceNpUniversalDataSystemInitParam;

// External function declarations
//
// Sysmodule
extern int sceSysmoduleLoadModule(uint16_t id);
extern int sceSysmoduleUnloadModule(uint16_t id);
//
// User service
extern int sceUserServiceGetInitialUser(SceUserServiceUserId *userId);
//
// Universal Data System
extern int sceNpUniversalDataSystemInitialize(const SceNpUniversalDataSystemInitParam *param);
extern int sceNpUniversalDataSystemTerminate(void);
extern int sceNpUniversalDataSystemCreateContext(SceNpUniversalDataSystemContext *context, SceUserServiceUserId userId, uint64_t serviceLabel, uint64_t options);
extern int sceNpUniversalDataSystemDestroyContext(SceNpUniversalDataSystemContext context);
extern int sceNpUniversalDataSystemCreateHandle(SceNpUniversalDataSystemHandle *handle);
extern int sceNpUniversalDataSystemDestroyHandle(SceNpUniversalDataSystemHandle handle);
extern int sceNpUniversalDataSystemRegisterContext(SceNpUniversalDataSystemContext context, SceNpUniversalDataSystemHandle handle, uint64_t options);
extern int sceNpUniversalDataSystemCreateEvent(const char *eventName, void *reserved, SceNpUniversalDataSystemEvent **event, SceNpUniversalDataSystemEventPropertyObject **prop);
extern int sceNpUniversalDataSystemDestroyEvent(SceNpUniversalDataSystemEvent *event);
extern int sceNpUniversalDataSystemEventPropertyObjectSetInt32(SceNpUniversalDataSystemEventPropertyObject *prop, const char *key, int32_t value);
extern int sceNpUniversalDataSystemPostEvent(SceNpUniversalDataSystemContext context, SceNpUniversalDataSystemHandle handle, SceNpUniversalDataSystemEvent *event, uint64_t options);
//
// Notification
extern int sceKernelSendNotificationRequest(int device, const char *msg, size_t len, int blocking);
//

static const size_t kUdsMemSize = 16 * 1024;

static void notify(const char *message) {
    sceKernelSendNotificationRequest(0, message, strlen(message), 0);
}

static int loadModules(void) {
    int ret;

    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NP_UNIVERSAL_DATA_SYSTEM);
    if (ret < 0) {
        printf("Failed to load NpUniversalDataSystem module: 0x%08X\n", ret);
        return ret;
    }
    printf("NpUniversalDataSystem module loaded\n");

    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NP_TROPHY2);
    if (ret < 0) {
        printf("Failed to load NpTrophy2 module: 0x%08X\n", ret);
        return ret;
    }
    printf("NpTrophy2 module loaded\n");

    return 0;
}

static int unloadModules(void) {
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NP_TROPHY2);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NP_UNIVERSAL_DATA_SYSTEM);
    printf("Modules unloaded\n");
    return 0;
}

static int initUds(void) {
    SceNpUniversalDataSystemInitParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.poolSize = kUdsMemSize;

    int ret = sceNpUniversalDataSystemInitialize(&param);
    if (ret < 0) {
        printf("Failed to initialize UDS: 0x%08X\n", ret);
        return ret;
    }
    printf("UDS initialized\n");
    return 0;
}

static int registerUdsContext(SceNpUniversalDataSystemContext context) {
    int ret;
    SceNpUniversalDataSystemHandle handle = SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_HANDLE;

    ret = sceNpUniversalDataSystemCreateHandle(&handle);
    if (ret < 0) {
        printf("Failed to create UDS handle for registration: 0x%08X\n", ret);
        return ret;
    }

    ret = sceNpUniversalDataSystemRegisterContext(context, handle, 0);
    sceNpUniversalDataSystemDestroyHandle(handle);
    if (ret < 0) {
        printf("Failed to register UDS context: 0x%08X\n", ret);
        return ret;
    }

    printf("UDS context registered\n");
    return 0;
}

static int unlockTrophy(SceNpUniversalDataSystemContext context, int32_t trophyId) {
    int ret;
    SceNpUniversalDataSystemHandle handle = SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_HANDLE;
    SceNpUniversalDataSystemEvent *event = NULL;
    SceNpUniversalDataSystemEventPropertyObject *prop = NULL;

    // Create a handle
    ret = sceNpUniversalDataSystemCreateHandle(&handle);
    if (ret < 0) {
        printf("Failed to create UDS handle: 0x%08X\n", ret);
        return ret;
    }

    // Create the _UnlockTrophy event
    ret = sceNpUniversalDataSystemCreateEvent("_UnlockTrophy", NULL, &event, &prop);
    if (ret < 0) {
        printf("Failed to create UDS event: 0x%08X\n", ret);
        goto cleanup;
    }

    // Set the trophy ID property
    ret = sceNpUniversalDataSystemEventPropertyObjectSetInt32(prop, "_trophy_id", trophyId);
    if (ret < 0) {
        printf("Failed to set trophy ID property: 0x%08X\n", ret);
        goto cleanup;
    }

    // Post the event to invoke the unlock
    ret = sceNpUniversalDataSystemPostEvent(context, handle, event, 0);
    if (ret < 0) {
        printf("Failed to post unlock event for trophy %d: 0x%08X\n", trophyId, ret);
        goto cleanup;
    }

    printf("Trophy %d unlock event posted!\n", trophyId);
    notify("Trophy unlocked!");

cleanup:
    if (event) {
        sceNpUniversalDataSystemDestroyEvent(event);
    }
    if (handle != SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_HANDLE) {
        sceNpUniversalDataSystemDestroyHandle(handle);
    }
    return ret;
}

int main() {
    int ret;
    SceUserServiceUserId userId = -1;
    SceNpUniversalDataSystemContext udsContext = SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_CONTEXT;
    int udsInitialized = 0;

    // Trophy ID to unlock
    int32_t trophyToUnlock = 0;

    printf("PS5 Trophy Unlocker v0.1 [WIP] by SvenGDK\n");
    printf("Trophy ID to unlock: %d\n\n", trophyToUnlock);

    // Load required modules
    ret = loadModules();
    if (ret < 0) goto cleanup;

    // Initialize the Universal Data System
    ret = initUds();
    if (ret < 0) goto cleanup;
    udsInitialized = 1;

    // Get user ID
    ret = sceUserServiceGetInitialUser(&userId);
    if (ret < 0) {
        printf("Failed to get user ID: 0x%08X\n", ret);
        goto cleanup;
    }
    printf("User ID obtained: %d\n", userId);

    // Create UDS context (binds to the current game's trophy set)
    ret = sceNpUniversalDataSystemCreateContext(&udsContext, userId, 0, 0);
    if (ret < 0) {
        printf("Failed to create UDS context: 0x%08X\n", ret);
        goto cleanup;
    }
    printf("UDS context created\n");

    // Register the UDS context
    ret = registerUdsContext(udsContext);
    if (ret < 0) goto cleanup;

    // Unlock the specified trophy
    ret = unlockTrophy(udsContext, trophyToUnlock);
    if (ret < 0) goto cleanup;

    printf("\nSuccess!\n");

cleanup:
    if (udsContext != SCE_NP_UNIVERSAL_DATA_SYSTEM_INVALID_CONTEXT) {
        sceNpUniversalDataSystemDestroyContext(udsContext);
    }
    if (udsInitialized) {
        sceNpUniversalDataSystemTerminate();
    }
    unloadModules();

    return ret;
}