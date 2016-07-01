#pragma once

#include "../Thing.h"


class Robot: public Thing
{
public:
	typedef Gallant::Signal0<void> Callback;
	typedef Gallant::Signal1<Robot*> TaskCallback;

	Robot(const std::string& name, const std::string& sprite_path);

	virtual ~Robot();

	void startTask(int turns);

	int fuelCellAge() const { return mFuelCellAge; }
	int turnsToCompleteTask() const { return mTurnsToCompleteTask; }

	bool selfDestruct() const { return mSelfDestruct; }
	void seldDestruct(bool _b) { mSelfDestruct = _b; }

	bool idle() const { return turnsToCompleteTask() == 0; }

	TaskCallback& taskComplete() { return mTaskCompleteCallback; }
	Callback& selfDestruct() { return mSelfDestructCallback; }

protected:

	int incrementFuelCellAge() { mFuelCellAge++; }

	void updateTask();

private:

	int				mFuelCellAge;
	int				mTurnsToCompleteTask;
	bool			mSelfDestruct;

	TaskCallback	mTaskCompleteCallback;
	Callback		mSelfDestructCallback;
};
