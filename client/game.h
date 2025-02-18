#pragma once
#include <sys/types.h>
#include <chrono>
#include <string>
#include "utils/vector.h"

struct view_matrix_t {
	float matrix[4][4];
};

class offsets {
public:
	// offsets
	std::ptrdiff_t dwEntityList;
	std::ptrdiff_t dwGameRules;
	std::ptrdiff_t dwLocalPlayerPawn;
	std::ptrdiff_t dwViewMatrix;

	// +attack and +use
	std::ptrdiff_t attack_btn;
	std::ptrdiff_t use_btn;

	// entity
	std::ptrdiff_t m_iHealth;
	std::ptrdiff_t m_iTeamNum;
	std::ptrdiff_t m_szLastPlaceName;
	std::ptrdiff_t m_iszPlayerName;
	std::ptrdiff_t m_steamID;
	std::ptrdiff_t m_hPlayerPawn;
	std::ptrdiff_t m_flFlashOverlayAlpha;
	std::ptrdiff_t m_bIsScoped;
	std::ptrdiff_t m_bIsDefusing;
	std::ptrdiff_t m_bIsGrabbingHostage;
	std::ptrdiff_t m_iShotsFired;
	std::ptrdiff_t m_vOldOrigin;
	std::ptrdiff_t m_pClippingWeapon;
	std::ptrdiff_t m_iIDEntIndex;
	std::ptrdiff_t m_pGameSceneNode;

	// weapon stuff
	std::ptrdiff_t m_AttributeManager;
	std::ptrdiff_t m_Item;
	std::ptrdiff_t m_iItemDefinitionIndex;

	// gamerules
	std::ptrdiff_t m_flGameStartTime;
};

extern offsets offset;

class client {
public:
	uint64_t base = 0;
	bool get_client_base();
	uintptr_t get_local_pawn();
	uintptr_t get_entity_list();
	float get_game_start();
	bool use_button_down();
	bool attack_button_down();
	view_matrix_t get_view_matrix();
};

extern client cl;

class entity {
public:
	uintptr_t get_entity_controller(int player, uintptr_t entity_list);
	uintptr_t get_entity_pawn(uintptr_t controller, uintptr_t entity_list);
	uintptr_t get_entity_pawn_from_id(int entity, uintptr_t entity_list);
	int get_team(uintptr_t pawn);
	int get_health(uintptr_t pawn);
	Vector3 get_pos(uintptr_t pawn);
	std::string get_location(uintptr_t pawn);
	std::string get_name(uintptr_t controller);
	uint64_t get_steam64(uintptr_t controller);
	int get_crosshair_id(uintptr_t local_pawn);
	int get_shots_fired(uintptr_t local_pawn);
	uintptr_t get_bone_array_ptr(uintptr_t pawn);
	Vector3 get_3d_bone_pos(uintptr_t bonearray, int bone);
	bool is_defusing(uintptr_t pawn);
	bool is_scoped(uintptr_t pawn);
	bool is_rescuing(uintptr_t pawn);
	bool is_flashed(uintptr_t pawn);
	uint16_t get_weapon(uintptr_t pawn);
};

extern entity ent;

class weapons {
public:
	int get_type(short id);
	std::string get_weapon(short id);
};

extern weapons weapon;