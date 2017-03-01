#include "..\SDK\PluginSDK.h"

PluginSetup("ggTristana");


void loadMenu();
void loadSkills();
void combo();
void harass();
void killsteal();
void draw();
void drawCircle(ISpell2 * spell);
PLUGIN_API void OnLoad(IPluginSDK * sdk);
PLUGIN_API void OnUnload();
PLUGIN_EVENT(void) onUpdate();
PLUGIN_EVENT(void) onRender();
PLUGIN_EVENT(void) onLevel();
IUnit * getEnemyWithE();

/*
	Menu's
*/
IMenu * baseMenu;
IMenu * comboMenu;
IMenu * harassMenu;
IMenu * killstealMenu;
IMenu * killstealMenuSkillR;
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
IMenuOption * harassAttackE;
IMenuOption * harassMinMana;
IMenuOption * miscFocusTargetWithE;
IMenuOption * miscDontEUnderHealth;
IMenuOption * killstealWithR;
IMenuOption * killstealAboveHealth;

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
			GEventManager->AddEventHandler(kEventOnLevelUp, onLevel);
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
	GEventManager->RemoveEventHandler(kEventOnLevelUp, onLevel);

}


void loadMenu() {

	baseMenu = GPluginSDK->AddMenu("ggTristana");
	comboMenu = baseMenu->AddMenu("Combo");
	harassMenu = baseMenu->AddMenu("Harass");
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
		Load Harass options into Menu
	*/
	harassAttackE = harassMenu->CheckBox("E", true);
	harassMinMana = harassMenu->AddInteger("Min Mana for Harass", 0, 100, 50);

	/*
		Load Killsteal options into Menu
	*/

	killstealMenuSkillR = killstealMenu->AddMenu("R");
	killstealWithR = killstealMenuSkillR->CheckBox("Use R", true);
	killstealAboveHealth = killstealMenuSkillR->AddInteger("Ignore Enemy below Health", 0, 10000, 100);

	/*
		Load Misc options into Menu
	*/
	miscMenuSkillE = miscMenu->AddMenu("E");
	miscFocusTargetWithE = miscMenuSkillE->CheckBox("Focus Targets with E Debuff", false);
	miscDontEUnderHealth = miscMenuSkillE->AddInteger("Dont E Enemy Below Health", 0, 10000, 0);

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

	E->SetOverrideRange(650);
	R->SetOverrideRange(650);
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
				if (enemy->GetHealth() >= miscDontEUnderHealth->GetInteger()) {

					E->CastOnTarget(enemy);
				}
			}
		}
	}

}

void harass() {

	if (miscFocusTargetWithE->Enabled()) {
		GOrbwalking->SetOverrideTarget(getEnemyWithE());
	}

	if (me->ManaPercent() >= harassMinMana->GetInteger()) {
		if (harassAttackE->Enabled()) {
			if (E->IsReady()) {
				auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
				if (me->IsValidTarget(enemy, E->Range())) {
					E->CastOnTarget(enemy);
				}
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
					if (enemy->GetHealth() >= killstealAboveHealth->GetInteger()) {
						auto damage = GDamage->GetSpellDamage(me, enemy, kSlotR);

						if (enemy->HasBuff(debuff2)) {
							auto damage2 = GDamage->GetSpellDamage(me, enemy, kSlotE);
							damage2 = damage2 * (0.3 * enemy->GetBuffCount(debuff2));
							damage += damage2;
						}

						if (damage > enemy->GetHealth() + 50) {
							if (R->CastOnTarget(enemy)) {
								return;
							}
						}
					}

				}
			}
		}
	}
}

IUnit * getEnemyWithE() {

	for (auto enemy : GEntityList->GetAllHeros(false, true)) {
		if (enemy->HasBuff(debuff1)) {
			if (me->IsValidTarget(enemy, me->AttackRange())) {
				return enemy;
			}
		}
	}
	

	return GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, me->AttackRange());
}

PLUGIN_EVENT(void) onUpdate() {

	switch (GOrbwalking->GetOrbwalkingMode()) {
	case kModeCombo:
		combo();
		break;
	case kModeLaneClear:
		harass();
		break;
	}

	

	killsteal();


}

PLUGIN_EVENT(void) onRender() {
	draw();
	

}

PLUGIN_EVENT(void) onLevel() {
	E->SetOverrideRange(650 + (me->GetLevel() * 7));
	R->SetOverrideRange(650 + (me->GetLevel() * 7));
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