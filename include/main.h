#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSMODULE_NP_UNIVERSAL_DATA_SYSTEM			0x0105 // Firmware variable (NP_SYSMODULE_LOAD_MODULE & NP_SYSMODULE_UNLOAD_MODULE) or fixed ?
#define NP_UNIVERSAL_DATA_SYSTEM_INVALID_CONTEXT		(-1)
#define NP_UNIVERSAL_DATA_SYSTEM_INVALID_HANDLE			(-1)

/* Full PS5 notification struct */
typedef struct {
    int type;                //0x00
    int req_id;              //0x04
    int priority;            //0x08
    int msg_id;              //0x0C
    int target_id;           //0x10
    int user_id;             //0x14
    int unk1;                //0x18
    int unk2;                //0x1C
    int app_id;              //0x20
    int error_num;           //0x24
    int unk3;                //0x28
    char use_icon_image_uri; //0x2C
    char message[1024];      //0x2D
    char uri[1024];          //0x42D
    char unkstr[1024];       //0x82D
} SceNotificationRequest;   //Size = 0xC30

int sceKernelSendNotificationRequest(int device, SceNotificationRequest *req, size_t size, int blocking);

void printf_notification(const char *fmt, ...);

int sceSysmoduleLoadModule(uint16_t id);
int sceSysmoduleUnloadModule(uint16_t id);

int loadNpUniversalDataSystemLib();
int unloadNpUniversalDataSystemLib();

typedef struct SceNpUniversalDataSystemInitParam {
	size_t size;
	size_t poolSize;
} SceNpUniversalDataSystemInitParam;

typedef struct SceNpUniversalDataSystemEvent SceNpUniversalDataSystemEvent;
typedef struct SceNpUniversalDataSystemEventPropertyObject SceNpUniversalDataSystemEventPropertyObject;

typedef int32_t SceNpUniversalDataSystemContext;
typedef int32_t SceNpUniversalDataSystemHandle;

typedef int32_t SceUserServiceUserId;
typedef uint32_t SceNpServiceLabel;

int sceNpUniversalDataSystemCreateHandle(
	SceNpUniversalDataSystemHandle *handle
	);

int sceNpUniversalDataSystemDestroyHandle(
	SceNpUniversalDataSystemHandle handle
	);

int sceNpUniversalDataSystemCreateContext(
	SceNpUniversalDataSystemContext *context,
	SceUserServiceUserId userId,
	SceNpServiceLabel serviceLabel,
	uint64_t options
	);

int sceNpUniversalDataSystemDestroyContext(
	SceNpUniversalDataSystemContext context
	);

int sceNpUniversalDataSystemRegisterContext(
	SceNpUniversalDataSystemContext context,
	SceNpUniversalDataSystemHandle handle,
	uint64_t options
	);

int sceNpUniversalDataSystemCreateEvent(
	const char *eventName,
	const SceNpUniversalDataSystemEventPropertyObject *prop,
	SceNpUniversalDataSystemEvent **newEvent,
	SceNpUniversalDataSystemEventPropertyObject **propPtr
	);

int sceNpUniversalDataSystemEventPropertyObjectSetInt32(
	SceNpUniversalDataSystemEventPropertyObject *object,
	const char *key,
	int32_t value
	);
	
int sceNpUniversalDataSystemPostEvent(
	SceNpUniversalDataSystemContext context,
	SceNpUniversalDataSystemHandle handle,
	const SceNpUniversalDataSystemEvent *event,
	uint64_t options
	);
	
int sceNpUniversalDataSystemDestroyEvent(
	SceNpUniversalDataSystemEvent *event
	);
	
int sceNpUniversalDataSystemInitialize(
	const SceNpUniversalDataSystemInitParam *param
	);
	
int sceNpUniversalDataSystemTerminate();

int unlockTrophy(
	int userId,
	int trophyId
);
