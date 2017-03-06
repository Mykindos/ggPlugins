#include "..\SDK\PluginSDK.h"
#include "..\Utilities\UtilPosition.h"
#include <algorithm>

void loadMenu();

PLUGIN_EVENT(void) onRender();
PLUGIN_API void OnLoad(IPluginSDK * sdk);
PLUGIN_API void OnUnload();
bool hasSmite(IUnit * hero);

std::vector<IUnit*> tracked;

IUnit * me;

IMenu * mainMenu;
IMenu * heroMenu;

IMenuOption * gankDistance;
IMenuOption * usePings;
IMenuOption * onlyShowSmite;



PluginSetup("ggGankAlerter");


void loadMenu() {

	mainMenu = GPluginSDK->AddMenu("ggGankAlerter");
	heroMenu = mainMenu->AddMenu("Champions");

	gankDistance = mainMenu->AddInteger("Gank Alert Distance", 0, 100000, 5000);
	usePings = mainMenu->CheckBox("Use Pings (Client Side)", true);
	onlyShowSmite = mainMenu->CheckBox("Only Show Enemies /w Smite", false);

	for (auto enemy : GEntityList->GetAllHeros(false, true)) {
		heroMenu->CheckBox(enemy->ChampionName(), true);
	}

}

PLUGIN_API void OnLoad(IPluginSDK * sdk) {
	PluginSDKSetup(sdk);
	me = GEntityList->Player();
	loadMenu();
	GEventManager->AddEventHandler(kEventOnRender, onRender);
}

PLUGIN_API void OnUnload() {
	GEventManager->RemoveEventHandler(kEventOnRender, onRender);
	mainMenu->Remove();
}

PLUGIN_EVENT(void) onRender() {
	for (auto e : GEntityList->GetAllHeros(false, true)) {
		if (e->IsVisible()) {
			if (!e->IsDead()) {
				if (UtilPosition::getDistance(me, e) < gankDistance->GetInteger()) {
					IMenuOption * temp = heroMenu->GetOption(e->ChampionName());
					if (onlyShowSmite->Enabled()) {
						if (!hasSmite(e)) {
							continue;
						}
					}
					if (temp) {
						if (temp->Enabled()) {
							if (usePings->Enabled()) {
								if (std::find(tracked.begin(), tracked.end(), e) == tracked.end()) {
									tracked.push_back(e);
									GGame->ShowPing(2, e->GetPosition(), true);

								}

							}

							Vec2 myPos;
							Vec2 ePos;
							GGame->Projection(me->GetPosition(), &myPos);
							GGame->Projection(e->GetPosition(), &ePos);
							GRender->DrawLine(myPos, ePos, Vec4(255, 0, 0, 255));
						}
					}
				}
				else {
					if (usePings->Enabled()) {
						tracked.erase(std::remove(tracked.begin(), tracked.end(), e), tracked.end());
					}
				}
				
			}
			

		}
		
	}
}

bool hasSmite(IUnit * hero) {
	return strcmp(hero->GetSpellName(kSummonerSlot1), "SummonerSmite") == 0 
		|| strcmp(hero->GetSpellName(kSummonerSlot2), "SummonerSmite") == 0;
}


