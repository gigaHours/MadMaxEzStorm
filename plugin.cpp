#include "plugin.h"
#include "mm/imgui/imgui.h"
#include <mm/game/go/character.h>
#include "mm/game/go/vehicle.h"
#include "mm/game/cameramanager.h"
#include "mm/core/input.h"

DLLATTATCH;

void PluginAttach(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
}


bool rightShiftPressed = false;
bool enabledAirBrake = false;
float speedBrake = 20.0;
DEFHOOK(void, CPlayer__UpdateController, (void* thiz, float dt)) {
	CCharacter* ch = *(CCharacter**)((uintptr_t)thiz + 0x20);

    static CVector3f savePos;

    if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) {
        if (!rightShiftPressed) {
            rightShiftPressed = true;
            enabledAirBrake = !enabledAirBrake;
            ch->m_Invulnerable = false;
            CMatrix4f mat;
            CVehicle* playerVeh = ch->GetVehiclePtr();
            if (playerVeh) {
                playerVeh->SetVelocity(CVector3f());
                playerVeh->GetTransform(&mat);
            }
            else {
                ch->ForceNeutralState();
                ch->GetTransform(&mat);  
            }
            savePos = mat.Position();
        }
    }
    else {
        rightShiftPressed = false;
    }

    if (enabledAirBrake) {

        auto map = CAvaSingleInstance_EXE(CDeviceManager, ->GetInputManager()->GetActionMap("player"));

        speedBrake += map->GetValue("VehicleWeaponSelectLeft") * 60.f; // MWHEELUP
        speedBrake -= map->GetValue("VehicleWeaponSelectRight") * 60.f; // MWHEELDOWN

        if (speedBrake < 10.0f)
            speedBrake = 10.0f;

        CMatrix4f camMat, chMat;
        CQuaternion chQuat;
        CAvaSingle<CCameraControlManager>::Instance->GetCameraMatrix(camMat);
        float radX, radY, radZ;
        camMat.ToEuler(radX, radY, radZ);

        CVehicle* playerVeh = ch->GetVehiclePtr();
        CGameObject* goBrake = nullptr;
        ch->m_Invulnerable = true;
        if (playerVeh) {
            goBrake = playerVeh;
            playerVeh->SetVelocity(CVector3f());
            playerVeh->GetTransform(&chMat);

            chMat = camMat;
            
            chQuat.FromEuler(0, radY, 0);
        }
        else {
            goBrake = ch;
            ch->ForceNeutralState();
            ch->RotateInstantly(radY);
            ch->GetTransform(&chMat);

            chQuat.FromMatrix4(chMat);
        }

        CVector3f chPos = savePos;

        auto speedDt = speedBrake * dt;

        chPos = chPos + (chQuat * CVector3f(
            map->GetValue("MoveLeft") ? -speedDt : map->GetValue("MoveRight") ? speedDt : 0,
            0.0,
            map->GetValue("MoveForward") ? -speedDt : map->GetValue("MoveBackward") ? speedDt : 0));

        chPos.y += (GetAsyncKeyState(VK_SPACE) & 0x8000) ? (speedDt * 0.5f) : (GetAsyncKeyState(VK_LSHIFT) & 0x8000) ? (speedDt * -0.5f) : 0;

        chMat.SetPosition(chPos);

        savePos = chPos;
        goBrake->SetTransform(&chMat);
    }

    return CPlayer__UpdateController_orig(thiz, dt);
}

void PluginHooks() {
	//ImGuiRenderer::Install();

    HookMgr::Install(ADDRESS(0x1404C6670, 0x1420DEAC0), CPlayer__UpdateController_hook, CPlayer__UpdateController_orig);
}