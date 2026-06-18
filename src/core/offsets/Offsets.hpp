namespace offsets
{
	// client.dll
	inline DWORD entityList;
	inline DWORD viewMatrix;
	inline DWORD localPlayerController;
	inline DWORD globalVars;
	inline DWORD plantedC4;
	inline DWORD localPlayerPawn;
	inline DWORD csgoInput;

	// engine2.dll
	inline DWORD buildNumber;

	namespace controller {
		constexpr std::ptrdiff_t m_iPing = 0x828; // uint32
		constexpr std::ptrdiff_t m_hPawn = 0x6BC; // CHandle<C_BasePlayerPawn>
		constexpr std::ptrdiff_t m_steamID = 0x780; // uint64
		constexpr std::ptrdiff_t m_iszPlayerName = 0x6F4; // char[128]
		constexpr std::ptrdiff_t m_bIsLocalPlayerController = 0x788; // bool
		constexpr std::ptrdiff_t m_pInGameMoneyServices = 0x808; // CCSPlayerController_InGameMoneyServices*
		constexpr std::ptrdiff_t m_iAccount = 0x40; // int32 - CCSPlayerController_InGameMoneyServices
		constexpr std::ptrdiff_t m_szClan = 0x7B0; // char[128] - CCSPlayerController
		constexpr std::ptrdiff_t m_iCompetitiveRanking = 0x864; // int32 - CCSPlayerController
	}

	namespace pawn {
		constexpr std::ptrdiff_t m_vOldOrigin = 0x1390; // Vector
		constexpr std::ptrdiff_t m_iHealth = 0x34C; // int32
		constexpr std::ptrdiff_t m_iTeamNum = 0x3EB; // uint8
		constexpr std::ptrdiff_t m_bIsScoped = 0x1C50; // bool
		constexpr std::ptrdiff_t m_ArmorValue = 0x1C7C; // int32
		constexpr std::ptrdiff_t m_bIsDefusing = 0x1C52; // bool
		constexpr std::ptrdiff_t m_vecAbsVelocity = 0x3FC; // Vector

		constexpr std::ptrdiff_t m_pGameSceneNode = 0x330; // CGameSceneNode*

		constexpr std::ptrdiff_t m_entitySpottedState = 0x1C38; // EntitySpottedState_t
		constexpr std::ptrdiff_t m_bSpottedByMask = 0xC; // uint32[2] - EntitySpottedState_t

		constexpr std::ptrdiff_t m_flFlashOverlayAlpha = 0x13F4; // float32 - C_CSPlayerPawnBase
			constexpr std::ptrdiff_t m_flFlashMaxAlpha = 0x13FC; // float32 - C_CSPlayerPawnBase
			constexpr std::ptrdiff_t m_flFlashDuration = 0x1400; // float32 - C_CSPlayerPawnBase

		constexpr std::ptrdiff_t m_pWeaponServices = 0x11E0; // CPlayer_WeaponServices*
		constexpr std::ptrdiff_t m_hActiveWeapon = 0x60; // CHandle<C_BasePlayerWeapon> - CPlayer_WeaponServices
		constexpr std::ptrdiff_t m_flNextAttack = 0xD0; // GameTime_t - CCSPlayer_WeaponServices (bolt/pump cycle delay)
		constexpr std::ptrdiff_t m_AttributeManager = 0x1180; // C_AttributeContainer - C_EconEntity (parent of C_BasePlayerWeapon)
		constexpr std::ptrdiff_t m_Item = 0x50; // C_EconItemView - C_AttributeContainer
		constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA; // uint16 - C_EconItemView
		constexpr std::ptrdiff_t m_iClip1 = 0x16D8; // int32 - C_BasePlayerWeapon
		constexpr std::ptrdiff_t m_bInReload = 0x17F4; // bool - C_CSWeaponBase

		constexpr std::ptrdiff_t m_zoomLevel = 0x17B8; // int32 - C_CSWeaponBase (0=unscoped, 1=zoom1, 2=zoom2)

			// C_CSWeaponBase
			constexpr std::ptrdiff_t m_nNextPrimaryAttackTick = 0x16C8; // GameTick_t
			constexpr std::ptrdiff_t m_fAccuracyPenalty = 0x17D0; // float32
			constexpr std::ptrdiff_t m_iRecoilIndex = 0x17DC; // int32
			constexpr std::ptrdiff_t m_nPostponeFireReadyTicks = 0x17EC; // GameTick_t - holds fire during bolt/pump animation
			constexpr std::ptrdiff_t m_flPostponeFireReadyFrac = 0x17F0; // float32 - fractional component of above
			constexpr std::ptrdiff_t m_flNextClientFireBulletTime = 0x1908; // float32 - client-side fire time gate

		constexpr std::ptrdiff_t m_pObserverServices = 0x11F8; // CPlayer_ObserverServices*

		constexpr std::ptrdiff_t m_fFlags = 0x3F8; // uint32 - FL_ONGROUND = (1 << 0)

	    constexpr std::ptrdiff_t m_pAimPunchServices = 0x1490; // CCSPlayer_AimPunchServices*
	    constexpr std::ptrdiff_t m_iShotsFired = 0x1C64; // int32

	    // AimPunchServices offsets (read pointer from m_pAimPunchServices, then offset from that)
	    namespace aimPunchServices {
	        constexpr std::ptrdiff_t m_predictableBaseAngle = 0x50; // QAngle - current aim punch (predictable component)
	    }

		constexpr std::ptrdiff_t m_vecViewOffset = 0x1548; // Vector - eye position offset

		// Smoke grenade opacity (C_SmokeGrenadeProjectile)
		constexpr std::ptrdiff_t m_nSmokeEffectTickBegin = 0x1960; // int32 - tick when smoke started
		constexpr std::ptrdiff_t m_bSmokeEffectSpawned = 0x1968; // bool
	}

	namespace smokeGrenade {
		constexpr std::ptrdiff_t m_smokeEffectAlpha = 0x1B0; // float - smoke overlay alpha factor in the smoke particle effect
	}

	namespace bomb {
		constexpr std::ptrdiff_t m_isPlanted = 0x8; // unk
		constexpr std::ptrdiff_t m_bC4Activated = 0x11A8; // bool
		constexpr std::ptrdiff_t m_nBombSite = 0x1164; // int32
		constexpr std::ptrdiff_t m_flC4Blow = 0x1190; // GameTime_t - detonation time
		constexpr std::ptrdiff_t m_bBeingDefused = 0x119C; // bool
		constexpr std::ptrdiff_t m_flDefuseLength = 0x11AC; // float32
		constexpr std::ptrdiff_t m_flDefuseCountDown = 0x11B0; // GameTime_t - defuse completion time
		constexpr std::ptrdiff_t m_bBombDefused = 0x11B4; // bool

		constexpr std::ptrdiff_t m_vecAbsOrigin = 0xC8; // VectorWS - CGameSceneNode
	}

	namespace input {
		constexpr std::ptrdiff_t viewAngles = 0x4; // QAngle offset from csgoInput pointer
		// csgoInput points to a CCSGOInput object; view angles are a Vec3 at +0x4
		// Read: p->read<Vec3_t>(p->read<uintptr_t>(client.base + offsets::csgoInput) + viewAngles)
	}

	namespace bone {
		constexpr std::ptrdiff_t m_modelState = 0x150; // CModelState
	}

	namespace observerServices {
		constexpr std::ptrdiff_t m_iObserverMode = 0x48;
		constexpr std::ptrdiff_t m_hObserverTarget = 0x4C;
	}

	namespace global {
		constexpr std::ptrdiff_t maxClients = 0x10;
		constexpr std::ptrdiff_t currentMapName = 0x180;
		constexpr std::ptrdiff_t currentTime = 0x2C;
	}

	namespace signatures
	{
		const std::string viewMatrix = "48 8D 0D ?? ?? ?? ?? 48 C1 E0 06";
		const std::string globalVars = "48 89 15 ?? ?? ?? ?? 48 89 42";
		const std::string entityList = "48 8B 0D ?? ?? ?? ?? 48 89 7C 24 ?? 8B FA C1 EB";
		const std::string localPlayerController = "48 8B 05 ?? ?? ?? ?? 41 89 BE";
		const std::string plantedC4 = "48 8b 15 ?? ?? ?? ?? 41 FF C0 48 8D 4C 24 ?? 44 89 05 ?? ?? ?? ??";
		const std::string weaponC4 =
			"48 89 05 ?? ?? ?? ?? "
			"F7 C1 ?? ?? ?? ?? "
			"74 ?? "
			"81 E1 ?? ?? ?? ?? "
			"89 0D ?? ?? ?? ?? "
			"8B 05 ?? ?? ?? ?? "
			"89 1D ?? ?? ?? ?? "
			"EB ?? "
			"48 8B 15 ?? ?? ?? ?? "
			"48 8B 5C 24 ?? "
			"FF C0 "
			"89 05 ?? ?? ?? ?? "
			"48 8B C6 48 89 34 EA 80 BE";

#if 1
		const std::string localPlayerPawn = "48 8D 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 83 EC ?? 8B 0D";

		const std::string csgoInput = "48 89 05 ?? ?? ?? ?? 0F 57 C0 0F 11 05";
		const std::string viewAngles = "F2 42 0F 10 84 28 ?? ?? ?? ??";
#endif

		const std::string buildNumber = "89 05 ?? ?? ?? ?? 48 8d 0d ?? ?? ?? ?? ff 15 ?? ?? ?? ?? 48 8b 0d";

	}
}
