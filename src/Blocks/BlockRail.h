
#pragma once

#include "BlockEntity.h"





enum ENUM_PURE
{
	E_PURE_UPDOWN = 0,
	E_PURE_DOWN = 1,
	E_PURE_NONE = 2
};





class cBlockRailHandler :
	public cBlockHandler
{
	typedef cBlockHandler super;
	
public:
	cBlockRailHandler(BLOCKTYPE a_BlockType)
		: cBlockHandler(a_BlockType)
	{
	}
	
	virtual bool GetPlacementBlockTypeMeta(
		cChunkInterface & a_ChunkInterface, cPlayer * a_Player,
		int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_BlockFace, 
		int a_CursorX, int a_CursorY, int a_CursorZ,
		BLOCKTYPE & a_BlockType, NIBBLETYPE & a_BlockMeta
	) override
	{
		a_BlockType = m_BlockType;
		a_BlockMeta = FindMeta(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ);
		return true;
	}


	virtual void OnPlaced(cChunkInterface & a_ChunkInterface, cWorldInterface & a_WorldInterface, int a_BlockX, int a_BlockY, int a_BlockZ, BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta) override
	{
		super::OnPlaced(a_ChunkInterface, a_WorldInterface, a_BlockX, a_BlockY, a_BlockZ, a_BlockType, a_BlockMeta);

		// Alert diagonal rails
		OnNeighborChanged(a_ChunkInterface, a_BlockX + 1, a_BlockY + 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX - 1, a_BlockY + 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ + 1);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ - 1);

		OnNeighborChanged(a_ChunkInterface, a_BlockX + 1, a_BlockY - 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX - 1, a_BlockY - 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ + 1);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ - 1);
	}


	virtual void OnDestroyed(cChunkInterface & a_ChunkInterface, cWorldInterface & a_WorldInterface, int a_BlockX, int a_BlockY, int a_BlockZ) override
	{
		super::OnDestroyed(a_ChunkInterface, a_WorldInterface, a_BlockX, a_BlockY, a_BlockZ);

		// Alert diagonal rails
		OnNeighborChanged(a_ChunkInterface, a_BlockX + 1, a_BlockY + 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX - 1, a_BlockY + 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ + 1);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ - 1);

		OnNeighborChanged(a_ChunkInterface, a_BlockX + 1, a_BlockY - 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX - 1, a_BlockY - 1, a_BlockZ);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ + 1);
		OnNeighborChanged(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ - 1);
	}


	virtual void OnNeighborChanged(cChunkInterface & a_ChunkInterface, int a_BlockX, int a_BlockY, int a_BlockZ) override
	{
		NIBBLETYPE Meta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY, a_BlockZ);
		if (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ) && (Meta != FindMeta(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ)))
		{
			a_ChunkInterface.FastSetBlock(a_BlockX, a_BlockY, a_BlockZ, m_BlockType, FindMeta(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ));
		}
	}


	virtual void ConvertToPickups(cItems & a_Pickups, NIBBLETYPE a_BlockMeta) override
	{
		super::ConvertToPickups(a_Pickups, 0);
	}


	virtual bool CanBeAt(cChunkInterface & a_ChunkInterface, int a_RelX, int a_RelY, int a_RelZ, const cChunk & a_Chunk) override
	{
		if (a_RelY <= 0)
		{
			return false;
		}
		if (!cBlockInfo::IsSolid(a_Chunk.GetBlock(a_RelX, a_RelY - 1, a_RelZ)))
		{
			return false;
		}

		NIBBLETYPE Meta = a_Chunk.GetMeta(a_RelX, a_RelY, a_RelZ);
		switch (Meta)
		{
			case E_META_RAIL_ASCEND_XP:
			case E_META_RAIL_ASCEND_XM:
			case E_META_RAIL_ASCEND_ZM:
			case E_META_RAIL_ASCEND_ZP:
			{
				// Mapping between the meta and the neighbors that need checking
				Meta -= E_META_RAIL_ASCEND_XP;  // Base index at zero
				static const struct
				{
					int x, z;
				} Coords[] =
				{
					{ 1,  0},  // east,  XP
					{-1,  0},  // west,  XM
					{ 0, -1},  // north, ZM
					{ 0,  1},  // south, ZP
				} ;
				BLOCKTYPE  BlockType;
				NIBBLETYPE BlockMeta;
				if (!a_Chunk.UnboundedRelGetBlock(a_RelX + Coords[Meta].x, a_RelY, a_RelZ + Coords[Meta].z, BlockType, BlockMeta))
				{
					// Too close to the edge, cannot simulate
					return true;
				}
				return cBlockInfo::IsSolid(BlockType);
			}
		}
		return true;
	}

	NIBBLETYPE FindMeta(cChunkInterface & a_ChunkInterface, int a_BlockX, int a_BlockY, int a_BlockZ)
	{
		NIBBLETYPE Meta = 0;
		char RailsCnt = 0;
		bool Neighbors[8]; // 0 - EAST, 1 - WEST, 2 - NORTH, 3 - SOUTH, 4 - EAST UP, 5 - WEST UP, 6 - NORTH UP, 7 - SOUTH UP
		memset(Neighbors, false, sizeof(Neighbors));
		Neighbors[0] = (IsUnstable(a_ChunkInterface, a_BlockX + 1, a_BlockY, a_BlockZ) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_EAST, E_PURE_DOWN));
		Neighbors[1] = (IsUnstable(a_ChunkInterface, a_BlockX - 1, a_BlockY, a_BlockZ) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_WEST, E_PURE_DOWN));
		Neighbors[2] = (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ - 1) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_NORTH, E_PURE_DOWN));
		Neighbors[3] = (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ + 1) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_SOUTH, E_PURE_DOWN));
		Neighbors[4] = (IsUnstable(a_ChunkInterface, a_BlockX + 1, a_BlockY + 1, a_BlockZ) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_EAST, E_PURE_NONE));
		Neighbors[5] = (IsUnstable(a_ChunkInterface, a_BlockX - 1, a_BlockY + 1, a_BlockZ) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_WEST, E_PURE_NONE));
		Neighbors[6] = (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ - 1) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_NORTH, E_PURE_NONE));
		Neighbors[7] = (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ + 1) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_SOUTH, E_PURE_NONE));
		if (IsUnstable(a_ChunkInterface, a_BlockX + 1, a_BlockY - 1, a_BlockZ) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ, BLOCK_FACE_EAST))
			Neighbors[0] = true;
		if (IsUnstable(a_ChunkInterface, a_BlockX - 1, a_BlockY - 1, a_BlockZ) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ, BLOCK_FACE_WEST))
			Neighbors[1] = true;
		if (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ - 1) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ, BLOCK_FACE_NORTH))
			Neighbors[2] = true;
		if (IsUnstable(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ + 1) || !IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY - 1, a_BlockZ, BLOCK_FACE_SOUTH))
			Neighbors[3] = true;
		for (int i = 0; i < 8; i++)
		{
			if (Neighbors[i])
			{
				RailsCnt++;
			}
		}
		if (RailsCnt == 1)
		{
			if (Neighbors[7]) return E_META_RAIL_ASCEND_ZP;
			else if (Neighbors[6]) return E_META_RAIL_ASCEND_ZM;
			else if (Neighbors[5]) return E_META_RAIL_ASCEND_XM;
			else if (Neighbors[4]) return E_META_RAIL_ASCEND_XP;
			else if (Neighbors[0] || Neighbors[1]) return E_META_RAIL_XM_XP;
			else if (Neighbors[2] || Neighbors[3]) return E_META_RAIL_ZM_ZP;
			ASSERT(!"Weird neighbor count");
			return Meta;
		}
		for (int i = 0; i < 4; i++)
		{
			if (Neighbors[i + 4])
			{
				Neighbors[i] = true;
			}
		}
		if (RailsCnt > 1)
		{
			if      (Neighbors[3] && Neighbors[0] && CanThisRailCurve()) return E_META_RAIL_CURVED_ZP_XP;
			else if (Neighbors[3] && Neighbors[1] && CanThisRailCurve()) return E_META_RAIL_CURVED_ZP_XM;
			else if (Neighbors[2] && Neighbors[0] && CanThisRailCurve()) return E_META_RAIL_CURVED_ZM_XP;
			else if (Neighbors[2] && Neighbors[1] && CanThisRailCurve()) return E_META_RAIL_CURVED_ZM_XM;
			else if (Neighbors[7] && Neighbors[2]) return E_META_RAIL_ASCEND_ZP;
			else if (Neighbors[3] && Neighbors[6]) return E_META_RAIL_ASCEND_ZM;
			else if (Neighbors[5] && Neighbors[0]) return E_META_RAIL_ASCEND_XM;
			else if (Neighbors[4] && Neighbors[1]) return E_META_RAIL_ASCEND_XP;
			else if (Neighbors[0] && Neighbors[1]) return E_META_RAIL_XM_XP;
			else if (Neighbors[2] && Neighbors[3]) return E_META_RAIL_ZM_ZP;

			if (CanThisRailCurve())
			{
				ASSERT(!"Weird neighbor count");
			}
		}
		return Meta;
	}


	inline bool CanThisRailCurve(void)
	{
		return m_BlockType == E_BLOCK_RAIL;
	}


	bool IsUnstable(cChunkInterface & a_ChunkInterface, int a_BlockX, int a_BlockY, int a_BlockZ)
	{
		if (!IsBlockRail(a_ChunkInterface.GetBlock(a_BlockX, a_BlockY, a_BlockZ)))
		{
			return false;
		}
		NIBBLETYPE Meta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY, a_BlockZ);
		switch (Meta)
		{
			case E_META_RAIL_ZM_ZP:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_NORTH, E_PURE_DOWN) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_SOUTH, E_PURE_DOWN)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_XM_XP:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_EAST, E_PURE_DOWN) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_WEST, E_PURE_DOWN)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_ASCEND_XP:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_EAST) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_WEST)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_ASCEND_XM:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_EAST) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_WEST)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_ASCEND_ZM:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_NORTH) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_SOUTH)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_ASCEND_ZP:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_NORTH) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY + 1, a_BlockZ, BLOCK_FACE_SOUTH)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_CURVED_ZP_XP:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_SOUTH) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_EAST)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_CURVED_ZP_XM:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_SOUTH) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_WEST)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_CURVED_ZM_XM:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_NORTH) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_WEST)
				)
				{
					return true;
				}
				break;
			}
			
			case E_META_RAIL_CURVED_ZM_XP:
			{
				if (
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_NORTH) || 
					IsNotConnected(a_ChunkInterface, a_BlockX, a_BlockY, a_BlockZ, BLOCK_FACE_EAST)
				)
				{
					return true;
				}
				break;
			}
		}
		return false;
	}


	bool IsNotConnected(cChunkInterface  & a_ChunkInterface, int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_BlockFace, char a_Pure = 0)
	{
		AddFaceDirection(a_BlockX, a_BlockY, a_BlockZ, a_BlockFace, false);
		NIBBLETYPE Meta;
		if (!IsBlockRail(a_ChunkInterface.GetBlock(a_BlockX, a_BlockY, a_BlockZ)))
		{
			if (!IsBlockRail(a_ChunkInterface.GetBlock(a_BlockX, a_BlockY + 1, a_BlockZ)) || (a_Pure != E_PURE_UPDOWN))
			{
				if (!IsBlockRail(a_ChunkInterface.GetBlock(a_BlockX, a_BlockY - 1, a_BlockZ)) || (a_Pure == E_PURE_NONE))
				{
					return true;
				}
				else
				{
					Meta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY - 1, a_BlockZ);
				}
			}
			else
			{
				Meta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY + 1, a_BlockZ);
			}
		}
		else
		{
			Meta = a_ChunkInterface.GetBlockMeta(a_BlockX, a_BlockY, a_BlockZ);
		}
		
		switch (a_BlockFace)
		{
			case BLOCK_FACE_NORTH:
			{
				if (
					(Meta == E_META_RAIL_ZM_ZP) ||
					(Meta == E_META_RAIL_ASCEND_ZM) ||
					(Meta == E_META_RAIL_ASCEND_ZP) ||
					(Meta == E_META_RAIL_CURVED_ZP_XP) ||
					(Meta == E_META_RAIL_CURVED_ZP_XM)
				)
				{
					return false;
				}
				break;
			}
			
			case BLOCK_FACE_SOUTH:
			{
				if (
					(Meta == E_META_RAIL_ZM_ZP) ||
					(Meta == E_META_RAIL_ASCEND_ZM) ||
					(Meta == E_META_RAIL_ASCEND_ZP) ||
					(Meta == E_META_RAIL_CURVED_ZM_XP) ||
					(Meta == E_META_RAIL_CURVED_ZM_XM)
				)
				{
					return false;
				}
				break;
			}
			
			case BLOCK_FACE_EAST:
			{
				if (
					(Meta == E_META_RAIL_XM_XP) ||
					(Meta == E_META_RAIL_ASCEND_XP) ||
					(Meta == E_META_RAIL_ASCEND_XM) ||
					(Meta == E_META_RAIL_CURVED_ZP_XM) ||
					(Meta == E_META_RAIL_CURVED_ZM_XM)
				)
				{
					return false;
				}
				break;
			}
			case BLOCK_FACE_WEST:
			{
				if (
					(Meta == E_META_RAIL_XM_XP) ||
					(Meta == E_META_RAIL_ASCEND_XP) ||
					(Meta == E_META_RAIL_ASCEND_XM) ||
					(Meta == E_META_RAIL_CURVED_ZP_XP) ||
					(Meta == E_META_RAIL_CURVED_ZM_XP)
				)
				{
					return false;
				}
				break;
			}
		}
		return true;
	}
} ;




