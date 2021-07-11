#ifndef OPENVRDIRECTMODE_H_INCLUDED
#define OPENVRDIRECTMODE_H_INCLUDED

#pragma warning (disable : 4005)

#include "HMDInterface.h"
#include "VkSubmitThreadCallback.h"

#include <mutex>
#include <condition_variable>
#include <atomic>


struct SharedTextureHolder {
	vr::VRVulkanTextureData_t m_VulkanData;
	vr::Texture_t			m_VRTexture;
};

struct ButtonState {
	bool m_touched;
	float m_value;
};

/**
* OpenVR Direct Mode render class.
*/
class OpenVRDirectMode : public HMDInterface,
	public VkSubmitThreadCallback
{
public:
	OpenVRDirectMode();
	virtual ~OpenVRDirectMode();

	virtual bool Init();	

	virtual VkSubmitThreadCallback* GetVkSubmitThreadCallback();

	// VkSubmitThreadCallback
	virtual void PrePresentCallBack();
	virtual void PostPresentCallback();

	//HMDInterface
	virtual void PrePresent();
	virtual void PostPresent();
	virtual void GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight);
	virtual float GetEyeDistance();
	virtual void GetEyeView(int eye, float matrix[4][4], bool invert);
	virtual bool IsConnected();
	virtual void StartFrame();
	virtual void StoreSharedTexture(int index, vr::VRVulkanTextureData_t* pVRVulkanTextureData);
	virtual void GetProjection(int eye, float matrix[4][4], bool invert);
	virtual int GetCurrentRenderTexture();
	virtual int GetTotalStoredTextures();

	//HMDInterface Tracking Stuff
	virtual void GetTrackingDataHMD(float hMatrix[4][4]);
	virtual void GetTrackingDataLeftController(float lcMatrix[4][4]);
	virtual void GetTrackingDataRightController(float rcMatrix[4][4]);

	//HMDInterface Button Stuff
	virtual bool  GetButtonHasChanged(ButtonList buttonID, ControllerType controllerType);
	virtual bool  GetButtonIsTouched(ButtonList buttonID, ControllerType controllerType);
	virtual bool  GetButtonIsPressed(ButtonList buttonID, ControllerType controllerType);
	virtual bool  GetButtonIsDownFrame(ButtonList buttonID, ControllerType controllerType);
	virtual bool  GetButtonIsUpFrame(ButtonList buttonID, ControllerType controllerType);
	virtual float GetButtonValue(ButtonList buttonID, ControllerType controllerType);

private:
	void SetClickButtonState(vr::VRControllerState_t &state, ButtonList button, vr::EVRButtonId vrButtonId);
	void SetXAxisState(vr::VRControllerState_t &state, ButtonList button, vr::EVRButtonId vrButtonId);
	void SetYAxisState(vr::VRControllerState_t &state, ButtonList button, vr::EVRButtonId vrButtonId);

private:
	SharedTextureHolder			m_SharedTextureHolder[5];

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	
private:
	bool m_initialised;
	int m_nextStoredTexture;
	int m_currentRenderTexture;
	bool m_hasHMDAttached;

	std::atomic<int>   m_submitTexture = { -1 };
	std::atomic<bool>   m_posesStale = { true };

	std::mutex m_mutex;
	std::condition_variable m_cv;

	ButtonState m_buttonStates[ButtonsList::right_GestureFist + 1];
	ButtonState m_previousButtonStates[ButtonsList::right_GestureFist + 1];
};

#endif //OPENVRDIRECTMODE_H_INCLUDED