
#ifndef HMDINTERFACE_H_INCLUDED
#define HMDINTERFACE_H_INCLUDED

#include "../../include/openvr/openvr.hpp"

#include "IPresentCallback.h"
#include "stCommon.h"

//Generic HMD interface in case we do want to add other APIs other than SteamVR
class HMDInterface
{
public:
	static HMDInterface* Get();

	virtual ~HMDInterface() {}

	virtual bool Init() = 0;

	virtual IPresentCallback* GetIPresentCallback() = 0;

	//Rendering Stuff
	virtual void Present() = 0;
	virtual void PrePresent() = 0;
	virtual void GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight) = 0;
	virtual float GetEyeDistance() = 0;
	virtual void GetEyeView(int eye, float matrix[4][4], bool invert) = 0;
	virtual bool IsConnected() = 0;
	virtual void StartFrame() = 0;
	virtual void StoreSharedTexture(int index, vr::VRVulkanTextureData_t* pVRVulkanTextureData) = 0;
	virtual void GetProjection(int eye, float matrix[4][4], bool invert) = 0;
	virtual int GetCurrentRenderTexture() = 0;
	virtual int GetTotalStoredTextures() = 0;

	//Tracjing Stuff
	virtual void GetTrackingDataHMD(float hMatrix[4][4]) = 0;
	virtual void GetTrackingDataLeftController(float lcMatrix[4][4]) = 0;
	virtual void GetTrackingDataRightController(float rcMatrix[4][4]) = 0;

	//Button Stuff
	virtual bool  GetButtonHasChanged(ButtonList buttonID, ControllerType controllerType) = 0;
	virtual bool  GetButtonIsTouched(ButtonList buttonID, ControllerType controllerType) = 0;
	virtual bool  GetButtonIsPressed(ButtonList buttonID, ControllerType controllerType) = 0;
	virtual bool  GetButtonIsDownFrame(ButtonList buttonID, ControllerType controllerType) = 0;
	virtual bool  GetButtonIsUpFrame(ButtonList buttonID, ControllerType controllerType) = 0;
	virtual float GetButtonValue(ButtonList buttonID, ControllerType controllerType) = 0;

};

#endif