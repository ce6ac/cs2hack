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
	bool web = true;
	int port = 6448;
	int refresh = 250;

	// aim
	float fov = 15.f;
	int shots = 2;
	float smooth = 4.f;
	bool spotted = false;

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
	json json_array = json::array();
	while (true) {
		if (!cfg.web)
			continue;
		try {
			std::cout << "webapp: refreshing data" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(cfg.refresh));
			uintptr_t entity_list = cl.get_entity_list();
			uintptr_t local_pawn = cl.get_local_pawn();

			int local_team = ent.get_team(cl.get_local_pawn());
			int local_health = ent.get_health(local_pawn);
			int local_index;
			Vector3 local_pos = ent.get_pos(local_pawn);

			for (int i = 1; i < 64; i++)
			{	
				uintptr_t entity_controller = ent.get_entity_controller(i, entity_list);
				if (!entity_controller) 
					continue;

				uintptr_t entity_pawn = ent.get_entity_pawn(entity_controller, entity_list);
				if (!entity_pawn)
					continue;

				if (entity_pawn == local_pawn)
					local_index = i;

				Vector3 entity_pos = ent.get_pos(entity_pawn);
				int entity_team = ent.get_team(entity_pawn);
				int entity_health = ent.get_health(entity_pawn);

				if (ent.get_pos(entity_pawn).IsZero()) 
					continue;

				std::string entity_flags = "";
				//if (ent.is_spotted(entity_pawn) & (uint32_t(1) << (local_index - 1))) entity_flags = "spotted";
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

				json_array.push_back(entityJson);
			}

			json host_json, post_json;
			host_json["team"] = local_team;
			host_json["health"] = local_health;
			host_json["pos"] = { local_pos.x, local_pos.y, local_pos.z };
			host_json["key"] = cfg.key;
			host_json["endpoint"] = cfg.ep;

			post_json["host"] = host_json;
			post_json["entities"] = json_array;
			if (json_array.size() > 0 && host_json.size() > 0 && post_json.size() > 0) {
				std::cout << "webapp: " << comms.post_data(post_json, app_url + "/receiver") << std::endl;
			}
			json_array.clear();
			host_json.clear();
			post_json.clear();
		} catch (const std::exception& e) {
			std::cerr << "webapp: exception - " << e.what() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
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

		int local_health = ent.get_health(local_pawn);
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
			Vector3 entity_pos = ent.get_pos(entity_pawn);
			if (entity_pos.IsZero())
				continue;
			
			if (!cfg.team && local_team == entity_team)
				continue;

			if (entity_health > 0 && (entity_team == 2 || entity_team == 3)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(random_int(cfg.delay, cfg.delay + 20)));
				qmp.mouse_down();
				std::this_thread::sleep_for(std::chrono::milliseconds(random_int(25, 35)));
				qmp.mouse_up();
				std::this_thread::sleep_for(std::chrono::milliseconds(random_int(cfg.cooldown, cfg.cooldown + 40)));
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
			int local_index;

			for (int i = 1; i <= 64; i++) {
				uintptr_t entity_controller = ent.get_entity_controller(i, entity_list);
				if (!entity_controller) 
					continue;
				uintptr_t entity_pawn = ent.get_entity_pawn(entity_controller, entity_list);
				if (!entity_pawn)
					continue;
				if (entity_pawn == local_pawn)
					local_index = i;
				if (cfg.spotted && !(ent.is_spotted(entity_pawn) & (uint32_t(1) << (local_index - 1))))
					continue;

				int entity_health = ent.get_health(entity_pawn);
				int entity_team = ent.get_team(entity_pawn);
				Vector3 entity_pos = ent.get_pos(entity_pawn);
				if (entity_pos.IsZero())
					continue;

				if (!cfg.team && entity_team == local_team) 
					continue;
				if (!(entity_health > 0)) 
					continue;

				std::vector<int> bones = { 3, 4, 5, 6 };

				if (weapon_type == 1) {
					bones = { 6 };
				}

				uintptr_t bonearray_ptr = ent.get_bone_array_ptr(entity_pawn);

				for (int bone : bones) {
					Vector3 pos2d, pos3d = ent.get_3d_bone_pos(bonearray_ptr, bone);
				
					if (pos3d.IsZero())
						continue; // skip invalid bone
				
					if (!world_to_screen(pos3d, pos2d, view_matrix))
						continue; // skip if offscreen
				
					float distance = sqrt(
						(pos2d.x - center.x) * (pos2d.x - center.x) +
						(pos2d.y - center.y) * (pos2d.y - center.y)
					);
				
					if (closest_dist > distance) {
						closest_dist = distance;
						closest_point = pos2d;
					}
				}

			}

			if (closest_dist > cfg.fov)
				continue;

			closest_point.x -= center.x + random_float(-0.5f, 0.5f);
			closest_point.y -= center.y + (-random_float(-0.5f, 0.5f));

			if (cfg.smooth) {
				closest_point.x /= (weapon_type == 1 ? 3.5f : 0.f) + (cfg.smooth + 1.1f) + (-random_float(-0.5f, 0.5f));
				closest_point.y /= (weapon_type == 1 ? 3.5f : 0.f) + (cfg.smooth + 1.1f) + random_float(-0.5f, 0.5f);
			}
			
			qmp.move_mouse(closest_point.x, closest_point.y);
		}
	}
}

#define PARSE_OFFSET(target, jpath) \
	try { \
		if (jpath.is_number_integer()) { \
			target = jpath.get<int>(); \
			std::cout << "parsed: " #target " 0x" << std::hex << target << std::endl; \
		} else { \
			std::cerr << "warning: " #target " is not an integer (null or wrong type)" << std::endl; \
		} \
	} catch (const std::exception& e) { \
		std::cerr << "error parsing " #target ": " << e.what() << std::endl; \
	}

bool get_offsets() {
	std::string offset_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.json");
	if (offset_json.empty()) return false;

	try {
		auto parsed = json::parse(offset_json);
		if (parsed.contains("client.dll")) {
			std::cout << "global: parsing offsets.json" << std::endl;

			PARSE_OFFSET(offset.dwEntityList, parsed["client.dll"]["dwEntityList"]);
			PARSE_OFFSET(offset.dwGameRules, parsed["client.dll"]["dwGameRules"]);
			PARSE_OFFSET(offset.dwLocalPlayerPawn, parsed["client.dll"]["dwLocalPlayerPawn"]);
			PARSE_OFFSET(offset.dwViewMatrix, parsed["client.dll"]["dwViewMatrix"]);
		} else return false;
	} catch (const std::exception& e) {
		std::cerr << "offsets.json error: " << e.what() << std::endl;
	}

	std::string clientdll_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.json");
	if (clientdll_json.empty()) return false;

	try {
		auto parsed = json::parse(clientdll_json);
		if (parsed.contains("client.dll")) {
			std::cout << "global: parsing client_dll.json" << std::endl;

			// C_BaseEntity
			PARSE_OFFSET(offset.m_iHealth, parsed["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"]);
			PARSE_OFFSET(offset.m_iTeamNum, parsed["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"]);
			PARSE_OFFSET(offset.m_pGameSceneNode, parsed["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_pGameSceneNode"]);

			// C_CSPlayerPawn
			PARSE_OFFSET(offset.m_szLastPlaceName, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_szLastPlaceName"]);
			PARSE_OFFSET(offset.m_entitySpottedState, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_entitySpottedState"]);
			PARSE_OFFSET(offset.m_bIsScoped, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsScoped"]);
			PARSE_OFFSET(offset.m_bIsDefusing, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsDefusing"]);
			PARSE_OFFSET(offset.m_bIsGrabbingHostage, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsGrabbingHostage"]);
			PARSE_OFFSET(offset.m_iShotsFired, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iShotsFired"]);
			PARSE_OFFSET(offset.m_pClippingWeapon, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pClippingWeapon"]);
			PARSE_OFFSET(offset.m_iIDEntIndex, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iIDEntIndex"]);

			// CBasePlayerController
			PARSE_OFFSET(offset.m_iszPlayerName, parsed["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"]);
			PARSE_OFFSET(offset.m_steamID, parsed["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_steamID"]);

			// CCSPlayerController
			PARSE_OFFSET(offset.m_hPlayerPawn, parsed["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"]);

			// C_CSPlayerPawnBase
			PARSE_OFFSET(offset.m_flFlashOverlayAlpha, parsed["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_flFlashOverlayAlpha"]);

			// C_BasePlayerPawn
			PARSE_OFFSET(offset.m_vOldOrigin, parsed["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"]);

			// Weapon-related
			PARSE_OFFSET(offset.m_AttributeManager, parsed["client.dll"]["classes"]["C_EconEntity"]["fields"]["m_AttributeManager"]);
			PARSE_OFFSET(offset.m_Item,	parsed["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"]);
			PARSE_OFFSET(offset.m_iItemDefinitionIndex, parsed["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"]);

		} else return false;
	} catch (const std::exception& e) {
		std::cerr << "client_dll.json error: " << e.what() << std::endl;
	}

	std::string buttons_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/buttons.json");
	if (buttons_json.empty()) return false;

	try {
		auto parsed = json::parse(buttons_json);
		if (parsed.contains("client.dll")) {
			std::cout << "global: parsing buttons.json" << std::endl;

			PARSE_OFFSET(offset.attack_btn, parsed["client.dll"]["attack"]);
			PARSE_OFFSET(offset.use_btn,	parsed["client.dll"]["use"]);
		} else return false;
	} catch (const std::exception& e) {
		std::cerr << "buttons.json error: " << e.what() << std::endl;
	}

	return true;
}


void read_param_config(int argc, char *argv[]) {
	std::cout << std::dec;
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "-noweb") == 0) {
			cfg.web = false;
			std::cout << "config: running without webapp" << std::endl;
		}
		else if (strcmp(argv[i], "-url") == 0 && cfg.web) {
			if (i + 1 < argc) {
				app_url = argv[i + 1];
				std::cout << "config: custom url set (" << app_url << ")" << std::endl;
			}
			else {
				std::cout << "config: no url specified, running with localhost" << std::endl;
			}
		}
		else if (strcmp(argv[i], "-ep") == 0 && cfg.web) {
			if (i + 1 < argc) {
				cfg.ep = argv[i + 1];
				std::cout << "config: valid endpoint set to /" << cfg.ep << std::endl;
			}
		}
		else if (strcmp(argv[i], "-key") == 0 && cfg.web) {
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
		else if (strcmp(argv[i], "-refresh") == 0 && cfg.web) {
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
				cfg.fov = strtof(argv[i + 1], NULL);
			}
		}
		else if (strcmp(argv[i], "-smooth") == 0) {
			if (i + 1 < argc) {
				cfg.smooth = strtof(argv[i + 1], NULL);
			}
		}
		else if (strcmp(argv[i], "-shots") == 0) {
			if (i + 1 < argc) {
				cfg.shots = strtol(argv[i + 1], NULL, 10);
			}
		}
		else if (strcmp(argv[i], "-vischeck") == 0) {
			cfg.spotted = true;
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
	std::cout << "config: resolution width " << screen_size.x << std::endl;
	std::cout << "config: resolution height " << screen_size.y << std::endl;
	std::cout << "config: aim fov " << cfg.fov << std::endl;
	std::cout << "config: aim smooth " << cfg.smooth << std::endl;
	std::cout << "config: aim shots " << cfg.shots << std::endl;
	std::cout << "config: aim vischeck " << cfg.spotted << std::endl;
	std::cout << "config: trigger delay " << cfg.delay << std::endl;
	std::cout << "config: trigger cooldown " << cfg.cooldown << std::endl;
}

int main(int argc, char *argv[]) {
	if (!get_offsets()) {
		std::cout << "global: could not get offsets, exiting..." << std::endl;
		return 0;
	}

	read_param_config(argc, argv);

	while (true) {
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
					if (!cfg.ep.empty() && cfg.web) {
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