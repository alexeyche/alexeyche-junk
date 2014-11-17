#pragma once

#include <snnlib/base.h>

class LearningRule : public Obj {
public:
	virtual void init(const ConstObj *_c) = 0;
};

class BlankLearningRule: public LearningRule {
public:
	void init(const ConstObj *_c) {}
};
