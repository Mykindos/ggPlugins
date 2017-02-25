#include "..\SDK\PluginSDK.h"

PluginSetup("ggTristana");

void loadMenu();
void loadSkills();
void combo();
void killsteal();
void draw();
void drawCircle(ISpell2 * spell);
PLUGIN_API void OnLoad(IPluginSDK * sdk);
PLUGIN_API void OnUnload();
PLUGIN_EVENT(void) onUpdate();
PLUGIN_EVENT(void) onRender();
IUnit * getEnemyWithE();

/*
	Menu's
*/
IMenu * baseMenu;
IMenu * comboMenu;
IMenu * killstealMenu;
IMenu * drawMenu;
IMenu * miscMenu;
IMenu * miscMenuSkillE;
IMenu * miscMenuSkillQ;

/*
	Menu Options
*/
IMenuOption * comboAttackE;
IMenuOption * comboAttackQ;
IMenuOption * comboAttackR;
IMenuOption * miscFocusTargetWithE;
IMenuOption * killstealWithR;

IMenuOption * drawE;
IMenuOption * drawW;
IMenuOption * drawR;

/*
	My Character
*/
IUnit * me;

/*
	Character Spells
*/
ISpell2 * Q;
ISpell2 * W;
ISpell2 * E;
ISpell2 * R;

const char * debuff1 = "tristanaechargesound";
const char * debuff2 = "tristanaecharge";


PLUGIN_API void OnLoad(IPluginSDK * sdk) {
	PluginSDKSetup(sdk);

	me = GEntityList->Player();
	if (me) {
		if (strcmp(GEntityList->Player()->ChampionName(), "Tristana") == 0) {
			loadMenu();
			loadSkills();

			GEventManager->AddEventHandler(kEventOnGameUpdate, onUpdate);
			GEventManager->AddEventHandler(kEventOnRender, onRender);
			GGame->PrintChat("ggTristana Loaded - Written by Mykindos");
		}
		else {
			GGame->PrintChat("Not Playing Tristana! - Unloaded");
		}
	}
}

PLUGIN_API void OnUnload()
{
	if (baseMenu) {
		baseMenu->Remove();
	}

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, onUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, onRender);

}


void loadMenu() {

	baseMenu = GPluginSDK->AddMenu("ggTristana");
	comboMenu = baseMenu->AddMenu("Combo");
	killstealMenu = baseMenu->AddMenu("Killsteal");
	miscMenu = baseMenu->AddMenu("Misc.");
	drawMenu = baseMenu->AddMenu("Drawing");


	/*
		Load Combo options into menu
	*/
	comboAttackQ = comboMenu->CheckBox("Q", true);
	comboAttackE = comboMenu->CheckBox("E", true);
	//comboAttackR = comboMenu->CheckBox("R", true);
	
	/*
		Load Killsteal options into Menu
	*/

	killstealWithR = killstealMenu->CheckBox("R", true);

	/*
		Load Misc options into Menu
	*/
	miscMenuSkillE = miscMenu->AddMenu("E");
	miscFocusTargetWithE = miscMenuSkillE->CheckBox("Focus Targets with E Debuff", false);

	/*
		Load Drawing options into Menu
	*/
	drawE = drawMenu->CheckBox("Draw E", true);
	drawW = drawMenu->CheckBox("Draw W", true);
	drawR = drawMenu->CheckBox("Draw R", true);
}

void loadSkills() {
	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, false, static_cast<eCollisionFlags>(kCollidesWithNothing));
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, true, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
}

void combo() {

	if (comboAttackQ->Enabled()) {
		if (Q->IsReady()) {
			auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, me->AttackRange());
			if (me->IsValidTarget(enemy, me->AttackRange())) {
				Q->CastOnPlayer();
			}
			
		}
	}

	if (comboAttackE->Enabled()) {

		if (miscFocusTargetWithE->Enabled()) {
			GOrbwalking->SetOverrideTarget(getEnemyWithE());
		}

		if (E->IsReady()) {
			auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
			if (me->IsValidTarget(enemy, E->Range())) {
				E->CastOnTarget(enemy);
			}
		}
	}

}

void killsteal() {
	if (killstealWithR->Enabled()) {
		
		if (R->IsReady()) {
			
			auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, R->Range());
			
				if (me->IsValidTarget(enemy, R->Range())) {
					if (!enemy->IsDead() && !enemy->IsInvulnerable() && !enemy->HasBuffOfType(BUFF_SpellImmunity)) {
						auto damage = GDamage->GetSpellDamage(me, enemy, kSlotR);
						
						if (enemy->HasBuff(debuff2)) {
							auto damage2 = GDamage->GetSpellDamage(me, enemy, kSlotE);
							damage2 = damage2 * (0.3 * enemy->GetBuffCount(debuff2));

							damage += damage2;
						}

						if (damage > enemy->GetHealth()) {
							if (R->CastOnTarget(enemy)) {
								return;
							}
						}
					}
				
			}
		}
	}
}

IUnit * getEnemyWithE() {
	/* Causes unknown client crash??
	for (auto enemy : GEntityList->GetAllHeros(false, true)) {
		if (enemy->HasBuff(debuff1)) {
			if (me->IsValidTarget(enemy, me->AttackRange())) {
				return enemy;
			}
		}
	}
	*/ 

	return GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, me->AttackRange());
}

PLUGIN_EVENT(void) onUpdate() {
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		combo();
	}

	killsteal();


}

PLUGIN_EVENT(void) onRender() {
	draw();
}

void draw() {
	if (drawE->Enabled()) {
		drawCircle(E);
	}
	if (drawW->Enabled()) {
		drawCircle(W);
	}
	if (drawR->Enabled()) {
		drawCircle(R);
	}
}

void drawCircle(ISpell2 * spell) {
	GRender->DrawOutlinedCircle(me->GetPosition(), Vec4(255, 255, 0, 255), spell->Range());
}