#include "main.h"

int
loadNpUniversalDataSystemLib()
{
	int ret;
	ret = sceSysmoduleLoadModule(0x0105); // NP_UNIVERSAL_DATA_SYSTEM
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int
unloadNpUniversalDataSystemLib()
{
	int ret;
	ret = sceSysmoduleUnloadModule(0x0105);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int
parse_int(const char *s, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);

    if (end == s) {
        return 1;
    }
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) {
        return 2;
    }
    *out = (int)v;
    return 0;
}

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
	// Load NpUniversalDataSystemLib
	int loadRet;
	loadRet = loadNpUniversalDataSystemLib();
	if (loadRet < 0) {
		perror("loadNpUniversalDataSystemLib");
		return -1;
	}

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

	// Unload NpUniversalDataSystemLib
	int unloadRet;
	unloadRet = unloadNpUniversalDataSystemLib();
	if (unloadRet < 0) {
		perror("unloadNpUniversalDataSystemLib");
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
main(int argc, char *argv[]) {
	if(argc == 1)
	{
		printf_notification("User or Trophy ID is missing.");
		return -1;
	}
	else if(argc >= 2)
	{
		int ret;
		int trophyID, userID;

		// Parse *argv[] to int values
		int parsedUserID = parse_int(argv[0], &userID); // user_id from notification2.db (? not sure)
		int parsedTrophyID = parse_int(argv[1], &trophyID); // ID can be found in tropconf/tropmeta

		// Notify about unlocking
		printf_notification("TEST: Unlocking trophy ID %d", parsedTrophyID); // -lSceNpTrophy2 import & the tropy_model class could be used to get information about the trophy to be unlocked.

		// Unlock
		ret = unlockTrophy(parsedUserID, parsedTrophyID);
		if(ret < 0) {
			return -1;
		}

		// Notify when done
		printf_notification("Unlocking succeeded.");

	}

	return 0;
}
