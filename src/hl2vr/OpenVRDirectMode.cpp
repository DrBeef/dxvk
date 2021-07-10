
#include "OpenVRDirectMode.h"
#include "HMDInterface.h"

#define BUTTON_DEADZONE  0.05f;

void Matrix_SetIdentity(float matrix[4][4])
{
	memset(matrix, 0, 16 * sizeof(float));
	matrix[0][0] = matrix[1][1] = matrix[2][2] = matrix[3][3] = 1.0f;
}

inline void Swap(float& a, float& b)
{
	float tmp = a;
	a = b;
	b = tmp;
}

void MatrixTranspose(float src[4][4], float dst[4][4])
{
	if (src == dst)
	{
		Swap(dst[0][1], dst[1][0]);
		Swap(dst[0][2], dst[2][0]);
		Swap(dst[0][3], dst[3][0]);
		Swap(dst[1][2], dst[2][1]);
		Swap(dst[1][3], dst[3][1]);
		Swap(dst[2][3], dst[3][2]);
	}
	else
	{
		dst[0][0] = src[0][0]; dst[0][1] = src[1][0]; dst[0][2] = src[2][0]; dst[0][3] = src[3][0];
		dst[1][0] = src[0][1]; dst[1][1] = src[1][1]; dst[1][2] = src[2][1]; dst[1][3] = src[3][1];
		dst[2][0] = src[0][2]; dst[2][1] = src[1][2]; dst[2][2] = src[2][2]; dst[2][3] = src[3][2];
		dst[3][0] = src[0][3]; dst[3][1] = src[1][3]; dst[3][2] = src[2][3]; dst[3][3] = src[3][3];
	}
}

/**
* Constructor.
***/ 
OpenVRDirectMode::OpenVRDirectMode() : 
	m_pHMD(NULL),
	m_initialised(false),
	m_nextStoredTexture(0),
	m_currentRenderTexture(0),
	m_lastSubmittedTexture(-1),
	m_hasHMDAttached(false),
	m_PresentCalled(false)
{
	memset(m_SharedTextureHolder, 0, sizeof(m_SharedTextureHolder));
	memset(m_buttonStates, 0, sizeof(m_buttonStates));
	memset(m_previousButtonStates, 0, sizeof(m_previousButtonStates));
}

OpenVRDirectMode::~OpenVRDirectMode()
{
}


bool OpenVRDirectMode::Init()
{
	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem *pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
	{
		if (eError == vr::VRInitError_Init_HmdNotFound)
		{
			
			m_initialised = true;
			m_nRenderWidth = m_nRenderHeight = 2016;
			return true;
		}

		return false;
	}

	m_pHMD = vr::OpenVRInternal_ModuleContext().VRSystem();

	if (eError != vr::VRInitError_None)
	{
		return false;
	}

	m_hasHMDAttached = true;

	m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);

	if (!vr::VRCompositor())
	{
		return false;
	}

	//vr::VRCompositor()->SetExplicitTimingMode(vr::VRCompositorTimingMode_Explicit_ApplicationPerformsPostPresentHandoff);

	m_initialised = true;

	return true;
}

VkSubmitThreadCallback* OpenVRDirectMode::GetVkSubmitThreadCallback()
{
	return this;
}

void OpenVRDirectMode::Present()
{
	if (!m_initialised)
	{
		return;
	}

	m_PresentCalled = true;

}

void OpenVRDirectMode::PresentSync()
{
	if (!m_initialised)
	{
		return;
	}

	m_semaphore.wait();
}

void OpenVRDirectMode::PrePresent()
{
	if (!m_initialised)
	{
		return;
	}

	//Only do this if we know the PresentEx on the Device was called by the client
	if (m_PresentCalled)
	{
		//reset the flag
		m_PresentCalled = false;

		static vr::VRTextureBounds_t leftBounds = { 0.0f, 0.0f, 0.5f, 1.0f };
		static vr::VRTextureBounds_t rightBounds = { 0.5f, 0.0f, 1.0f, 1.0f };

		if (m_hasHMDAttached)
		{
			if (vr::VRCompositor())
			{
				vr::EVRCompositorError error = vr::VRCompositor()->Submit(vr::Eye_Left, &(m_SharedTextureHolder[m_currentRenderTexture].m_VRTexture), &leftBounds);
				if (error != vr::VRCompositorError_None)
				{
				}

				error = vr::VRCompositor()->Submit(vr::Eye_Right, &(m_SharedTextureHolder[m_currentRenderTexture].m_VRTexture), &rightBounds);
				if (error != vr::VRCompositorError_None)
				{
				}
			}
		}

		//Move to the next texture
		if (++m_currentRenderTexture > m_nextStoredTexture)
		{
			m_currentRenderTexture = 0;
		}
	}
}

void OpenVRDirectMode::PostPresentHandoff()
{
	if (!m_initialised)
	{
		return;
	}

	if (m_hasHMDAttached)
	{
		//PostPresentHandoff called implicitly by WaitGetPoses
		//vr::VRCompositor()->PostPresentHandoff();

		vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	}

	m_semaphore.notify();
}

float OpenVRDirectMode::GetEyeDistance()
{
	if (!m_initialised || !m_hasHMDAttached)
	{
		return 0.0f;
	}

	vr::HmdMatrix34_t rightEye = m_pHMD->GetEyeToHeadTransform(vr::Eye_Right);
	return rightEye.m[0][3];
}

void OpenVRDirectMode::GetEyeView(int eye, float matrix[4][4], bool invert)
{
	if (!m_initialised || !m_hasHMDAttached)
	{
		Matrix_SetIdentity(matrix);
		return;
	}

	vr::HmdMatrix34_t eyeView = m_pHMD->GetEyeToHeadTransform((vr::EVREye)eye);
	memcpy(matrix, eyeView.m, 16 * sizeof(float));
}

void OpenVRDirectMode::GetProjection(int eye, float matrix[4][4], bool invert)
{
	if (!m_initialised || !m_hasHMDAttached)
	{
		static float projection[16] = {
			0.64759356,			0.0,			-0.12823947,	0.0,
			0.0,				0.78749996,		0.0,			0.0,
			0.0,				0.0,			-1.0001,		-0.10001001,
			0.0,				0.0,			-1.0,			0.0 
		};
		memcpy(matrix, projection, 16 * sizeof(float));

		return;
	}

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix((vr::EVREye)eye, 0.1f, 300.0f);
	memcpy(matrix, mat.m, 16 * sizeof(float));
}

void OpenVRDirectMode::GetTrackingDataHMD(float hMatrix[4][4])
{
	if (m_hasHMDAttached && m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		Matrix_SetIdentity(hMatrix);
		memcpy(hMatrix, m_rTrackedDevicePose[0].mDeviceToAbsoluteTracking.m, 12 * sizeof(float));
		MatrixTranspose(hMatrix, hMatrix);
	}
}

void OpenVRDirectMode::GetTrackingDataLeftController(float lcMatrix[4][4])
{
	if (!m_hasHMDAttached)
	{
		return;
	}

	vr::TrackedDeviceIndex_t leftIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);

	if (m_rTrackedDevicePose[leftIndex].bPoseIsValid)
	{
		Matrix_SetIdentity(lcMatrix);
		memcpy(lcMatrix, m_rTrackedDevicePose[leftIndex].mDeviceToAbsoluteTracking.m, 12 * sizeof(float));
		MatrixTranspose(lcMatrix, lcMatrix);
	}
}

void OpenVRDirectMode::GetTrackingDataRightController(float rcMatrix[4][4])
{
	if (!m_hasHMDAttached)
	{
		return;
	}
	vr::TrackedDeviceIndex_t rightIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

	if (m_rTrackedDevicePose[rightIndex].bPoseIsValid)
	{
		Matrix_SetIdentity(rcMatrix);
		memcpy(rcMatrix, m_rTrackedDevicePose[rightIndex].mDeviceToAbsoluteTracking.m, 12 * sizeof(float));
		MatrixTranspose(rcMatrix, rcMatrix);
	}
}

void OpenVRDirectMode::GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight)
{
	*pnWidth = m_nRenderWidth;
	*pnHeight = m_nRenderHeight;
}

bool OpenVRDirectMode::IsConnected()
{
	if (!m_hasHMDAttached)
	{
		return m_initialised;
	}

	return m_pHMD != NULL && m_pHMD->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd);
}

void OpenVRDirectMode::SetClickButtonState(vr::VRControllerState_t &state, ButtonList button, vr::EVRButtonId vrButtonId)
{
	m_buttonStates[button].m_touched = (state.ulButtonTouched & vr::ButtonMaskFromId(vrButtonId)) == vr::ButtonMaskFromId(vrButtonId);
	m_buttonStates[button].m_value = (state.ulButtonPressed & vr::ButtonMaskFromId(vrButtonId)) ? 1.0f : 0.0f;
}

void OpenVRDirectMode::SetXAxisState(vr::VRControllerState_t &state, ButtonList button, vr::EVRButtonId vrButtonId)
{
	m_buttonStates[button].m_touched = (state.ulButtonTouched & vr::ButtonMaskFromId(vrButtonId)) == vr::ButtonMaskFromId(vrButtonId);
	m_buttonStates[button].m_value = state.rAxis[vrButtonId -vr::k_EButton_Axis0].x;
}

void OpenVRDirectMode::SetYAxisState(vr::VRControllerState_t &state, ButtonList button, vr::EVRButtonId vrButtonId)
{
	m_buttonStates[button].m_value = state.rAxis[vrButtonId - vr::k_EButton_Axis0].y;
}

bool  OpenVRDirectMode::GetButtonHasChanged(ButtonList buttonID, ControllerType controllerType)
{
	return m_buttonStates[buttonID].m_value != m_previousButtonStates[buttonID].m_value;
}

bool  OpenVRDirectMode::GetButtonIsTouched(ButtonList buttonID, ControllerType controllerType)
{
	return m_buttonStates[buttonID].m_touched;
}

bool  OpenVRDirectMode::GetButtonIsPressed(ButtonList buttonID, ControllerType controllerType)
{
	return fabs(m_buttonStates[buttonID].m_value) > BUTTON_DEADZONE;
}

bool  OpenVRDirectMode::GetButtonIsDownFrame(ButtonList buttonID, ControllerType controllerType)
{
	//What does this mean?!
	return false;
}

bool  OpenVRDirectMode::GetButtonIsUpFrame(ButtonList buttonID, ControllerType controllerType)
{
	//What does this mean?!
	return false;
}

float OpenVRDirectMode::GetButtonValue(ButtonList buttonID, ControllerType controllerType)
{
	return m_buttonStates[buttonID].m_value;
}


void OpenVRDirectMode::StartFrame()
{
	if (!m_initialised || !m_hasHMDAttached || !vr::VRCompositor())
	{
		return;
	}

	//Store previous frame's controller state
	memcpy(m_previousButtonStates, m_buttonStates, sizeof(m_buttonStates));

	{
		vr::TrackedDeviceIndex_t leftIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(leftIndex, &state, sizeof(state)))
		{
			SetClickButtonState(state, ButtonList::left_Menu, vr::k_EButton_System);
			SetClickButtonState(state, ButtonList::left_ButtonA, vr::k_EButton_A);
			SetClickButtonState(state, ButtonList::left_ButtonB, vr::k_EButton_ApplicationMenu); //B Button
			SetClickButtonState(state, ButtonList::left_Pad, vr::k_EButton_SteamVR_Touchpad);

			SetXAxisState(state, ButtonList::left_Bumper, vr::k_EButton_Axis2);
			SetXAxisState(state, ButtonList::left_Trigger, vr::k_EButton_SteamVR_Trigger);

			SetXAxisState(state, ButtonList::left_PadXAxis, vr::k_EButton_SteamVR_Touchpad);
			SetYAxisState(state, ButtonList::left_PadYAxis, vr::k_EButton_SteamVR_Touchpad); 
		}
	}

	{
		vr::TrackedDeviceIndex_t rightIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(rightIndex, &state, sizeof(state)))
		{
			SetClickButtonState(state, ButtonList::right_Menu, vr::k_EButton_System);
			SetClickButtonState(state, ButtonList::right_ButtonA, vr::k_EButton_A);
			SetClickButtonState(state, ButtonList::right_ButtonB, vr::k_EButton_ApplicationMenu); //B Button
			SetClickButtonState(state, ButtonList::right_Pad, vr::k_EButton_SteamVR_Touchpad);

			SetXAxisState(state, ButtonList::right_Bumper, vr::k_EButton_Axis2);
			SetXAxisState(state, ButtonList::right_Trigger, vr::k_EButton_SteamVR_Trigger);

			SetXAxisState(state, ButtonList::right_PadXAxis, vr::k_EButton_SteamVR_Touchpad);
			SetYAxisState(state, ButtonList::right_PadYAxis, vr::k_EButton_SteamVR_Touchpad);
		}
	}
}

void OpenVRDirectMode::StoreSharedTexture(int index, vr::VRVulkanTextureData_t* pVRVulkanTextureData)
{
	m_nextStoredTexture = index;

	if (m_hasHMDAttached)
	{
		{
			memcpy(&m_SharedTextureHolder[m_nextStoredTexture].m_VulkanData, pVRVulkanTextureData, sizeof(vr::VRVulkanTextureData_t));

			m_SharedTextureHolder[m_nextStoredTexture].m_VRTexture.handle = &m_SharedTextureHolder[m_nextStoredTexture].m_VulkanData;
			m_SharedTextureHolder[m_nextStoredTexture].m_VRTexture.eColorSpace = vr::ColorSpace_Auto;
			m_SharedTextureHolder[m_nextStoredTexture].m_VRTexture.eType = vr::TextureType_Vulkan;
		}
	}
}

int OpenVRDirectMode::GetCurrentRenderTexture()
{
	return m_currentRenderTexture;
}

int OpenVRDirectMode::GetTotalStoredTextures()
{
	return m_nextStoredTexture+1;
}


