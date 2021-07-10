#pragma once

class VkSubmitThreadCallback {
public:
	virtual void PrePresentCallBack() = 0;
	virtual void PostPresentCallback() = 0;
};