#ifndef HOOKS_HPP
#define HOOKS_HPP

#include "../Player/Player.h"
#include "../Player/PedTypes.h"

typedef void (*pOrigTakeDamageAddy)();
pOrigTakeDamageAddy oTakeDamageAddy = (pOrigTakeDamageAddy)0x004B32E8;
bool e_onTakeDamage = false;

// HOOKED EVENT
void onPlayerTakeDamage(void) {
    e_onTakeDamage = true;
    oTakeDamageAddy();
}

typedef Player* (__cdecl* pOrigAddPed)(ePedType pedType, eModelID modelIndex, CVector3D& createAtPos, bool makeWander);
pOrigAddPed oAddPedAddy = (pOrigAddPed)0x612711;
/* USAGE EXAMPLE
* 
    CVector3D pos;
    pos.x = LOCAL_PLAYER.ped->getPosition()->x;
    pos.y = LOCAL_PLAYER.ped->getPosition()->y + 2;
    pos.z = LOCAL_PLAYER.ped->getPosition()->z;
    std::cout << pos.x << " | " << pos.y << " | " << pos.z << "\n";
    Player* ped = oAddPedAddy(PED_TYPE_CIVMALE, (eModelID)0, pos, 0);

    THE MODEL ID MUST BE LOADED FIRST!
*/

typedef void(__cdecl* pOrigRemoveAllRandomPeds)(void);
pOrigRemoveAllRandomPeds oRemoveAllRandomPedsAddy = (pOrigRemoveAllRandomPeds)0x6122C0;
/* USAGE EXAMPLE

    oRemoveAllRandomPeds();
    
*/

typedef void(__cdecl* pOrigRemovePed)(Player* ped);
pOrigRemovePed oRemovePedAddy = (pOrigRemovePed)0x610F20;

#endif