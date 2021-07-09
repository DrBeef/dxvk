#ifndef OPENVRDIRECTMODE_H_INCLUDED
#define OPENVRDIRECTMODE_H_INCLUDED

#pragma warning (disable : 4005)

#include "HMDInterface.h"
#include "IPresentCallback.h"

#include <mutex>
#include <condition_variable>


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
	public IPresentCallback
{
public:
	OpenVRDirectMode();
	virtual ~OpenVRDirectMode();

	virtual bool Init();	

	virtual IPresentCallback* GetIPresentCallback();

	// IPresentCallback
	virtual void PrePresent();
	virtual void BeforeFirstQueueSubmit();
	virtual void PostPresentHandoff();

	//Rendering Stuff
	virtual void Present();
	virtual void GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight);
	virtual float GetEyeDistance();
	virtual void GetEyeView(int eye, float matrix[4][4], bool invert);
	virtual bool IsConnected();
	virtual void StartFrame();
	virtual void StoreSharedTexture(int index, vr::VRVulkanTextureData_t* pVRVulkanTextureData);
	virtual void GetProjection(int eye, float matrix[4][4], bool invert);
	virtual int GetCurrentRenderTexture();
	virtual int GetTotalStoredTextures();

	//Tracjing Stuff
	virtual void GetTrackingDataHMD(float hMatrix[4][4]);
	virtual void GetTrackingDataLeftController(float lcMatrix[4][4]);
	virtual void GetTrackingDataRightController(float rcMatrix[4][4]);

	//Button Stuff
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
	
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];

private:
	bool m_initialised;
	int m_nextStoredTexture;
	int m_currentRenderTexture;
	int m_lastSubmittedTexture;
	bool m_hasHMDAttached;
	bool m_PresentCalled;

	ButtonState m_buttonStates[ButtonsList::right_GestureFist + 1];
	ButtonState m_previousButtonStates[ButtonsList::right_GestureFist + 1];
};

#endif //OPENVRDIRECTMODE_H_INCLUDED