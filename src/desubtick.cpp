#include "desubtick.h"

#include "utils/addresses.h"

#include "tier0/memdbgon.h"
#include "utils/cdetour.h"

DesubtickPlugin g_DesubtickPlugin;
CUtlVector<CDetourBase*> g_vecDetours;

#define DECLARE_BASEPLAYER_DETOUR(name) DECLARE_DETOUR(name, BasePlayer::Detour_##name, &modules::server);
#define DECLARE_BASEPLAYER_EXTERN_DETOUR(name) extern CDetour<decltype(BasePlayer::Detour_##name)> name;

namespace BasePlayer
{
	void FASTCALL Detour_ProcessUsercmds(void* self, CUserCmd* usercmd, int totalcmds, bool paused);
}

DECLARE_BASEPLAYER_EXTERN_DETOUR(ProcessUsercmds);
DECLARE_BASEPLAYER_DETOUR(ProcessUsercmds);



PLUGIN_EXPOSE(DesubtickPlugin, g_DesubtickPlugin);

bool DesubtickPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	modules::server = new CModule(GAMEBIN, "server");
	
	INIT_DETOUR(ProcessUsercmds);

	return true;
}

void FASTCALL BasePlayer::Detour_ProcessUsercmds(void* self, CUserCmd* usercmd, int totalcmds, bool paused)
{
	for(int x = 0; x < totalcmds; x++)
	{
		CUserCmd* ptr = reinterpret_cast<CUserCmd*>(reinterpret_cast<char*>(usercmd) + (sizeof(CUserCmd)*x));
		int subtick_moves_count = ptr->base->subtick_moves.current_size;

		for(int i = 0; i < subtick_moves_count; i++)
		{
			ptr->base->subtick_moves.rep->elements[i]->when = 0.0f;
		}
	}
	ProcessUsercmds(self, usercmd, totalcmds, paused);
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
	return "0.0.0.1";
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