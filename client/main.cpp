#include <stdio.h>
#include <sys/types.h>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include <codecvt>
#include <locale>
#include "comms.h"
#include "memory.h"
#include "game.h"
#include "utils/vector.h"
#include "utils/utils.h"
#include "include/qmp.h"

using json = nlohmann::json;

memory mem;
communications comms;
client cl;
entity ent;
weapons wpn;
offsets offset;
qemu::qmp qmp;

std::string app_url = "http://localhost:3000";

struct config {
	// general
	int port = 6448;
	int refresh = 250;

	// aim
	int fov = 20;
	int shots = 2;
	int smooth = 4;

	// trigger
	int delay = 33;
	int cooldown = 120;

	// global
	bool team = false;

	// site conifg
	std::string key = "l33ts3cr3t";
	std::string ep = "";
}; static config cfg = {};

static void run_info_esp() {
	json jsonArray = json::array();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(cfg.refresh));
		uintptr_t entity_list = cl.get_entity_list();
		uintptr_t local_pawn = cl.get_local_pawn();

		int local_team = ent.get_team(cl.get_local_pawn());
		int local_health = ent.get_health(local_pawn);
		Vector3 local_pos = ent.get_pos(local_pawn);

		for (int i = 1; i < 64; i++)
		{	
			uintptr_t entity_controller = ent.get_entity_controller(i, entity_list);
			if (!entity_controller) 
				continue;

			uintptr_t entity_pawn = ent.get_entity_pawn(entity_controller, entity_list);
			if (!entity_pawn)
				continue;

			Vector3 entity_pos = ent.get_pos(entity_pawn);
			int entity_team = ent.get_team(entity_pawn);
			int entity_health = ent.get_health(entity_pawn);

			if (ent.get_pos(entity_pawn).IsZero()) 
				continue;

			std::string entity_flags = "";
			if (ent.is_scoped(entity_pawn)) entity_flags = "scoped";
			if (ent.is_flashed(entity_pawn)) entity_flags = "flashed";
			if (ent.is_defusing(entity_pawn)) entity_flags = "defusing";
			if (ent.is_rescuing(entity_pawn)) entity_flags = "rescuing";

			json entityJson;
			entityJson["team"] = entity_team;
			entityJson["health"] = entity_health;
			entityJson["pos"] = { entity_pos.x, entity_pos.y, entity_pos.z };
			entityJson["steam"] = sanitize_utf8(std::to_string(ent.get_steam64(entity_controller)));
			entityJson["loc"] = sanitize_utf8(ent.get_location(entity_pawn));
			entityJson["name"] = sanitize_utf8(ent.get_name(entity_controller));
			entityJson["flags"] = sanitize_utf8(entity_flags);
			entityJson["gun"] = sanitize_utf8(wpn.get_weapon(ent.get_weapon(entity_pawn)));

			jsonArray.push_back(entityJson);
		}

		json hostJson, postJson;
		hostJson["team"] = local_team;
		hostJson["health"] = local_health;
		hostJson["pos"] = { local_pos.x, local_pos.y, local_pos.z };
		hostJson["key"] = cfg.key;
		hostJson["endpoint"] = cfg.ep;

		postJson["host"] = hostJson;
		postJson["entities"] = jsonArray;
		if (jsonArray.size() > 0 && cl.get_game_start() != 0.00f)
			std::cout << "webapp: " << comms.post_data(postJson, app_url + "/receiver") << std::endl;
		jsonArray.clear();
		postJson.clear();
	}
}

static void run_aim_trigger() {
	Vector3 center = screen_size;
	center.x /= 2;
	center.y /= 2;

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		uintptr_t local_pawn = cl.get_local_pawn();
		if(!local_pawn) 
			continue;

		int local_health = ent.get_team(local_pawn);
		if (!local_health) 
			continue;

		int entity_id = ent.get_crosshair_id(local_pawn);
		int local_team = ent.get_team(local_pawn);

		short weapon_type = wpn.get_type(ent.get_weapon(local_pawn));
		if (weapon_type < 1)
			continue;

		// triggerbot
		if (cl.use_button_down() && entity_id) {
			uintptr_t entity_list = cl.get_entity_list();
			uintptr_t entity_pawn = ent.get_entity_pawn_from_id(entity_id, entity_list);
			int entity_team = ent.get_team(entity_pawn);
			int entity_health = ent.get_health(entity_pawn);
			
			if (!cfg.team && local_team == entity_team)
				continue;

			if (entity_health > 0 && (entity_team == 2 || entity_team == 3)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(random_value(cfg.delay, cfg.delay + 20)));
				qmp.mouse_down();
				std::this_thread::sleep_for(std::chrono::milliseconds(random_value(25, 35)));
				qmp.mouse_up();
				std::this_thread::sleep_for(std::chrono::milliseconds(random_value(cfg.cooldown, cfg.cooldown + 40)));
			}
		} else { // aimbot
			if (!cl.attack_button_down())
				continue;

			if (ent.get_shots_fired(local_pawn) > cfg.shots)
				continue;

			uintptr_t entity_list = cl.get_entity_list();
			if (!entity_list) 
				continue;

			view_matrix_t view_matrix = cl.get_view_matrix();

			float closest_dist = 99999999999999.f;
			Vector3 closest_point;

			for (int i = 1; i <= 64; i++) {
				uintptr_t entity_controller = ent.get_entity_controller(i, entity_list);
				if (!entity_controller) 
					continue;
				uintptr_t entity_pawn = ent.get_entity_pawn(entity_controller, entity_list);
				if (!entity_pawn || entity_pawn == local_pawn)
					continue;

				int entity_health = ent.get_health(entity_pawn);
				int entity_team = ent.get_team(entity_pawn);

				if (!cfg.team && entity_team == local_team) 
					continue;
				if (!(entity_health > 0)) 
					continue;

				int bones[] = { 3, 4, 5, 6 };
				uintptr_t bonearray_ptr = ent.get_bone_array_ptr(entity_pawn);

				for (int j = 0; j < sizeof(bones); j++) {
					Vector3 pos2d, pos3d = ent.get_3d_bone_pos(bonearray_ptr, bones[j]);
					if(!world_to_screen(pos3d, pos2d, view_matrix))
						break;
						
					float distance = sqrt((pos2d.x - center.x) * (pos2d.x - center.x) 
										+ (pos2d.y - center.y) * (pos2d.y - center.y));
					if (closest_dist > distance) {
						closest_dist = distance;
						closest_point = pos2d;
					}
				}
			}

			if (closest_dist > cfg.fov)
				continue;

			int random = random_value(-1, 1);
			closest_point.x -= center.x + random;
			closest_point.y -= center.y + (random * -1);

			if (cfg.smooth) {
				closest_point.x /= (cfg.smooth + 2 + random);
				closest_point.y /= (cfg.smooth + 2 - random);
			}
			
			qmp.move_mouse(closest_point.x, closest_point.y);
		}
	}
}

bool get_offsets() {
	std::string offset_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.json");

	std::cout << std::hex;

	if (offset_json.empty())
		return false;

	try {
		auto parsed_data = nlohmann::json::parse(offset_json);

		if (parsed_data.contains("client.dll")) {
			std::cout << "global: parsing offsets.json" << std::endl;
			offset.dwEntityList = parsed_data["client.dll"]["dwEntityList"];
			offset.dwGameRules = parsed_data["client.dll"]["dwGameRules"];
			offset.dwLocalPlayerPawn = parsed_data["client.dll"]["dwLocalPlayerPawn"];
			offset.dwViewMatrix = parsed_data["client.dll"]["dwViewMatrix"];
			std::cout << "parsed: dwEntityList 0x" << offset.dwEntityList << std::endl;
			std::cout << "parsed: dwGameRules 0x" << offset.dwGameRules << std::endl;
			std::cout << "parsed: dwLocalPlayerPawn 0x" << offset.dwLocalPlayerPawn << std::endl;
			std::cout << "parsed: dwViewMatrix 0x" << offset.dwViewMatrix << std::endl;
		} else {
			return false;
		}
	} catch (const std::exception& e) {
		std::cerr << "parsed: " << "error " << e.what() << std::endl;
	}

	std::string clientdll_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.json");

	if (clientdll_json.empty())
		return false;

	try {
		auto parsed_data = nlohmann::json::parse(clientdll_json);

		if (parsed_data.contains("client.dll")) {
			std::cout << "global: parsing client_dll.json" << std::endl;
			// C_BaseEntity
			offset.m_iHealth 				= parsed_data["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"];
			offset.m_iTeamNum 				= parsed_data["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"];
			offset.m_pGameSceneNode 		= parsed_data["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_pGameSceneNode"];
			// C_CSPlayerPawn
			offset.m_szLastPlaceName		= parsed_data["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_szLastPlaceName"];
			offset.m_bIsDefusing			= parsed_data["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsDefusing"];
			offset.m_bIsGrabbingHostage		= parsed_data["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsGrabbingHostage"];
			offset.m_bIsScoped				= parsed_data["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsScoped"];
			offset.m_iShotsFired			= parsed_data["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iShotsFired"];
			// CBasePlayerController
			offset.m_iszPlayerName 			= parsed_data["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"];
			offset.m_steamID 				= parsed_data["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_steamID"];
			// CCSPlayerController
			offset.m_hPlayerPawn 			= parsed_data["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
			// C_CSPlayerPawnBase
			offset.m_flFlashOverlayAlpha 	= parsed_data["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_flFlashOverlayAlpha"];
			offset.m_pClippingWeapon		= parsed_data["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_pClippingWeapon"];
			offset.m_iIDEntIndex			= parsed_data["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_iIDEntIndex"];
			// C_BasePlayerPawn
			offset.m_vOldOrigin				= parsed_data["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"];
			// weapon stuff too lazy
			offset.m_AttributeManager		= parsed_data["client.dll"]["classes"]["C_EconEntity"]["fields"]["m_AttributeManager"];
			offset.m_Item					= parsed_data["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"];
			offset.m_iItemDefinitionIndex	= parsed_data["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];

			std::cout << "parsed: m_iHealth 0x" << offset.m_iHealth << std::endl;
			std::cout << "parsed: m_iTeamNum 0x" << offset.m_iTeamNum << std::endl;
			std::cout << "parsed: m_pGameSceneNode 0x" << offset.m_pGameSceneNode << std::endl;
			std::cout << "parsed: m_szLastPlaceName 0x" << offset.m_szLastPlaceName << std::endl;
			std::cout << "parsed: m_bIsDefusing 0x" << offset.m_bIsDefusing << std::endl;
			std::cout << "parsed: m_bIsGrabbingHostage 0x" << offset.m_bIsGrabbingHostage << std::endl;
			std::cout << "parsed: m_bIsScoped 0x" << offset.m_bIsScoped << std::endl;
			std::cout << "parsed: m_iShotsFired 0x" << offset.m_iShotsFired << std::endl;
			std::cout << "parsed: m_iszPlayerName 0x" << offset.m_iszPlayerName << std::endl;
			std::cout << "parsed: m_steamID 0x" << offset.m_steamID << std::endl;
			std::cout << "parsed: m_hPlayerPawn 0x" << offset.m_hPlayerPawn << std::endl;
			std::cout << "parsed: m_flFlashOverlayAlpha 0x" << offset.m_flFlashOverlayAlpha << std::endl;
			std::cout << "parsed: m_pClippingWeapon 0x" << offset.m_pClippingWeapon << std::endl;
			std::cout << "parsed: m_iIDEntIndex 0x" << offset.m_iIDEntIndex << std::endl;
			std::cout << "parsed: m_vOldOrigin 0x" << offset.m_vOldOrigin << std::endl;
			std::cout << "parsed: m_AttributeManager 0x" << offset.m_AttributeManager << std::endl;
			std::cout << "parsed: m_Item 0x" << offset.m_Item << std::endl;
			std::cout << "parsed: m_iItemDefinitionIndex 0x" << offset.m_iItemDefinitionIndex << std::endl;
		} else {
			return false;
		}
	} catch (const std::exception& e) {
		std::cerr << "parsed: " << "error " << e.what() << std::endl;
	}

	std::string buttons_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/buttons.json");

	if (buttons_json.empty())
		return false;
	try {
		auto parsed_data = nlohmann::json::parse(buttons_json);

		if (parsed_data.contains("client.dll")) {
			std::cout << "global: parsing buttons.json" << std::endl;
			offset.attack_btn = parsed_data["client.dll"]["attack"];
			offset.use_btn = parsed_data["client.dll"]["use"];
			std::cout << "parsed: attack button 0x" << offset.attack_btn << std::endl;
			std::cout << "parsed: use button 0x" << offset.use_btn << std::endl;
		} else {
			return false;
		}
	} catch (const std::exception& e) {
		std::cerr << "parsed: " << "error " << e.what() << std::endl;
	}

	return true;
}

void read_param_config(int argc, char *argv[]) {
	std::cout << std::dec;
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "-url") == 0) {
			if (i + 1 < argc) {
				app_url = argv[i + 1];
				std::cout << "config: custom url set (" << app_url << ")" << std::endl;
			}
			else {
				std::cout << "config: no url specified, running with localhost" << std::endl;
			}
		}
		else if (strcmp(argv[i], "-ep") == 0) {
			if (i + 1 < argc) {
				cfg.ep = argv[i + 1];
				std::cout << "config: valid endpoint set to /" << cfg.ep << std::endl;
			}
		}
		else if (strcmp(argv[i], "-key") == 0) {
			if (i + 1 < argc) {
				cfg.key = argv[i + 1];
				std::cout << "config: found post key" << std::endl;
				json key;
				key["key"] = cfg.key;
				std::cout << "webapp: " << comms.post_data(key, app_url + "/testkey") << std::endl;
			}
		}
		else if (strcmp(argv[i], "-port") == 0) {
			if (i + 1 < argc) {
				cfg.port = strtol(argv[i + 1], NULL, 10);
				std::cout << "config: using qmp port " << cfg.port << std::endl;
			}
		}
		else if (strcmp(argv[i], "-refresh") == 0) {
			if (i + 1 < argc) {
				cfg.refresh = strtol(argv[i + 1], NULL, 10);
				std::cout << "config: refresh set to " << cfg.refresh << std::endl;
			}
		}
		else if (strcmp(argv[i], "-team") == 0) {
			std::cout << "config: including teammates" << std::endl;
			cfg.team = true;
		}
		else if (strcmp(argv[i], "-fov") == 0) {
			if (i + 1 < argc) {
				cfg.fov = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-smooth") == 0) {
			if (i + 1 < argc) {
				cfg.smooth = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-shots") == 0) {
			if (i + 1 < argc) {
				cfg.shots = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-delay") == 0) {
			if (i + 1 < argc) {
				cfg.delay = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-cooldown") == 0) {
			if (i + 1 < argc) {
				cfg.cooldown = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-w") == 0) {
			if (i + 1 < argc) {
				screen_size.x = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-h") == 0) {
			if (i + 1 < argc) {
				screen_size.y = strtol(argv[i + 1], NULL, 10);
			}
		}
	}
	std::cout << "config: aim fov " << cfg.fov << std::endl;
	std::cout << "config: aim smooth " << cfg.smooth << std::endl;
	std::cout << "config: aim shots " << cfg.shots << std::endl;
	std::cout << "config: trigger delay " << cfg.delay << std::endl;
	std::cout << "config: trigger cooldown " << cfg.cooldown << std::endl;
	std::cout << "config: resolution width " << screen_size.x << std::endl;
	std::cout << "config: resolution height " << screen_size.y << std::endl;
}

int main(int argc, char *argv[]) {
	if (!get_offsets()) {
		std::cout << "global: could not get offsets, exiting..." << std::endl;
		return 0;
	}

	read_param_config(argc, argv);

	while (!0) {
		if (mem.get_proc_status() != process_status::FOUND_READY) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			mem.open_proc("cs2.exe");
			if(mem.get_proc_status() == process_status::FOUND_READY) {
				if (cl.get_client_base()) {
					std::cout << "memory: found client.dll 0x" << std::hex << cl.base << std::endl;
					std::this_thread::sleep_for(std::chrono::seconds(2));
					if (qmp.setup("127.0.0.1", cfg.port) && qmp.enable_cmds()) {
						std::cout << "global: qmp connection established" << std::endl;
						std::thread aim_thread;
						aim_thread = std::thread(run_aim_trigger);
						aim_thread.detach();
					} else {
						std::cout << "global: could not connect to qmp, aim/trigger unavailable" << std::endl;
					}
					if (!cfg.ep.empty()) {
						std::cout << "global: info page should be " << app_url << "/" << cfg.ep << std::endl;
					}
					run_info_esp();
					qmp.disconnect();
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}