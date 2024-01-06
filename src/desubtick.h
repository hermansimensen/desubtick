#pragma once

#ifdef _WIN64
#define NOMINMAX
#endif

#include "common.h"

class DesubtickPlugin : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	bool Pause(char *error, size_t maxlen);
	bool Unpause(char *error, size_t maxlen);
	void AllPluginsLoaded();
public:
	const char *GetAuthor();
	const char *GetName();
	const char *GetDescription();
	const char *GetURL();
	const char *GetLicense();
	const char *GetVersion();
	const char *GetDate();
	const char *GetLogTag();
};

class CMoveData
{
public:
	CMoveData() = default;
	CMoveData(const CMoveData& source) :
		moveDataFlags{ source.moveDataFlags },
		m_nPlayerHandle{ source.m_nPlayerHandle },
		m_vecAbsViewAngles{ source.m_vecAbsViewAngles },
		m_vecViewAngles{ source.m_vecViewAngles },
		m_vecLastMovementImpulses{ source.m_vecLastMovementImpulses },
		m_flForwardMove{ source.m_flForwardMove },
		m_flSideMove{ source.m_flSideMove },
		m_flUpMove{ source.m_flUpMove },
		m_flSubtickFraction{ source.m_flSubtickFraction },
		m_vecVelocity{ source.m_vecVelocity },
		m_vecAngles{ source.m_vecAngles },
		m_nMovementCmdsThisTick{ source.m_nMovementCmdsThisTick },
		m_bGameCodeMovedPlayer{ source.m_bGameCodeMovedPlayer },
		m_collisionNormal{ source.m_collisionNormal },
		m_groundNormal{ source.m_groundNormal },
		m_vecAbsOrigin{ source.m_vecAbsOrigin },
		m_nGameModeMovedPlayer{ source.m_nGameModeMovedPlayer },
		m_vecOldAngles{ source.m_vecOldAngles },
		m_flMaxSpeed{ source.m_flMaxSpeed },
		m_flClientMaxSpeed{ source.m_flClientMaxSpeed },
		m_flSubtickAccelSpeed{ source.m_flSubtickAccelSpeed },
		m_bJumpedThisTick{ source.m_bJumpedThisTick },
		m_bShouldApplyGravity{ source.m_bShouldApplyGravity },
		m_outWishVel{ source.m_outWishVel }
	{
	}
public:
	uint8_t moveDataFlags; // 0x0
	void* m_nPlayerHandle; // 0x4 don't know if this is actually a CHandle. <CBaseEntity> is a placeholder
	QAngle m_vecAbsViewAngles; // 0x8 unsure
	QAngle m_vecViewAngles; // 0x14
	Vector m_vecLastMovementImpulses;
	float m_flForwardMove; // 0x20
	float m_flSideMove; // 0x24
	float m_flUpMove; // 0x28
	float m_flSubtickFraction; // 0x38
	Vector m_vecVelocity; // 0x3c
	Vector m_vecAngles; // 0x48
	uint8_t padding1[4]; //0x54 unsure
	int m_nMovementCmdsThisTick; // 0x58 unsure, but it goes up if you spam lots of key in a tick
	uint8_t padding2[4]; // 0x5c
	uint8_t unknown3[8]; // 0x60 unsure, address of some sort, doesn't seem to change during gameplay
	uint8_t unknown4[8]; // 0x68 always 0, could be padding
	bool m_bGameCodeMovedPlayer; // 0x70
	uint8_t padding3[4]; // 0x74
	char padddd[0x18]; // 0x78
	Vector m_collisionNormal; // 0x90
	Vector m_groundNormal; // 0x9c unsure
	Vector m_vecAbsOrigin; // 0xa8
	uint8_t padding4[4]; // 0xb4 unsure
	bool m_nGameModeMovedPlayer; // 0xb8
	Vector m_vecOldAngles; // 0xbc
	float m_flMaxSpeed; // 0xc8
	float m_flClientMaxSpeed; // 0xcc
	float m_flSubtickAccelSpeed; // 0xd0 Related to ground acceleration subtick stuff with sv_stopspeed and friction
	bool m_bJumpedThisTick; // 0xd4 something to do with basevelocity and the tick the player jumps
	bool m_bShouldApplyGravity; // 0xd5
	Vector m_outWishVel; //0xd8
};

struct BasePB
{
	void *vftable;
	uint32_t has_bits;
	uint64_t cached_size;
};

template <typename T>
struct RepeatedPtrField
{
	struct Rep
	{
		int allocated_size;
		T *elements[(std::numeric_limits<int>::max() - 2 * sizeof(int)) / sizeof(void *)];
	};

	void *arena;
	int current_size;
	int total_size;
	Rep *rep;
};

struct CPlayer_MovementServices
{
	char pad[0x194];
	float m_arrForceSubtickMoveWhen[4];
};

struct CBasePlayerPawn
{
	char pad[0xA10];
	CPlayer_MovementServices* m_pMovementServices;
};

struct CInButtonStateReal
{
	void *table;
	uint64_t buttonstate1;
	uint64_t buttonstate2;
	uint64_t buttonstate3;
	char8 pad[0x30];
};

struct CSubtickMoveStep : BasePB
{
	uint64_t button;
	bool pressed;
	float when;
};

struct CBaseUserCmdPB : BasePB
{
	RepeatedPtrField<CSubtickMoveStep> subtick_moves;
};

class CUserCmd
{
public:
	uint8_t pad[0x20];
	uint32_t tick_count;
	uint32_t unknown;
	void *inputHistory;
	CBaseUserCmdPB *base;
	CInButtonStateReal buttonState;

#ifdef _WIN64
	void* windowsMoment;
#endif
};

extern DesubtickPlugin g_DesubtickPlugin;

internal void Hook_ClientCommand(CPlayerSlot slot, const CCommand& args);
internal void Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick);
internal float Hook_ProcessUsercmds_Pre(CPlayerSlot slot, bf_read *buf, int numcmds, bool ignore, bool paused);
internal float Hook_ProcessUsercmds_Post(CPlayerSlot slot, bf_read *buf, int numcmds, bool ignore, bool paused);
internal void Hook_CEntitySystem_Spawn_Post(int nCount, const EntitySpawnInfo_t* pInfo);
internal void Hook_CheckTransmit(CCheckTransmitInfo **pInfo, int, CBitVec<16384> &, const Entity2Networkable_t **pNetworkables, const uint16 *pEntityIndicies, int nEntities);
internal void Hook_ClientPutInServer(CPlayerSlot slot, char const *pszName, int type, uint64 xuid);