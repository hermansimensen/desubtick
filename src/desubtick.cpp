#include "desubtick.h"

#include "utils/addresses.h"

#include "tier0/memdbgon.h"
#include "utils/cdetour.h"

DesubtickPlugin g_DesubtickPlugin;
CUtlVector<CDetourBase*> g_vecDetours;
ISmmAPI* g_MetaAPI;
IVEngineServer2* pEngine = nullptr;
QAngle LastViewAngles;

CPlayer_MovementServices* g_pMovementServices;

#define DECLARE_BASEPLAYER_DETOUR(name) DECLARE_DETOUR(name, BasePlayer::Detour_##name, &modules::server);
#define DECLARE_BASEPLAYER_EXTERN_DETOUR(name) extern CDetour<decltype(BasePlayer::Detour_##name)> name;

namespace BasePlayer
{
	void FASTCALL Detour_ProcessUsercmds(CBasePlayerPawn* self, CUserCmd* usercmd, int totalcmds, bool paused);
	void FASTCALL Detour_ProcessMovement(CPlayer_MovementServices* s, CMoveData* mv);
}

DECLARE_BASEPLAYER_EXTERN_DETOUR(ProcessUsercmds);
DECLARE_BASEPLAYER_DETOUR(ProcessUsercmds);
DECLARE_BASEPLAYER_EXTERN_DETOUR(ProcessMovement);
DECLARE_BASEPLAYER_DETOUR(ProcessMovement);


PLUGIN_EXPOSE(DesubtickPlugin, g_DesubtickPlugin);

bool DesubtickPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	modules::server = new CModule(GAMEBIN, "server");
	
	GET_V_IFACE_CURRENT(GetEngineFactory, pEngine, IVEngineServer2, INTERFACEVERSION_VENGINESERVER);

	INIT_DETOUR(ProcessUsercmds);
	INIT_DETOUR(ProcessMovement);

	return true;
}

CGlobalVars* GetServerGlobals()
{
	return pEngine->GetServerGlobals();
}

void FASTCALL BasePlayer::Detour_ProcessMovement(CPlayer_MovementServices* ms, CMoveData* mv)
{
	g_pMovementServices = ms;

	float nearest_tick = floor(GetServerGlobals()->curtime * 64.f);
	float remainder = (GetServerGlobals()->curtime * 64.f) - nearest_tick;
	if (remainder < 0.49f)
	{
		QAngle vecCopy = QAngle();
		memcpy(&vecCopy, &mv->m_vecViewAngles, sizeof(QAngle));

		QAngle newAngles = mv->m_vecViewAngles;
		QAngle oldAngles = LastViewAngles;
		if (newAngles[YAW] - oldAngles[YAW] > 180)
		{
			newAngles[YAW] -= 360.0f;
		}
		else if (newAngles[YAW] - oldAngles[YAW] < -180)
		{
			newAngles[YAW] += 360.0f;
		}

		for (u32 i = 0; i < 3; i++)
		{
			newAngles[i] += oldAngles[i];
			newAngles[i] *= 0.5f;
		}

		mv->m_vecViewAngles = newAngles;
		ProcessMovement(ms, mv);
		mv->m_vecViewAngles = vecCopy;
		return;
	}

	ProcessMovement(ms, mv);
	memcpy(&LastViewAngles, &mv->m_vecViewAngles, sizeof(QAngle));
}

void FASTCALL BasePlayer::Detour_ProcessUsercmds(CBasePlayerPawn* player, CUserCmd* usercmd, int totalcmds, bool paused)
{
	for (int x = 0; x < totalcmds; x++)
	{
		CUserCmd* ptr = reinterpret_cast<CUserCmd*>(reinterpret_cast<char*>(usercmd) + (sizeof(CUserCmd) * x));
		int subtick_moves_count = ptr->base->subtick_moves.current_size;

		for (int i = 0; i < subtick_moves_count; i++)
		{
			float when = ptr->base->subtick_moves.rep->elements[i]->when;
			ptr->base->subtick_moves.rep->elements[i]->when = (when >= 0.5) ? 0.5 : 0.0;
		}
	}

	if (g_pMovementServices)
	{
		for (int y = 0; y < 4; y++)
		{
			g_pMovementServices->m_arrForceSubtickMoveWhen[y] = (GetServerGlobals()->tickcount+y) * 0.015625f - 0.5f * 0.015625f;
		}
	} 

	ProcessUsercmds(player, usercmd, totalcmds, paused);
}

bool DesubtickPlugin::Unload(char *error, size_t maxlen)
{	
	return true;
}

void DesubtickPlugin::AllPluginsLoaded()
{
}

bool DesubtickPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool DesubtickPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *DesubtickPlugin::GetLicense()
{
	return "GPLv3";
}

const char *DesubtickPlugin::GetVersion()
{
	return "1.0.0";
}

const char *DesubtickPlugin::GetDate()
{
	return __DATE__;
}

const char *DesubtickPlugin::GetLogTag()
{
	return "desubtick";
}

const char *DesubtickPlugin::GetAuthor()
{
	return "carnifex";
}

const char *DesubtickPlugin::GetDescription()
{
	return "Disables subticked movement";
}

const char *DesubtickPlugin::GetName()
{
	return "CS2 Desubtick";
}

const char *DesubtickPlugin::GetURL()
{
	return "";
}
