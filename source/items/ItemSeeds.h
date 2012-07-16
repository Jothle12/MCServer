#pragma once

#include "Item.h"
#include "../cWorld.h"

class cItemSeedsHandler : public cItemHandler
{
public:
	cItemSeedsHandler(int a_ItemID)
		: cItemHandler(a_ItemID)
	{

	}

	virtual bool IsPlaceable()
	{
		return true;
	}

	virtual bool AllowBlockOnTop()
	{
		return false;
	}
	
	virtual BLOCKTYPE GetBlockType()
	{
		switch(m_ItemID)
		{
		case E_ITEM_SEEDS:
			return E_BLOCK_CROPS;
		case E_ITEM_MELON_SEEDS:
			E_BLOCK_MELON_STEM;
		case E_ITEM_PUMPKIN_SEEDS:
			E_BLOCK_PUMPKIN_STEM;
		default:
			return E_BLOCK_AIR;
		}
	}

	virtual char GetBlockMeta(char a_ItemMeta)
	{
		return 0;	//Not grown yet
	}
	
	virtual void PlaceBlock(cWorld *a_World, cPlayer *a_Player, cItem *a_Item, int a_X, int a_Y, int a_Z, char a_Dir)
	{
		int X = a_X, 
			Y = a_Y, 
			Z = a_Z;

		AddDirection(X, Y, Z, a_Dir, true);

		if(a_World->GetBlock(X, Y, Z) != E_BLOCK_FARMLAND)
			return;
		
		return cItemHandler::PlaceBlock(a_World, a_Player, a_Item, a_X, a_Y, a_Z, a_Dir);
	}
};