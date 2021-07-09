#pragma once

class IPresentCallback {
public:
	virtual void PrePresent() = 0;
	virtual void BeforeFirstQueueSubmit() = 0;
	virtual void PostPresentHandoff() = 0;
};