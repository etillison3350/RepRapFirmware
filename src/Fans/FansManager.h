/*
 * FansManager.h
 *
 *  Created on: 3 Sep 2019
 *      Author: David
 */

#ifndef SRC_FANS_FANSMANAGER_H_
#define SRC_FANS_FANSMANAGER_H_

#include "RepRapFirmware.h"
#include "Fan.h"
#include "GCodes/GCodeResult.h"
#include <RTOSIface/RTOSIface.h>

class GCodeBuffer;
class LocalFan;

class FansManager
{
public:
	FansManager();
	void Init();
	bool CheckFans();
	GCodeResult ConfigureFanPort(uint32_t fanNum, GCodeBuffer& gb, const StringRef& reply);
	bool ConfigureFan(unsigned int mcode, size_t fanNum, GCodeBuffer& gb, const StringRef& reply, bool& error);
	float GetFanValue(size_t fanNum) const;
	void SetFanValue(size_t fanNum, float speed);
	bool IsFanControllable(size_t fanNum) const;
	const char *GetFanName(size_t fanNum) const;
	int32_t GetFanRPM(size_t fanNum) const;

#if HAS_MASS_STORAGE
	bool WriteFanSettings(FileStore *f) const;
#endif

private:
	ReadLockedPointer<Fan> FindFan(size_t fanNum) const;
	LocalFan *CreateLocalFan(uint32_t fanNum, const char *pinNames, PwmFrequency freq, const StringRef& reply);

	mutable ReadWriteLock fansLock;
	Fan *fans[NumTotalFans];
};

#endif /* SRC_FANS_FANSMANAGER_H_ */