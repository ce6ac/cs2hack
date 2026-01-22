#include "game.h"
#include "memory.h"
#include "offsets.h"

/* client.dll stuff */

bool client::get_client_base() {
	this->base = mem.get_module_address("cs2.exe", "client.dll");
	if (this->base != 0) {
		return true;
	}
	return false;
}

float client::get_game_start() {
	uintptr_t game_rules;
	mem.read<uintptr_t>(this->base + offset.dwGameRules, game_rules);
	float game_start;
	mem.read<float>(game_rules + offset.m_flGameStartTime, game_start);
	return game_start;
}

uintptr_t client::get_local_pawn() {
	uintptr_t pawn;
	mem.read<uintptr_t>(this->base + offset.dwLocalPlayerPawn, pawn);
	return pawn;
}

uintptr_t client::get_entity_list() {
	uintptr_t entity_list;
	mem.read<uintptr_t>(this->base + offset.dwEntityList, entity_list);
	return entity_list;
}

bool client::use_button_down() {
	int use;
	mem.read<int>(this->base + offset.use_btn, use);
	if (use & (1 << 0))
		return true;
	return false;
}

bool client::attack_button_down() {
	int attack;
	mem.read<int>(this->base + offset.attack_btn, attack);
	if (attack & (1 << 0))
		return true;
	return false;
}

view_matrix_t client::get_view_matrix() {
	view_matrix_t view_matrix;
	mem.read<view_matrix_t>(this->base + offset.dwViewMatrix, view_matrix);
	return view_matrix;
}

/* entity */

uintptr_t entity::get_entity_controller(int player, uintptr_t entity_list) {
	uintptr_t entry_ptr;
	mem.read<uintptr_t>(entity_list + (0x8 * ((player & 0x7FFF) >> 9) + 0x10), entry_ptr);
	if (!entry_ptr) return 0;
	uintptr_t controller_ptr;
	mem.read<uintptr_t>(entry_ptr + 0x70 * (player & 0x1FF), controller_ptr);
	if (!controller_ptr) return 0;
	
	return controller_ptr;
}

uintptr_t entity::get_entity_pawn(uintptr_t controller, uintptr_t entity_list) {
	uintptr_t controller_pawn_ptr;
	mem.read<uintptr_t>(controller + offset.m_hPlayerPawn, controller_pawn_ptr);
	uintptr_t list_entry_ptr; 
	mem.read<uintptr_t>(entity_list + 0x8 * ((controller_pawn_ptr & 0x7FFF) >> 9) + 16, list_entry_ptr);
	if(!list_entry_ptr) return 0;
	uintptr_t player_pawn;
	mem.read<uintptr_t>(list_entry_ptr + 0x70 * (controller_pawn_ptr & 0x1FF), player_pawn);

	return player_pawn;
}

uintptr_t entity::get_entity_pawn_from_id(int entity, uintptr_t entity_list) {
	uintptr_t entry_ptr;
	mem.read<uintptr_t>(entity_list + (0x8 * (entity >> 9) + 0x10), entry_ptr);
	if (!entry_ptr) return 0;
	uintptr_t controller_ptr;
	mem.read<uintptr_t>(entry_ptr + 0x70 * (entity & 0x1FF), controller_ptr);
	if (!controller_ptr) return 0;

	return controller_ptr;
}

int entity::get_team(uintptr_t pawn) {
	int team;
	mem.read<int>(pawn + offset.m_iTeamNum, team);
	return team;
}

int entity::get_health(uintptr_t pawn) {
	int health;
	mem.read<int>(pawn + offset.m_iHealth, health);
	return health;
}

std::string entity::get_location(uintptr_t pawn) {
	char place[18];
	mem.read_array<char>(pawn + offset.m_szLastPlaceName, &place[0], 18);
	return place;
}

std::string entity::get_name(uintptr_t controller) {
	char name[128];
	mem.read_array<char>(controller + offset.m_iszPlayerName, &name[0], 128);
	return name;
}

uint64_t entity::get_steam64(uintptr_t controller) {
	uint64_t steam64;
	mem.read<uint64_t>(controller + offset.m_steamID, steam64);
	return steam64;
}

int entity::get_crosshair_id(uintptr_t local_pawn) {
	int id;
	mem.read<int>(local_pawn + offset.m_iIDEntIndex, id);
	return id;
}

int entity::get_shots_fired(uintptr_t local_pawn) {
	int shots;
	mem.read<int>(local_pawn + offset.m_iShotsFired, shots);
	return shots;
}

uintptr_t entity::get_bone_array_ptr(uintptr_t pawn) {
	uintptr_t m_pBoneArray = offset.m_modelState + 0x80;
	uintptr_t gamescene, bonearray_ptr;
	mem.read<uintptr_t>(pawn + offset.m_pGameSceneNode, gamescene); 
	mem.read<uintptr_t>(gamescene + m_pBoneArray, bonearray_ptr);
	
	return bonearray_ptr;
}

Vector3 entity::get_3d_bone_pos(uintptr_t bonearray, int bone) {
	Vector3 pos3d;
	mem.read<Vector3>(bonearray + bone * 32, pos3d);
	return pos3d;
}

uint32_t entity::is_spotted(uintptr_t pawn) {
	// m_bSpottedByMask = 0xC
	uint32_t spotted;
	mem.read<uint32_t>(pawn + offset.m_entitySpottedState + 0xC, spotted);
	return spotted;
}

bool entity::is_scoped(uintptr_t pawn) {
	bool b;
	mem.read<bool>(pawn + offset.m_bIsScoped, b);
	return b;
}

bool entity::is_rescuing(uintptr_t pawn) {
	bool b;
	mem.read<bool>(pawn + offset.m_bIsGrabbingHostage, b);
	return b;
}

bool entity::is_defusing(uintptr_t pawn) {
	bool b;
	mem.read<bool>(pawn + offset.m_bIsDefusing, b);
	return b;
}

bool entity::is_flashed(uintptr_t pawn) {
	float flash;
	mem.read<float>(pawn + offset.m_flFlashOverlayAlpha, flash);
	return flash;
}

Vector3 entity::get_pos(uintptr_t pawn) {
	Vector3 pos;
	mem.read<Vector3>(pawn + offset.m_vOldOrigin, pos);
	return pos;
}

uint16_t entity::get_weapon(uintptr_t pawn) {
	uintptr_t entity_weapon;
	mem.read<uintptr_t>(pawn + offset.m_pClippingWeapon, entity_weapon);
	uint16_t entity_item;
	mem.read<uint16_t>(entity_weapon + offset.m_AttributeManager + offset.m_Item + offset.m_iItemDefinitionIndex, entity_item);
	return entity_item;
}

/* weapon related */

int weapons::get_type(short id) {
	/*
	-1 = none, defined by game
	0 = knife / hands / nades, non firing weapons
	1 = pistol
	2 = rifle + autosniper
	3 = single shot sniper
	4 = shot guns
	5 = smg 
	6 = lmg
	*/
	switch (id) {
		case 1:
			return 1; // "deagle";
		case 2:
			return 1; // "dualies";
		case 3:
			return 1; // "fiveseven";
		case 4:
			return 1; // "glock";
		case 7:
			return 2; // "ak47";
		case 8:
			return 2; // "aug";
		case 9:
			return 3; // "awp";
		case 10:
			return 2; // "famas";
		case 11:
			return 2; // "g3sg1";
		case 13:
			return 2; // "galil";
		case 14:
			return 6; // "m249";
		case 16:
			return 2; // "m4a4";
		case 17:
			return 5; // "mac10";
		case 19:
			return 5; // "p90";
		case 23:
			return 5; // "mp5sd";
		case 24:
			return 5; // "ump45";
		case 25:
			return 4; // "xm1014";
		case 26:
			return 5; // "ppbizon";
		case 27:
			return 4; // "mag7";
		case 28:
			return 6; // "negev";
		case 29:
			return 4; // "sawedoff";
		case 30:
			return 1; // "tec9";
		case 31:
			return 7; // "taser";
		case 32:
			return 1; // "p2000";
		case 33:
			return 5; // "mp7";
		case 34:
			return 5; // "mp9";
		case 35:
			return 4; // "nova";
		case 36:
			return 1; // "p250";
		case 38:
			return 2; // "scar20";
		case 39:
			return 2; // "sg556";
		case 40:
			return 3; // "scout";
		case 60:
			return 2; // "m4a1s";
		case 61:
			return 1; // "usps";
		case 63:
			return 1; // "cz75";
		case 64:
			return 1; // "revolver";
		case -1:
			return -1; // "none";
		default:
			return 0;
	}
}

std::string weapons::get_weapon(short id) {
	switch (id) {
		case 1:
			return "deagle";
		case 2:
			return "dualies";
		case 3:
			return "fiveseven";
		case 4:
			return "glock";
		case 7:
			return "ak47";
		case 8:
			return "aug";
		case 9:
			return "awp";
		case 10:
			return "famas";
		case 11:
			return "g3sg1";
		case 13:
			return "galil";
		case 14:
			return "m249";
		case 16:
			return "m4a4";
		case 17:
			return "mac10";
		case 19:
			return "p90";
		case 23:
			return "mp5sd";
		case 24:
			return "ump45";
		case 25:
			return "xm1014";
		case 26:
			return "ppbizon";
		case 27:
			return "mag7";
		case 28:
			return "negev";
		case 29:
			return "sawedoff";
		case 30:
			return "tec9";
		case 31:
			return "taser";
		case 32:
			return "p2000";
		case 33:
			return "mp7";
		case 34:
			return "mp9";
		case 35:
			return "nova";
		case 36:
			return "p250";
		case 37:
			return "shield";
		case 38:
			return "scar20";
		case 39:
			return "sg556";
		case 40:
			return "scout";
		case 43:
			return "flash";
		case 44:
			return "grenade";
		case 45:
			return "smoke";
		case 46:
			return "molotov";
		case 47:
			return "decoy";
		case 48:
			return "incendiary";
		case 49:
			return "bomb";
		case 57:
			return "healthshot";
		case 60:
			return "m4a1s";
		case 61:
			return "usps";
		case 63:
			return "cz75";
		case 64:
			return "revolver";
		case 68:
			return "ta";
		case 69:
			return "fists";
		case 70:
			return "breach";
		case 72:
			return "tablet";
		case 74:
			return "melee";
		case 75:
			return "axe";
		case 76:
			return "hammer";
		case 78:
			return "spanner";
		case 80:
			return "knife";
		case 81:
			return "firebomb";
		case 82:
			return "diversion";
		case 83:
			return "frag";
		case 84:
			return "snowball";
		case 85:
			return "bumpmine";
		case -1:
			return "none";
		default:
			return "knife";
	}
}