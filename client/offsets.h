#pragma once
#include <sys/types.h>
#include <string>
#include "include/json.hpp"

class communications;

class offsets {
    public:
		std::string output_url = "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output";
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
	    std::ptrdiff_t m_entitySpottedState;
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

        bool get_offsets(communications& comms);
	private:
		void parse_offset(std::ptrdiff_t& target, const nlohmann::json& jpath, std::string name);
};


extern offsets offset;