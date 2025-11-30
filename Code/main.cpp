#include <SmSdk/Creation/ControllerBase.hpp>
#include <SmSdk/Creation/ChildShape.hpp>
#include <SmSdk/TimestampCheck.hpp>
#include <SmSdk/offsets.hpp>

#include "Utils/Console.hpp"

#include <Windows.h>
#include <MinHook.h>

#pragma comment(lib, "User32.lib")

static bool ms_mhInitialized = false;
static bool ms_mhHooksAttached = false;

#define DEFINE_HOOK(address, detour, original) \
	MH_CreateHook((LPVOID)(v_mod_base + address), (LPVOID)detour, (LPVOID*)&original)

static std::uint32_t (*o_getLogicControllerConnectionColor)(ControllerBase*) = nullptr;
std::uint32_t h_getLogicControllerConnectionColor(ControllerBase* self)
{
	return self->parent_shape->getColor().data;
}

#if _SM_VERSION_NUM == 073776
# define CC_GET_LOGIC_CONNECTION_COLOR_ADDR 0x974780
#elif _SM_VERSION_NUM == 072775
# define CC_GET_LOGIC_CONNECTION_COLOR_ADDR 0x974B10
#else
# define CC_GET_LOGIC_CONNECTION_COLOR_ADDR 0x963890
#endif

void process_attach()
{
	if (!SmSdk::CheckTimestamp(_SM_TIMESTAMP_073_776))
	{
		MessageBoxA(
			NULL,
			"Your game version is not supported by Colorable Connections. The current version of the mod has been built for Scrap Mechanic 0.7.2.775\n\nPress OK to continue loading without the mod.",
			"Unsupported Version",
			MB_ICONWARNING);
		return;
	}

	AttachDebugConsole();

	if (MH_Initialize() != MH_OK)
	{
		DebugErrorL("Couldn't initialize minHook");
		return;
	}

	ms_mhInitialized = true;

	//Do the hooking here
	const std::uintptr_t v_mod_base = std::uintptr_t(GetModuleHandle(NULL));
	if (DEFINE_HOOK(CC_GET_LOGIC_CONNECTION_COLOR_ADDR, h_getLogicControllerConnectionColor, o_getLogicControllerConnectionColor) != MH_OK)
		return;

	ms_mhHooksAttached = MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
}

void process_detach(HMODULE hmod)
{
	if (ms_mhInitialized)
	{
		if (ms_mhHooksAttached)
			MH_DisableHook(MH_ALL_HOOKS);

		MH_Uninitialize();
	}

	FreeLibrary(hmod);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		process_attach();
		break;
	case DLL_PROCESS_DETACH:
		process_detach(hModule);
		break;
	}

	return TRUE;
}