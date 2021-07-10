#pragma once

class VkSubmitThreadCallback {
public:
	virtual void PrePresent() = 0;
	virtual void PostPresentHandoff() = 0;
};