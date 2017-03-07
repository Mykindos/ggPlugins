#include "..\SDK\PluginSDK.h"

class UtilPosition {

public:
	static float getDistance(IUnit* p, IUnit* t)
	{
		return (p->GetPosition() - t->GetPosition()).Length2D();
	}
};