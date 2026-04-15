#include "ClassFlowImage.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <dirent.h>
#ifdef __cplusplus
}
#endif

#include "time_sntp.h"
#include "ClassLogFile.h"
#include "CImageBasis.h"
#include "esp_log.h"
#include "../../include/defines.h"

static const char* TAG = "FLOWIMAGE";

/* Path of the small state file that persists the circular-buffer slot on the SD card */
static const char* CBUF_STATE_FILE = "/sdcard/img_tmp/cbuf_state.ini";

/* ------------------------------------------------------------------ */
/* Circular-buffer state helpers                                        */
/* ------------------------------------------------------------------ */

static bool cbuf_read_state(int &slot, std::string &lastDate)
{
    FILE *f = fopen(CBUF_STATE_FILE, "r");
    if (!f) {
        return false;
    }
    int s = 0;
    char dateStr[16] = "";
    int matched = fscanf(f, "slot=%d\ndate=%15s\n", &s, dateStr);
    fclose(f);
    if (matched != 2) {
        return false;
    }
    slot     = s;
    lastDate = std::string(dateStr);
    return true;
}

static bool cbuf_write_state(int slot, const std::string &date)
{
    FILE *f = fopen(CBUF_STATE_FILE, "w");
    if (!f) {
        return false;
    }
    fprintf(f, "slot=%d\ndate=%s\n", slot, date.c_str());
    fclose(f);
    return true;
}

/* ------------------------------------------------------------------ */
/* ClassFlowImage constructors                                          */
/* ------------------------------------------------------------------ */

ClassFlowImage::ClassFlowImage(const char* logTag)
{
	this->logTag = logTag;
	isLogImage = false;
    disabled = false;
    this->imagesRetention = 5;
    circularBufferEnabled     = false;
    circularBufferDays        = 30;
    circularBufferCurrentSlot = 0;
    circularBufferLastDate    = "";
}

ClassFlowImage::ClassFlowImage(std::vector<ClassFlow*> * lfc, const char* logTag) : ClassFlow(lfc)
{
	this->logTag = logTag;
	isLogImage = false;
    disabled = false;
    this->imagesRetention = 5;
    circularBufferEnabled     = false;
    circularBufferDays        = 30;
    circularBufferCurrentSlot = 0;
    circularBufferLastDate    = "";
}

ClassFlowImage::ClassFlowImage(std::vector<ClassFlow*> * lfc, ClassFlow *_prev, const char* logTag) :  ClassFlow(lfc, _prev)
{
	this->logTag = logTag;
	isLogImage = false;
    disabled = false;
    this->imagesRetention = 5;
    circularBufferEnabled     = false;
    circularBufferDays        = 30;
    circularBufferCurrentSlot = 0;
    circularBufferLastDate    = "";
}

/* ------------------------------------------------------------------ */
/* updateCircularBufferSlot                                             */
/*                                                                      */
/* Called at the start of each CreateLogFolder invocation when          */
/* the circular buffer is active.  When the calendar date has           */
/* changed since the last write the slot index advances (wrapping at    */
/* circularBufferDays) and the contents of the old slot folder are      */
/* deleted so that storage is re-used.                                  */
/* ------------------------------------------------------------------ */
void ClassFlowImage::updateCircularBufferSlot(const std::string& currentDate)
{
    /* Load persisted state on first call (empty lastDate sentinel) */
    if (circularBufferLastDate.empty()) {
        int   savedSlot = 0;
        std::string savedDate;
        if (cbuf_read_state(savedSlot, savedDate)) {
            circularBufferCurrentSlot = savedSlot;
            circularBufferLastDate    = savedDate;
            ESP_LOGI(TAG, "Circular buffer state loaded: slot=%d date=%s",
                     circularBufferCurrentSlot, circularBufferLastDate.c_str());
        } else {
            /* No state file yet – start at slot 0 with today's date */
            circularBufferCurrentSlot = 0;
            circularBufferLastDate    = currentDate;
            cbuf_write_state(circularBufferCurrentSlot, circularBufferLastDate);
            ESP_LOGI(TAG, "Circular buffer state initialised: slot=0 date=%s", currentDate.c_str());
            return;
        }
    }

    if (currentDate == circularBufferLastDate) {
        return; /* Same day – keep using the current slot */
    }

    /* Date changed: advance the slot and clear the folder that will be re-used */
    int nextSlot = (circularBufferCurrentSlot + 1) % circularBufferDays;

    char slotName[16];
    snprintf(slotName, sizeof(slotName), "cbuf_%02d", nextSlot);
    std::string oldSlotPath = imagesLocation + "/" + slotName;

    /* Delete the entire old slot folder (best-effort) */
    removeFolder(oldSlotPath.c_str(), logTag);
    ESP_LOGD(TAG, "Circular buffer: advanced to slot %d, cleared %s", nextSlot, oldSlotPath.c_str());

    circularBufferCurrentSlot = nextSlot;
    circularBufferLastDate    = currentDate;
    cbuf_write_state(circularBufferCurrentSlot, circularBufferLastDate);
}

/* ------------------------------------------------------------------ */

string ClassFlowImage::CreateLogFolder(string time) {
	if (!isLogImage)
		return "";

    std::string logPath;

    if (circularBufferEnabled) {
        /* Circular-buffer mode: use a numbered slot folder            */
        /* (cbuf_00 … cbuf_NN) and advance when the date changes.      */
        std::string currentDate = time.LOGFILE_TIME_FORMAT_DATE_EXTR; /* YYYYMMDD */
        updateCircularBufferSlot(currentDate);

        char slotName[16];
        snprintf(slotName, sizeof(slotName), "cbuf_%02d", circularBufferCurrentSlot);
        logPath = imagesLocation + "/" + slotName + "/" + time.LOGFILE_TIME_FORMAT_HOUR_EXTR;
    } else {
        /* Default date-based folder structure */
        logPath = imagesLocation + "/" + time.LOGFILE_TIME_FORMAT_DATE_EXTR + "/" + time.LOGFILE_TIME_FORMAT_HOUR_EXTR;
    }

    isLogImage = mkdir_r(logPath.c_str(), S_IRWXU) == 0;
    if (!isLogImage) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't create log folder for images. Path " + logPath);
    }

	return logPath;
}

void ClassFlowImage::LogImage(string logPath, string name, float *resultFloat, int *resultInt, string time, CImageBasis *_img) {
	if (!isLogImage)
		return;
	
    
	char buf[10];

	if (resultFloat != NULL) {
        if (*resultFloat < 0)
            sprintf(buf, "N.N_");
        else
        {
            sprintf(buf, "%.1f_", *resultFloat);
            if (strcmp(buf, "10.0_") == 0)
                sprintf(buf, "0.0_");
        }
            
	} else if (resultInt != NULL) {
		sprintf(buf, "%d_", *resultInt);
	} else {
		buf[0] = '\0';
	}

	string nm = logPath + "/" + buf + name + "_" + time + ".jpg";
	nm = FormatFileName(nm);
	string output = "/sdcard/img_tmp/" + name + ".jpg";
	output = FormatFileName(output);
	ESP_LOGD(logTag, "save to file: %s", nm.c_str());
	_img->SaveToFile(nm);
//	CopyFile(output, nm);
}

void ClassFlowImage::RemoveOldLogs()
{
	if (!isLogImage)
		return;

    /* In circular-buffer mode the slot rotation in CreateLogFolder   */
    /* handles cleanup; date-based deletion is not applicable.        */
    if (circularBufferEnabled)
        return;
	
	ESP_LOGD(TAG, "remove old images");
    if (imagesRetention == 0) {
        return;
    }

    time_t rawtime;
    struct tm* timeinfo;
    char cmpfilename[30];

    time(&rawtime);
    rawtime = addDays(rawtime, -1 * imagesRetention + 1);
    timeinfo = localtime(&rawtime);
    //ESP_LOGD(TAG, "ImagefileRetentionInDays: %d", imagesRetention);
    
    strftime(cmpfilename, 30, LOGFILE_TIME_FORMAT, timeinfo);
    //ESP_LOGD(TAG, "file name to compare: %s", cmpfilename);
	string folderName = string(cmpfilename).LOGFILE_TIME_FORMAT_DATE_EXTR;

    DIR *dir = opendir(imagesLocation.c_str());
    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir: %s", imagesLocation.c_str());
        return;
    }

    struct dirent *entry;
    int deleted = 0;
    int notDeleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        string folderPath = imagesLocation + "/" + entry->d_name;
		if (entry->d_type == DT_DIR) {
			//ESP_LOGD(TAG, "Compare %s to %s", entry->d_name, folderName.c_str());	
			if ((strlen(entry->d_name) == folderName.length()) && (strcmp(entry->d_name, folderName.c_str()) < 0)) {
                removeFolder(folderPath.c_str(), logTag);
                deleted++;
			} else {
                notDeleted ++;
            }
		}
    }
    ESP_LOGD(TAG, "Image folder deleted: %d | Image folder not deleted: %d", deleted, notDeleted);	
    closedir(dir);
}

