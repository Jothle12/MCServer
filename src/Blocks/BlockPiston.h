
#pragma once

#include "BlockHandler.h"
#include "../Defines.h"





class cBlockPistonHandler :
	public cBlockHandler
{
public:
	cBlockPistonHandler(BLOCKTYPE a_BlockType);
	
	virtual void OnDestroyed(cChunkInterface & a_ChunkInterface, cWorldInterface & a_WorldInterface, int a_BlockX, int a_BlockY, int a_BlockZ) override;

	virtual bool GetPlacementBlockTypeMeta(
		cChunkInterface & a_ChunkInterface, cPlayer * a_Player,
		int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_BlockFace, 
		int a_CursorX, int a_CursorY, int a_CursorZ,
		BLOCKTYPE & a_BlockType, NIBBLETYPE & a_BlockMeta
	) override;

	
	static NIBBLETYPE RotationPitchToMetaData(double a_Rotation, double a_Pitch)
	{
		if (a_Pitch >= 50)
		{
			return 0x1;
		}
		else if (a_Pitch <= -50)
		{
			return 0x0;
		}
		else
		{
			a_Rotation += 90 + 45;  // So its not aligned with axis

			if (a_Rotation > 360)
			{
				a_Rotation -= 360;
			}
			if ((a_Rotation >= 0) && (a_Rotation < 90))
			{
				return 0x4;
			}
			else if ((a_Rotation >= 180) && (a_Rotation < 270))
			{
				return 0x5;
			}
			else if ((a_Rotation >= 90) && (a_Rotation < 180))
			{
				return 0x2;
			}
			else
			{
				return 0x3;
			}
		}
	}

	static eBlockFace MetaDataToDirection(NIBBLETYPE a_MetaData)
	{
		switch (a_MetaData)
		{
			//case -1:  return BLOCK_FACE_NONE; //can never happen as metadata is unsigned
			case 0x0: return BLOCK_FACE_YM;
			case 0x1: return BLOCK_FACE_YP;
			case 0x2: return BLOCK_FACE_ZM;
			case 0x3: return BLOCK_FACE_ZP;
			case 0x4: return BLOCK_FACE_XM;
			case 0x5: return BLOCK_FACE_XP;
			default:
			{
				ASSERT(!"Invalid Metadata");
				return BLOCK_FACE_NONE;
			}
		}
	}

	//
	void ExtendPiston( int, int, int );
	void RetractPiston( int, int, int );
	
	/// Returns true if the piston (specified by blocktype) is a sticky piston
	static bool IsSticky(BLOCKTYPE a_BlockType);
	
	/// Returns true if the piston (with the specified meta) is extended
	static bool IsExtended(NIBBLETYPE a_PistonMeta);
	
	/// Returns true if the specified block can be pushed by a piston (and left intact)
	static bool CanPush(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta);
	
	/// Returns true if the specified block can be pushed by a piston and broken / replaced
	static bool CanBreakPush(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta);
	
	/// Returns true if the specified block can be pulled by a sticky piston
	static bool CanPull(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta);

} ;





class cBlockPistonHeadHandler :
	public cBlockHandler
{
	typedef cBlockHandler super;
	
public:
	cBlockPistonHeadHandler(void);
	
	virtual void OnDestroyedByPlayer(cChunkInterface & a_ChunkInterface, cWorldInterface & a_WorldInterface, cPlayer * a_Player, int a_BlockX, int a_BlockY, int a_BlockZ) override;

	virtual void ConvertToPickups(cItems & a_Pickups, NIBBLETYPE a_BlockMeta) override
	{
		// No pickups
		// Also with 1.7, the item forms of these technical blocks have been removed, so giving someone this will crash their client...
	}
} ;




