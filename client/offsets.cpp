#include "offsets.h"
#include "comms.h"
#include <iostream>

using json = nlohmann::json;

void offsets::parse_offset(std::ptrdiff_t& target, const json& jpath, std::string name) {
    try {
        if (jpath.is_number_integer()) {
            target = jpath.get<int>();
            std::cout << "parsed: " << name << " 0x"
                      << std::hex << target << std::dec << std::endl;
        } 
    } catch (const std::exception& e) {
        std::cerr << "error parsing " << name << ": "
                  << e.what() << std::endl;
    }
}

bool offsets::get_offsets(communications& comms) {
	std::string offset_json = comms.get_data(output_url + "/offsets.json");
	if (offset_json.empty()) return false;

	try {
		auto parsed = json::parse(offset_json);
		if (parsed.contains("client.dll")) {
			std::cout << "global: parsing offsets.json" << std::endl;

			parse_offset(offset.dwEntityList, parsed["client.dll"]["dwEntityList"], "entitylist");
			parse_offset(offset.dwGameRules, parsed["client.dll"]["dwGameRules"], "gamerules");
			parse_offset(offset.dwLocalPlayerPawn, parsed["client.dll"]["dwLocalPlayerPawn"], "localplayerpawn");
			parse_offset(offset.dwViewMatrix, parsed["client.dll"]["dwViewMatrix"], "viewmatrix");
		} else return false;
	} catch (const std::exception& e) {
		std::cerr << "offsets.json error: " << e.what() << std::endl;
	}

	std::string clientdll_json = comms.get_data(output_url + "/client_dll.json");
	if (clientdll_json.empty()) return false;

	try {
		auto parsed = json::parse(clientdll_json);
		if (parsed.contains("client.dll")) {
			std::cout << "global: parsing client_dll.json" << std::endl;

			// C_BaseEntity
			parse_offset(offset.m_iHealth, parsed["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"], "health");
			parse_offset(offset.m_iTeamNum, parsed["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"], "teamnum");
			parse_offset(offset.m_pGameSceneNode, parsed["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_pGameSceneNode"], "gamescenenode");

			// C_CSPlayerPawn
			parse_offset(offset.m_szLastPlaceName, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_szLastPlaceName"], "lastplacename");
			parse_offset(offset.m_entitySpottedState, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_entitySpottedState"], "spottedstate");
			parse_offset(offset.m_bIsScoped, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsScoped"], "isscoped");
			parse_offset(offset.m_bIsDefusing, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsDefusing"], "isdefusing");
			parse_offset(offset.m_bIsGrabbingHostage, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsGrabbingHostage"], "isgrabbinghostage");
			parse_offset(offset.m_iShotsFired, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iShotsFired"], "shotsfired");
			parse_offset(offset.m_pClippingWeapon, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pClippingWeapon"], "clippingweapon");
			parse_offset(offset.m_iIDEntIndex, parsed["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iIDEntIndex"], "identindex");

			// CBasePlayerController
			parse_offset(offset.m_iszPlayerName, parsed["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"], "playername");
			parse_offset(offset.m_steamID, parsed["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_steamID"], "steamid");

			// CCSPlayerController
			parse_offset(offset.m_hPlayerPawn, parsed["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"], "playerpawn");

			// C_CSPlayerPawnBase
			parse_offset(offset.m_flFlashOverlayAlpha, parsed["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_flFlashOverlayAlpha"], "flashoverlayalpha");

			// C_BasePlayerPawn
			parse_offset(offset.m_vOldOrigin, parsed["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"], "oldorigin");

			// Weapon-related
			parse_offset(offset.m_AttributeManager, parsed["client.dll"]["classes"]["C_EconEntity"]["fields"]["m_AttributeManager"], "attributemanager");
			parse_offset(offset.m_Item,	parsed["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"], "item");
			parse_offset(offset.m_iItemDefinitionIndex, parsed["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"], "itemdefinitionindex");

		} else return false;
	} catch (const std::exception& e) {
		std::cerr << "client_dll.json error: " << e.what() << std::endl;
	}

	std::string buttons_json = comms.get_data(output_url + "/buttons.json");
	if (buttons_json.empty()) return false;

	try {
		auto parsed = json::parse(buttons_json);
		if (parsed.contains("client.dll")) {
			std::cout << "global: parsing buttons.json" << std::endl;

			parse_offset(offset.attack_btn, parsed["client.dll"]["attack"], "attack");
			parse_offset(offset.use_btn,	parsed["client.dll"]["use"], "use");
		} else return false;
	} catch (const std::exception& e) {
		std::cerr << "buttons.json error: " << e.what() << std::endl;
	}

	return true;
}