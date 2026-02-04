#include "main.h"

void
printf_notification(const char *fmt, ...)
{
    SceNotificationRequest noti;
    memset(&noti, 0, sizeof(noti));

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(noti.message, sizeof(noti.message), fmt, ap);
    va_end(ap);

    noti.type = 0;
    noti.use_icon_image_uri = 1;
    noti.target_id = -1;
    strncpy(noti.uri, "cxml://psnotification/tex_morpheus_trophy_platinum", sizeof(noti.uri)-1);

    sceKernelSendNotificationRequest(0, &noti, sizeof(noti), 0);
}

int
unlockTrophy(int userId, int trophyId)
{
	// Init NpUniversalDataSystem
	SceNpUniversalDataSystemInitParam param;
	param.size = sizeof(param);
	param.poolSize = 16 * 1024;
	if(sceNpUniversalDataSystemInitialize(&param) < 0) {
		perror("sceNpUniversalDataSystemInitialize");
		return -1;
	}

    // Create handle & context
	int unlockHandle;
	SceNpUniversalDataSystemHandle contextHandle = -1;
	unlockHandle = sceNpUniversalDataSystemCreateHandle(&contextHandle);
	if (unlockHandle < 0) {
		perror("sceNpUniversalDataSystemCreateHandle");
		return -1;
	}

	int newUnlockContext;
	SceNpUniversalDataSystemContext unlockContext = -1;
	newUnlockContext = sceNpUniversalDataSystemCreateContext(&unlockContext, userId, 0, 0);
	if (newUnlockContext < 0) {
		perror("sceNpUniversalDataSystemCreateContext");
		return -1;
	}

	// Register context
	int unlockContextReg;
	unlockContextReg = sceNpUniversalDataSystemRegisterContext(newUnlockContext, unlockHandle, 0);
	if (unlockContextReg < 0) {
		perror("sceNpUniversalDataSystemRegisterContext");
		return -1;
	}

    // Invoke unlock
    int unlockEvent;
    SceNpUniversalDataSystemEvent *_event = NULL;
    SceNpUniversalDataSystemEventPropertyObject *_prop = NULL;
    unlockEvent = sceNpUniversalDataSystemCreateEvent("_UnlockTrophy", NULL, &_event, &_prop);
    if(unlockEvent < 0) {
		perror("sceNpUniversalDataSystemCreateEvent");
        goto error;
    }
    unlockEvent = sceNpUniversalDataSystemEventPropertyObjectSetInt32(_prop, "_trophy_id", trophyId);
    if(unlockEvent < 0) {
		perror("sceNpUniversalDataSystemEventPropertyObjectSetInt32");
        goto error;
    }
    unlockEvent = sceNpUniversalDataSystemPostEvent(unlockContextReg, unlockHandle, _event, 0);
    if (unlockEvent < 0) {
		perror("sceNpUniversalDataSystemPostEvent");
        goto error;
    }

	// Destroy handle, context & unlock event
    sceNpUniversalDataSystemDestroyHandle(unlockHandle);
    sceNpUniversalDataSystemDestroyContext(unlockContextReg);
    sceNpUniversalDataSystemDestroyEvent(_event);

	// Terminate NpUniversalDataSystem
	int terminateReturn;
	terminateReturn = sceNpUniversalDataSystemTerminate();
	if (terminateReturn < 0) {
		perror("sceNpUniversalDataSystemTerminate");
		return -1;
	}

    return 0;

error:
    if (_event) {
        sceNpUniversalDataSystemDestroyEvent(_event);
    }
    return unlockEvent;
}

int
main() {
	// Notify about unlocking
	printf_notification("TEST: Unlocking trophy ID 0003 (A Masterpiece)"); // -lSceNpTrophy2 import & the tropy_model class could be used to get information about the trophy to be unlocked.

	int ret;
	int trophyID = 3; // ID from tropconf/tropmeta
	int userID = 0; // user_id from notification2.db (?)

	// Unlock
	ret = unlockTrophy(userID, trophyID);
	if(ret < 0) {
        return -1;
    }

	// Notify when done
	printf_notification("Unlocking succeeded.");
	return 0;
}
