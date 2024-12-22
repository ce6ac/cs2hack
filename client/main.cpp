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
#include "offsets.h"
#include "vector.h"
#include "json.hpp"
#include "weapons.h"

using json = nlohmann::json;

memory game;
communications comms;
uint64_t client_base = 0;

std::string app_url = "http://localhost:3000/receiver";

std::string sanitize_utf8(const std::string& input) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	try {
		std::wstring wide_str = converter.from_bytes(input);
		return converter.to_bytes(wide_str);
	} catch (const std::range_error&) {
		return "";
	}
}

static void run_info_esp() {
	json jsonArray = json::array();
	while (client_base) {
		std::this_thread::sleep_for(std::chrono::milliseconds(333));
		uintptr_t entity_list, local_pawn, local_player, game_rules;
		float game_start;
		int local_team;
		game.read<uintptr_t>(client_base + dwGameRules, game_rules);
		game.read<float>(game_rules + m_flGameStartTime, game_start);
		game.read<uintptr_t>(client_base + dwEntityList, entity_list);
		game.read<uintptr_t>(client_base + dwLocalPlayerPawn, local_pawn);
		game.read<int>(local_pawn + m_iTeamNum, local_team);
		Vector3 local_pos;
		game.read<Vector3>(local_pawn + m_vOldOrigin, local_pos);

		for (int i = 1; i < 64; i++)
		{
			uintptr_t ent_entry[1], entity_player, entity_pawn;
			uint32_t pawn;
			game.read<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 0x10, ent_entry[0]);
			if (!ent_entry[0])
				continue;
			game.read<uintptr_t>(ent_entry[0] + 0x78 * (i & 0x1FF), entity_player);
			if (!entity_player)
				continue;

			game.read<uint32_t>(entity_player + m_hPlayerPawn, pawn);
			game.read<uintptr_t>(entity_list + 0x8 * ((pawn & 0x7FFF) >> 9) + 0x10, ent_entry[1]);
			game.read<uintptr_t>(ent_entry[1] + 0x78 * (pawn & 0x1FF), entity_pawn);
			
			static char place[18];
			static char name[64];
			int entity_team, entity_health;
			uint64_t entity_steam;
			Vector3 entity_pos;
			float entity_flash;
			bool entity_def, entity_scoped, entity_hos, entity_sneak;
			game.read<int>(entity_pawn + m_iTeamNum, entity_team);
			game.read<int>(entity_pawn + m_iHealth, entity_health);
			game.read<Vector3>(entity_pawn + m_vOldOrigin, entity_pos);
			if (entity_pos.IsZero()) continue;
			game.read_array<char>(entity_pawn + m_szLastPlaceName, &place[0], 18);
			game.read_array<char>(entity_player + m_iszPlayerName, &name[0], 64);
			game.read<uint64_t>(entity_player + m_steamID, entity_steam);
			std::string entity_loc = place;
			std::string entity_name = name;

			game.read<bool>(entity_pawn + m_bIsScoped, entity_scoped);
			game.read<bool>(entity_pawn + m_bIsDefusing, entity_def);
			game.read<bool>(entity_pawn + m_bIsGrabbingHostage, entity_hos);
			game.read<float>(entity_pawn + m_flFlashOverlayAlpha, entity_flash);

			uintptr_t entity_weapon;
			game.read<uintptr_t>(entity_pawn + m_pClippingWeapon, entity_weapon);
			uint16_t entity_item;
			game.read<uint16_t>(entity_weapon + m_AttributeManager  + m_Item + m_iItemDefinitionIndex, entity_item);

			std::string entity_flags = "";
			if (entity_scoped) entity_flags = "scoped";
			if (entity_flash) entity_flags = "flashed";
			if (entity_def || entity_hos) entity_flags = "defusing";

			json entityJson;
			entityJson["team"] = entity_team;
			entityJson["health"] = entity_health;
	   	 	entityJson["pos"] = { entity_pos.x, entity_pos.y, entity_pos.z };
			entityJson["steam"] = sanitize_utf8(std::to_string(entity_steam));
			entityJson["loc"] = sanitize_utf8(entity_loc);
			entityJson["name"] = sanitize_utf8(entity_name);
			entityJson["flags"] = sanitize_utf8(entity_flags);
			entityJson["gun"] = sanitize_utf8(weapon(entity_item));

			jsonArray.push_back(entityJson);
		}	
		if (jsonArray.size() > 0 && game_start != 0.00f)
			comms.post_data(jsonArray, app_url);
		jsonArray.clear();
	}
}

bool get_offsets() {
	std::string offset_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.json");

	std::cout << std::hex;

	if (offset_json.empty())
		return false;

	try {
		auto parsedData = nlohmann::json::parse(offset_json);

		if (parsedData.contains("client.dll")) {
			std::cout << "global: parsing offsets.json" << std::endl;
			dwEntityList = parsedData["client.dll"]["dwEntityList"];
			dwGameRules = parsedData["client.dll"]["dwGameRules"];
			dwLocalPlayerPawn = parsedData["client.dll"]["dwLocalPlayerPawn"];
			std::cout << "parsed: dwEntityList 0x" << dwEntityList << std::endl;
			std::cout << "parsed: dwGameRules 0x" << dwGameRules << std::endl;
			std::cout << "parsed: dwLocalPlayerPawn 0x" << dwLocalPlayerPawn << std::endl;

			

		}
	} catch (const std::exception& e) {
		std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
	}

	std::string clientdll_json = comms.get_data("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.json");

	if (clientdll_json.empty())
		return false;

	try {
		auto parsedData = nlohmann::json::parse(clientdll_json);

		if (parsedData.contains("client.dll")) {
			std::cout << "global: parsing client_dll.json" << std::endl;
			// C_BaseEntity
			m_iHealth 				= parsedData["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"];
			m_iTeamNum 				= parsedData["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"];
			// C_CSPlayerPawn
			m_szLastPlaceName		= parsedData["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_szLastPlaceName"];
			m_bIsDefusing			= parsedData["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsDefusing"];
			m_bIsGrabbingHostage	= parsedData["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsGrabbingHostage"];
			m_bIsScoped				= parsedData["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsScoped"];
			// CBasePlayerController
			m_iszPlayerName 		= parsedData["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"];
			m_steamID 				= parsedData["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_steamID"];
			// CCSPlayerController
			m_hPlayerPawn 			= parsedData["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
			// C_CSPlayerPawnBase
			m_flFlashOverlayAlpha 	= parsedData["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_flFlashOverlayAlpha"];
			m_pClippingWeapon		= parsedData["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_pClippingWeapon"];
			// C_BasePlayerPawn
			m_vOldOrigin			= parsedData["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"];
			// weapon stuff too lazy
			m_AttributeManager		= parsedData["client.dll"]["classes"]["C_EconEntity"]["fields"]["m_AttributeManager"];
			m_Item					= parsedData["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"];
			m_iItemDefinitionIndex	= parsedData["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];

			std::cout << "parsed: m_iHealth 0x" << m_iHealth << std::endl;
			std::cout << "parsed: m_iTeamNum 0x" << m_iTeamNum << std::endl;
			std::cout << "parsed: m_szLastPlaceName 0x" << m_szLastPlaceName << std::endl;
			std::cout << "parsed: m_bIsDefusing 0x" << m_bIsDefusing << std::endl;
			std::cout << "parsed: m_bIsGrabbingHostage 0x" << m_bIsGrabbingHostage << std::endl;
			std::cout << "parsed: m_bIsScoped 0x" << m_bIsScoped << std::endl;
			std::cout << "parsed: m_iszPlayerName 0x" << m_iszPlayerName << std::endl;
			std::cout << "parsed: m_steamID 0x" << m_steamID << std::endl;
			std::cout << "parsed: m_hPlayerPawn 0x" << m_hPlayerPawn << std::endl;
			std::cout << "parsed: m_flFlashOverlayAlpha 0x" << m_flFlashOverlayAlpha << std::endl;
			std::cout << "parsed: m_pClippingWeapon 0x" << m_pClippingWeapon << std::endl;
			std::cout << "parsed: m_vOldOrigin 0x" << m_vOldOrigin << std::endl;
			std::cout << "parsed: m_AttributeManager 0x" << m_AttributeManager << std::endl;
			std::cout << "parsed: m_Item 0x" << m_Item << std::endl;
			std::cout << "parsed: m_iItemDefinitionIndex 0x" << m_iItemDefinitionIndex << std::endl;

		}
	} catch (const std::exception& e) {
		std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
	}

	return true;
}

int main(int argc, char *argv[]) {
	const char* cs2_proc = "cs2.exe";

	if (!get_offsets()) {
		std::cout << "global: could not get offsets, exiting..." << std::endl;
		return 0;
	}

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
	}

	while (!0) {
		if (game.get_proc_status() != process_status::FOUND_READY) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			game.open_proc(cs2_proc);
			if(game.get_proc_status() == process_status::FOUND_READY) {
				client_base = game.get_module_address(cs2_proc, "client.dll");
				if (client_base) {
					std::cout << "memory: found client.dll 0x" << std::hex << client_base << std::endl;
					std::this_thread::sleep_for(std::chrono::seconds(2));
					run_info_esp();
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}
