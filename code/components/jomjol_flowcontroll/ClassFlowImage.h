#pragma once

#ifndef CLASSFLOWIMAGE_H
#define CLASSFLOWIMAGE_H

#include "ClassFlow.h"

using namespace std;

class ClassFlowImage : public ClassFlow
{
protected:
	string imagesLocation;
    bool isLogImage;
    unsigned short imagesRetention;
	const char* logTag;

    /* Circular-buffer image storage (optional, enabled via config) */
    bool circularBufferEnabled;
    unsigned short circularBufferDays;  /* number of day-slots (1–30) */
    int circularBufferCurrentSlot;  /* slot currently in use (0-based) */
    string circularBufferLastDate;  /* date of the last write: "YYYYMMDD" */

    /* Update the active slot when the calendar date changes */
    void updateCircularBufferSlot(const string& currentDate);

	string CreateLogFolder(string time);
	void LogImage(string logPath, string name, float *resultFloat, int *resultInt, string time, CImageBasis *_img);


public:
	ClassFlowImage(const char* logTag);
	ClassFlowImage(std::vector<ClassFlow*> * lfc, const char* logTag);
	ClassFlowImage(std::vector<ClassFlow*> * lfc, ClassFlow *_prev, const char* logTag);
	
	void RemoveOldLogs();
};

#endif //CLASSFLOWIMAGE_H
