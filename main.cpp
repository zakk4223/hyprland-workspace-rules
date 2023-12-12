#include <hyprland/src/includes.hpp>
#include <any>
#include <sstream>
#define private public
#include <hyprland/src/config/ConfigManager.hpp>
#undef private
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/SharedDefs.hpp>
#include "globals.hpp"


// Methods
// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}


namespace {


std::deque<SWorkspaceRule> g_vMonitorWorkspaceRules;

	void onMonitorWSRule(const std::string& command, const std::string& value) {
    // This can either be the monitor or the workspace identifier
    const auto     FIRST_DELIM = value.find_first_of(',');

    std::string    name        = "";
    auto           monitor_ident = removeBeginEndSpacesTabs(value.substr(0, FIRST_DELIM)); //monitor description

    auto           rules = value.substr(FIRST_DELIM + 1);
    SWorkspaceRule wsRule;
    wsRule.workspaceString = monitor_ident;

    const static std::string ruleOnCreatedEmtpy    = "on-created-empty:";
    const static int         ruleOnCreatedEmtpyLen = ruleOnCreatedEmtpy.length();

    auto                     assignRule = [&](std::string rule) {
        size_t delim = std::string::npos;
        if ((delim = rule.find("gapsin:")) != std::string::npos)
            wsRule.gapsIn = std::stoi(rule.substr(delim + 7));
        else if ((delim = rule.find("gapsout:")) != std::string::npos)
            wsRule.gapsOut = std::stoi(rule.substr(delim + 8));
        else if ((delim = rule.find("bordersize:")) != std::string::npos)
            wsRule.borderSize = std::stoi(rule.substr(delim + 11));
        else if ((delim = rule.find("border:")) != std::string::npos)
            wsRule.border = configStringToInt(rule.substr(delim + 7));
        else if ((delim = rule.find("shadow:")) != std::string::npos)
            wsRule.shadow = configStringToInt(rule.substr(delim + 7));
        else if ((delim = rule.find("rounding:")) != std::string::npos)
            wsRule.rounding = configStringToInt(rule.substr(delim + 9));
        else if ((delim = rule.find("decorate:")) != std::string::npos)
            wsRule.decorate = configStringToInt(rule.substr(delim + 9));
        else if ((delim = rule.find("monitor:")) != std::string::npos)
            wsRule.monitor = rule.substr(delim + 8);
        else if ((delim = rule.find("default:")) != std::string::npos)
            wsRule.isDefault = configStringToInt(rule.substr(delim + 8));
        else if ((delim = rule.find("persistent:")) != std::string::npos)
            wsRule.isPersistent = configStringToInt(rule.substr(delim + 11));
        else if ((delim = rule.find(ruleOnCreatedEmtpy)) != std::string::npos)
            wsRule.onCreatedEmptyRunCmd = cleanCmdForWorkspace(name, rule.substr(delim + ruleOnCreatedEmtpyLen));
        else if ((delim = rule.find("layoutopt:")) != std::string::npos) {
            std::string opt = rule.substr(delim + 10);
            if (!opt.contains(":")) {
                // invalid
                Debug::log(ERR, "Invalid workspace rule found: {}", rule);
                g_pConfigManager->parseError = "Invalid workspace rule found: " + rule;
                return;
            }

            std::string val = opt.substr(opt.find(":") + 1);
            opt             = opt.substr(0, opt.find(":"));

            wsRule.layoutopts[opt] = val;
        }
    };

    size_t      pos = 0;
    std::string rule;
    while ((pos = rules.find(',')) != std::string::npos) {
        rule = rules.substr(0, pos);
        assignRule(rule);
        rules.erase(0, pos + 1);
    }
    assignRule(rules); // match remaining rule

    const auto IT = std::find_if(g_pConfigManager->m_dWorkspaceRules.begin(), g_pConfigManager->m_dWorkspaceRules.end(), [&](const auto& other) { return other.workspaceString == wsRule.workspaceString; });

    if (IT == g_pConfigManager->m_dWorkspaceRules.end())
        g_pConfigManager->m_dWorkspaceRules.emplace_back(wsRule);
    else
        *IT = wsRule;
	}

  inline CFunctionHook *g_pgetWorkspaceRuleForHook = nullptr;

	SWorkspaceRule hkgetWorkspaceRuleFor(void *thisptr, CWorkspace* pWorkspace) {
    const auto WORKSPACEIDSTR = std::to_string(pWorkspace->m_iID);
    const auto IT             = std::find_if(g_pConfigManager->m_dWorkspaceRules.begin(), g_pConfigManager->m_dWorkspaceRules.end(), [&](const auto& other) {
        return other.workspaceName == pWorkspace->m_szName /* name matches */
            || (pWorkspace->m_bIsSpecialWorkspace && other.workspaceName.starts_with("special:") &&
                other.workspaceName.substr(8) == pWorkspace->m_szName)           /* special and special:name */
            || (pWorkspace->m_iID > 0 && WORKSPACEIDSTR == other.workspaceName); /* id matches and workspace is numerical */
    });
    if (IT == g_pConfigManager->m_dWorkspaceRules.end()) {
				const auto PMONITOR = g_pCompositor->getMonitorFromID(pWorkspace->m_iMonitorID);
				const auto MONITORNAME = PMONITOR->szName;
				std::string MONITORDESC = PMONITOR->output->description ? PMONITOR->output->description : "";
				//Try monitor specific rules. I overloaded the workspace string....
				const auto MT = std::find_if(g_pConfigManager->m_dWorkspaceRules.begin(), g_pConfigManager->m_dWorkspaceRules.end(), [&](const auto& other) {
					return other.workspaceString == MONITORNAME || (other.workspaceString.starts_with("desc:") && (other.workspaceString.substr(5) == MONITORDESC || other.workspaceString.substr(5) == removeBeginEndSpacesTabs(MONITORDESC.substr(0,MONITORDESC.find_first_of('('))))); });
				if (MT == g_pConfigManager->m_dWorkspaceRules.end()) {
        	return SWorkspaceRule{};
				} else {
					return *MT;
				}
		}
    return *IT;
	}


}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;


		
	 	static const auto WSRULEMETHODS = HyprlandAPI::findFunctionsByName(PHANDLE, "getWorkspaceRuleFor");
		g_pgetWorkspaceRuleForHook = HyprlandAPI::createFunctionHook(PHANDLE, WSRULEMETHODS[0].address, (void *)&hkgetWorkspaceRuleFor);
		g_pgetWorkspaceRuleForHook->hook();
		
		HyprlandAPI::addConfigKeyword(PHANDLE, "monitorwsrule", [&](const std::string& k, const std::string& v) {onMonitorWSRule(k,v);});

    HyprlandAPI::reloadConfig();



    return {"Monitor WS rules", "Allow workspace rules per monitor", "Zakk", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    HyprlandAPI::invokeHyprctlCommand("seterror", "disable");
}
