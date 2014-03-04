
#include "Globals.h"
#include "BlockPiston.h"
#include "../Item.h"
#include "../World.h"
#include "../Entities/Player.h"
#include "../Piston.h"

#include "../ChunkDef.h"
#include "../Entities/Pickup.h"
#include "../Root.h"
#include "../ClientHandle.h"
#include "../Server.h"
#include "../BlockInServerPluginInterface.h"





#define AddPistonDir(x, y, z, dir, amount) \
	switch (dir) \
	{ \
		case 0: (y) -= (amount); break; \
		case 1: (y) += (amount); break; \
		case 2: (z) -= (amount); break; \
		case 3: (z) += (amount); break; \
		case 4: (x) -= (amount); break; \
		case 5: (x) += (amount); break; \
	}

/// Number of ticks that the piston extending / retracting waits before setting the block
const int PISTON_TICK_DELAY = 1;


cBlockPistonHandler::cBlockPistonHandler(BLOCKTYPE a_BlockType)
	: cBlockHandler(a_BlockType)
{
}





void cBlockPistonHandler::OnDestroyed(cChunkInterface & a_ChunkInterface, cWorldInterface & a_WorldInterface, int a_BlockX, int a_BlockY, int a_BlockZ)
{
	NIBBLETYPE OldMeta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY, a_BlockZ);

	int newX = a_BlockX;
	int newY = a_BlockY;
	int newZ = a_BlockZ;
	AddPistonDir(newX, newY, newZ, OldMeta & ~(8), 1);

	if (a_ChunkInterface.GetBlock(newX, newY, newZ) == E_BLOCK_PISTON_EXTENSION)
	{
		a_ChunkInterface.SetBlock(a_WorldInterface, newX, newY, newZ, E_BLOCK_AIR, 0);
	}
}





bool cBlockPistonHandler::GetPlacementBlockTypeMeta(
	cChunkInterface & a_ChunkInterface, cPlayer * a_Player,
	int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_BlockFace, 
	int a_CursorX, int a_CursorY, int a_CursorZ,
	BLOCKTYPE & a_BlockType, NIBBLETYPE & a_BlockMeta
)
{
	a_BlockType = m_BlockType;
	a_BlockMeta = BlockPiston::RotationPitchToMetaData(a_Player->GetYaw(), a_Player->GetPitch());
	return true;
}





void cBlockPistonHandler::ExtendPiston(int pistx, int pisty, int pistz)
{
	BLOCKTYPE pistonBlock;
	NIBBLETYPE pistonMeta;
	m_World->GetBlockTypeMeta(pistx, pisty, pistz, pistonBlock, pistonMeta);
	
	if (IsExtended(pistonMeta))
	{
		// Already extended, bail out
		return;
	}

	int dist = FirstPassthroughBlock(pistx, pisty, pistz, pistonMeta);
	if (dist < 0)
	{
		// FirstPassthroughBlock says piston can't push anything, bail out
		return;
	}

	m_World->BroadcastBlockAction(pistx, pisty, pistz, 0, pistonMeta, pistonBlock);
	m_World->BroadcastSoundEffect("tile.piston.out", pistx * 8, pisty * 8, pistz * 8, 0.5f, 0.7f);	
	
	// Drop the breakable block in the line, if appropriate:
	AddPistonDir(pistx, pisty, pistz, pistonMeta, dist + 1);  // "pist" now at the breakable / empty block
	BLOCKTYPE currBlock;
	NIBBLETYPE currMeta;
	m_World->GetBlockTypeMeta(pistx, pisty, pistz, currBlock, currMeta);
	if (currBlock != E_BLOCK_AIR)
	{
		cBlockHandler * Handler = BlockHandler(currBlock);
		if (Handler->DoesDropOnUnsuitable())
		{
			cChunkInterface ChunkInterface(m_World->GetChunkMap());
			cBlockInServerPluginInterface PluginInterface(*m_World);
			Handler->DropBlock(ChunkInterface, *m_World, PluginInterface, NULL, pistx, pisty, pistz);
		}
	}
	
	// Push blocks, from the furthest to the nearest:
	int oldx = pistx, oldy = pisty, oldz = pistz;
	NIBBLETYPE currBlockMeta;
	for (int i = dist + 1; i > 1; i--)
	{
		AddPistonDir(pistx, pisty, pistz, pistonMeta, -1);
		m_World->GetBlockTypeMeta(pistx, pisty, pistz, currBlock, currBlockMeta);
		m_World->QueueSetBlock( oldx, oldy, oldz, currBlock, currBlockMeta, PISTON_TICK_DELAY);
		oldx = pistx;
		oldy = pisty;
		oldz = pistz;
	}

	int extx = pistx;
	int exty = pisty;
	int extz = pistz;
	AddPistonDir(pistx, pisty, pistz, pistonMeta, -1);
	// "pist" now at piston body, "ext" at future extension
	
	m_World->SetBlock(pistx, pisty, pistz, pistonBlock, pistonMeta | 0x8);
	m_World->QueueSetBlock(extx, exty, extz, E_BLOCK_PISTON_EXTENSION, pistonMeta | (IsSticky(pistonBlock) ? 8 : 0), PISTON_TICK_DELAY);
}





void cBlockPistonHandler::RetractPiston(int pistx, int pisty, int pistz)
{
	BLOCKTYPE pistonBlock;
	NIBBLETYPE pistonMeta;
	m_World->GetBlockTypeMeta(pistx, pisty, pistz, pistonBlock, pistonMeta);

	if (!IsExtended(pistonMeta))
	{
		// Already retracted, bail out
		return;
	}

	// Check the extension:
	AddPistonDir(pistx, pisty, pistz, pistonMeta, 1);
	if (m_World->GetBlock(pistx, pisty, pistz) != E_BLOCK_PISTON_EXTENSION)
	{
		LOGD("%s: Piston without an extension - still extending, or just in an invalid state?", __FUNCTION__);
		return;
	}
	
	AddPistonDir(pistx, pisty, pistz, pistonMeta, -1);
	m_World->SetBlock(pistx, pisty, pistz, pistonBlock, pistonMeta & ~(8));
	m_World->BroadcastBlockAction(pistx, pisty, pistz, 1, pistonMeta & ~(8), pistonBlock);
	m_World->BroadcastSoundEffect("tile.piston.in", pistx * 8, pisty * 8, pistz * 8, 0.5f, 0.7f);
	AddPistonDir(pistx, pisty, pistz, pistonMeta, 1);
	
	// Retract the extension, pull block if appropriate
	if (IsSticky(pistonBlock))
	{
		int tempx = pistx, tempy = pisty, tempz = pistz;
		AddPistonDir(tempx, tempy, tempz, pistonMeta, 1);
		BLOCKTYPE tempBlock;
		NIBBLETYPE tempMeta;
		m_World->GetBlockTypeMeta(tempx, tempy, tempz, tempBlock, tempMeta);
		if (CanPull(tempBlock, tempMeta))
		{
			// Pull the block
			m_World->QueueSetBlock(pistx, pisty, pistz, tempBlock, tempMeta, PISTON_TICK_DELAY);
			m_World->QueueSetBlock(tempx, tempy, tempz, E_BLOCK_AIR, 0, PISTON_TICK_DELAY);
		}
		else
		{
			// Retract without pulling
			m_World->QueueSetBlock(pistx, pisty, pistz, E_BLOCK_AIR, 0, PISTON_TICK_DELAY);
		}
	}
	else
	{
		m_World->QueueSetBlock(pistx, pisty, pistz, E_BLOCK_AIR, 0, PISTON_TICK_DELAY);
	}
}





bool cBlockPistonHandler::IsExtended(NIBBLETYPE a_PistonMeta)
{
	return ((a_PistonMeta & 0x8) != 0x0);
}





bool cBlockPistonHandler::IsSticky(BLOCKTYPE a_BlockType)
{
	return (a_BlockType == E_BLOCK_STICKY_PISTON);
}





bool cBlockPistonHandler::CanPush(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta)
{
	switch (a_BlockType)
	{
		case E_BLOCK_ANVIL:
		case E_BLOCK_BED:
		case E_BLOCK_BEDROCK:
		case E_BLOCK_BREWING_STAND:
		case E_BLOCK_CHEST:
		case E_BLOCK_COMMAND_BLOCK:
		case E_BLOCK_DISPENSER:
		case E_BLOCK_DROPPER:
		case E_BLOCK_ENCHANTMENT_TABLE:
		case E_BLOCK_END_PORTAL:
		case E_BLOCK_END_PORTAL_FRAME:
		case E_BLOCK_FURNACE:
		case E_BLOCK_LIT_FURNACE:
		case E_BLOCK_HOPPER:
		case E_BLOCK_JUKEBOX:
		case E_BLOCK_MOB_SPAWNER:
		case E_BLOCK_NETHER_PORTAL:
		case E_BLOCK_NOTE_BLOCK:
		case E_BLOCK_OBSIDIAN:
		case E_BLOCK_PISTON_EXTENSION:
		{
			return false;
		}
		case E_BLOCK_STICKY_PISTON:
		case E_BLOCK_PISTON:
		{
			// A piston can only be pushed if retracted:
			return !IsExtended(a_BlockMeta);
		}
	}
	return true;
}





bool cBlockPistonHandler::CanBreakPush(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta)
{
	UNUSED(a_BlockMeta);
	return cBlockInfo::IsPistonBreakable(a_BlockType);
}





bool cBlockPistonHandler::CanPull(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta)
{
	switch (a_BlockType)
	{
		case E_BLOCK_LAVA:
		case E_BLOCK_STATIONARY_LAVA:
		case E_BLOCK_STATIONARY_WATER:
		case E_BLOCK_WATER:
		{
			return false;
		}
	}

	if (CanBreakPush(a_BlockType, a_BlockMeta))
	{
		return false; // CanBreakPush returns true, but we need false to prevent pulling
	}

	return CanPush(a_BlockType, a_BlockMeta);
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// cBlockPistonHeadHandler:

cBlockPistonHeadHandler::cBlockPistonHeadHandler(void) :
	super(E_BLOCK_PISTON_EXTENSION)
{
}





void cBlockPistonHeadHandler::OnDestroyedByPlayer(cChunkInterface & a_ChunkInterface, cWorldInterface & a_WorldInterface, cPlayer * a_Player, int a_BlockX, int a_BlockY, int a_BlockZ)
{
	NIBBLETYPE OldMeta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY, a_BlockZ);

	int newX = a_BlockX;
	int newY = a_BlockY;
	int newZ = a_BlockZ;
	AddPistonDir(newX, newY, newZ, OldMeta & ~(8), -1);

	BLOCKTYPE Block = a_ChunkInterface.GetBlock(newX, newY, newZ);
	if ((Block == E_BLOCK_STICKY_PISTON) || (Block == E_BLOCK_PISTON))
	{
		a_ChunkInterface.DigBlock(a_WorldInterface, newX, newY, newZ);
		if (a_Player->IsGameModeCreative())
		{
			return; // No pickups if creative
		}

		cItems Pickups;
		Pickups.push_back(cItem(Block, 1));
		a_WorldInterface.SpawnItemPickups(Pickups, a_BlockX + 0.5, a_BlockY + 0.5, a_BlockZ + 0.5);
	}
}





