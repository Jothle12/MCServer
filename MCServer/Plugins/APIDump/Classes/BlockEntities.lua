return
{
	cBlockEntity =
	{
		Desc = [[
			Block entities are simply blocks in the world that have persistent data, such as the text for a sign
			or contents of a chest. All block entities are also saved in the chunk data of the chunk they reside in.
			The cBlockEntity class acts as a common ancestor for all the individual block entities.
		]],

		Functions =
		{
			GetBlockType = { Params = "", Return = "BLOCKTYPE", Notes = "Returns the blocktype which is represented by this blockentity. This is the primary means of type-identification" },
			GetChunkX    = { Params = "", Return = "number", Notes = "Returns the chunk X-coord of the block entity's chunk" },
			GetChunkZ    = { Params = "", Return = "number", Notes = "Returns the chunk Z-coord of the block entity's chunk" },
			GetPosX      = { Params = "", Return = "number", Notes = "Returns the block X-coord of the block entity's block" },
			GetPosY      = { Params = "", Return = "number", Notes = "Returns the block Y-coord of the block entity's block" },
			GetPosZ      = { Params = "", Return = "number", Notes = "Returns the block Z-coord of the block entity's block" },
			GetRelX      = { Params = "", Return = "number", Notes = "Returns the relative X coord of the block entity's block within the chunk" },
			GetRelZ      = { Params = "", Return = "number", Notes = "Returns the relative Z coord of the block entity's block within the chunk" },
			GetWorld     = { Params = "", Return = "{{cWorld|cWorld}}", Notes = "Returns the world to which the block entity belongs" },
		},
	},

	cBlockEntityWithItems =
	{
		Desc = [[
			This class is a common ancestor for all {{cBlockEntity|block entities}} that provide item storage.
			Internally, the object has a {{cItemGrid|cItemGrid}} object for storing the items; this ItemGrid is
			accessible through the API. The storage is a grid of items, items in it can be addressed either by a slot
			number, or by XY coords within the grid. If a UI window is opened for this block entity, the item storage
			is monitored for changes and the changes are immediately sent to clients of the UI window.
		]],

		Inherits = "cBlockEntity",

		Functions =
		{
			GetContents = { Params = "", Return = "{{cItemGrid|cItemGrid}}", Notes = "Returns the cItemGrid object representing the items stored within this block entity" },
			GetSlot =
			{
				{ Params = "SlotNum", Return = "{{cItem|cItem}}", Notes = "Returns the cItem for the specified slot number. Returns nil for invalid slot numbers" },
				{ Params = "X, Y", Return = "{{cItem|cItem}}", Notes = "Returns the cItem for the specified slot coords. Returns nil for invalid slot coords" },
			},
			SetSlot =
			{
				{ Params = "SlotNum, {{cItem|cItem}}", Return = "", Notes = "Sets the cItem for the specified slot number. Ignored if invalid slot number" },
				{ Params = "X, Y, {{cItem|cItem}}", Return = "", Notes = "Sets the cItem for the specified slot coords. Ignored if invalid slot coords" },
			},
		},
	},

	cChestEntity =
	{
		Desc = [[
			A chest entity is a {{cBlockEntityWithItems|cBlockEntityWithItems}} descendant that represents a chest
			in the world. Note that doublechests consist of two separate cChestEntity objects, they do not collaborate
			in any way.</p>
			<p>
			To manipulate a chest already in the game, you need to use {{cWorld}}'s callback mechanism with
			either DoWithChestAt() or ForEachChestInChunk() function. See the code example below
		]],

		Inherits = "cBlockEntityWithItems",

		Constants =
		{
			ContentsHeight = { Notes = "Height of the contents' {{cItemGrid|ItemGrid}}, as required by the parent class, {{cBlockEntityWithItems}}" },
			ContentsWidth = { Notes = "Width of the contents' {{cItemGrid|ItemGrid}}, as required by the parent class, {{cBlockEntityWithItems}}" },
		},
		AdditionalInfo =
		{
			{
				Header = "Code example",
				Contents = [[
					The following example code sets the top-left item of each chest in the same chunk as Player to
					64 * diamond:
<pre class="prettyprint lang-lua">
-- Player is a {{cPlayer}} object instance
local World = Player:GetWorld();
World:ForEachChestInChunk(Player:GetChunkX(), Player:GetChunkZ(),
	function (ChestEntity)
		ChestEntity:SetSlot(0, 0, cItem(E_ITEM_DIAMOND, 64));
	end
);
</pre>
				]],
			},
		},  -- AdditionalInfo
	},

	cDispenserEntity =
	{
		Desc = [[
			This class represents a dispenser block entity in the world. Most of this block entity's
			functionality is implemented in the {{cDropSpenserEntity|cDropSpenserEntity}} class that represents
			the behavior common with a {{cDropperEntity|dropper}} entity.
		]],
		Inherits = "cDropSpenserEntity",
	},

	cDropperEntity =
	{
		Desc = [[
			This class represents a dropper block entity in the world. Most of this block entity's functionality
			is implemented in the {{cDropSpenserEntity|cDropSpenserEntity}} class that represents the behavior
			common with the {{cDispenserEntity|dispenser}} entity.</p>
			<p>
			An object of this class can be created from scratch when generating chunks ({{OnChunkGenerated|OnChunkGenerated}} and {{OnChunkGenerating|OnChunkGenerating}} hooks).
		]],
		Inherits = "cDropSpenserEntity",
	},  -- cDropperEntity

	cDropSpenserEntity =
	{
		Desc = [[
			This is a class that implements behavior common to both {{cDispenserEntity|dispensers}} and {{cDropperEntity|droppers}}.
		]],
		Functions =
		{
			Activate = { Params = "", Return = "", Notes = "Sets the block entity to dropspense an item in the next tick" },
			AddDropSpenserDir = { Params = "BlockX, BlockY, BlockZ, BlockMeta", Return = "BlockX, BlockY, BlockZ", Notes = "Adjusts the block coords to where the dropspenser items materialize" },
			SetRedstonePower = { Params = "IsPowered", Return = "", Notes = "Sets the redstone status of the dropspenser. If the redstone power goes from off to on, the dropspenser will be activated" },
		},
		Constants =
		{
			ContentsWidth = { Notes = "Width (X) of the {{cItemGrid}} representing the contents" },
			ContentsHeight = { Notes = "Height (Y) of the {{cItemGrid}} representing the contents" },
		},
		Inherits = "cBlockEntityWithItems";
	},  -- cDropSpenserEntity

	cFurnaceEntity =
	{
		Desc = [[
			This class represents a furnace block entity in the world.</p>
			<p>
			See also {{cRoot}}'s GetFurnaceRecipe() and GetFurnaceFuelBurnTime() functions
		]],
		Functions =
		{
			GetCookTimeLeft = { Params = "", Return = "number", Notes = "Returns the time until the current item finishes cooking, in ticks" },
			GetFuelBurnTimeLeft = { Params = "", Return = "number", Notes = "Returns the time until the current fuel is depleted, in ticks" },
			GetFuelSlot = { Params = "", Return = "{{cItem|cItem}}", Notes = "Returns the item in the fuel slot" },
			GetInputSlot = { Params = "", Return = "{{cItem|cItem}}", Notes = "Returns the item in the input slot" },
			GetOutputSlot = { Params = "", Return = "{{cItem|cItem}}", Notes = "Returns the item in the output slot" },
			GetTimeCooked = { Params = "", Return = "number", Notes = "Returns the time that the current item has been cooking, in ticks" },
			HasFuelTimeLeft = { Params = "", Return = "bool", Notes = "Returns true if there's time before the current fuel is depleted" },
			SetFuelSlot = { Params = "{{cItem|cItem}}", Return = "", Notes = "Sets the item in the fuel slot" },
			SetInputSlot = { Params = "{{cItem|cItem}}", Return = "", Notes = "Sets the item in the input slot" },
			SetOutputSlot = { Params = "{{cItem|cItem}}", Return = "", Notes = "Sets the item in the output slot" },
		},
		Constants =
		{
			fsInput = { Notes = "Index of the input slot" },
			fsFuel = { Notes = "Index of the fuel slot" },
			fsOutput = { Notes = "Index of the output slot" },
			ContentsWidth = { Notes = "Width (X) of the {{cItemGrid|cItemGrid}} representing the contents" },
			ContentsHeight = { Notes = "Height (Y) of the {{cItemGrid|cItemGrid}} representing the contents" },
		},
		ConstantGroups =
		{
			SlotIndices =
			{
				Include = "fs.*",
				TextBefore = "When using the GetSlot() or SetSlot() function, use these constants for slot index:",
			},
		},
		Inherits = "cBlockEntityWithItems"
	},  -- cFurnaceEntity

	cHopperEntity =
	{
		Desc = [[
			This class represents a hopper block entity in the world.
		]],
		Functions =
		{
			GetOutputBlockPos = { Params = "BlockMeta", Return = "bool, BlockX, BlockY, BlockZ", Notes = "Returns whether the hopper is attached, and if so, the block coords of the block receiving the output items, based on the given meta." },
		},
		Constants =
		{
			ContentsHeight = { Notes = "Height (Y) of the internal {{cItemGrid}} representing the hopper contents." },
			ContentsWidth = { Notes = "Width (X) of the internal {{cItemGrid}} representing the hopper contents." },
			TICKS_PER_TRANSFER = { Notes = "Number of ticks between when the hopper transfers items." },
		},
		Inherits = "cBlockEntityWithItems",
	},  -- cHopperEntity

	cJukeboxEntity =
	{
		Desc = [[
			This class represents a jukebox in the world. It can play the records, either when the
			{{cPlayer|player}} uses the record on the jukebox, or when a plugin instructs it to play.
		]],
		Inherits = "cBlockEntity",
		Functions =
		{
			EjectRecord = { Params = "", Return = "bool", Notes = "Ejects the current record as a {{cPickup|pickup}}. No action if there's no current record. To remove record without generating the pickup, use SetRecord(0). Returns true if pickup ejected." },
			GetRecord = { Params = "", Return = "number", Notes = "Returns the record currently present. Zero for no record, E_ITEM_*_DISC for records." },
			IsPlayingRecord = { Params = "", Return = "bool", Notes = "Returns true if the jukebox is playing a record." },
			IsRecordItem = { Params = "ItemType", Return = "bool", Notes = "Returns true if the specified item is a record that can be played." },
			PlayRecord = { Params = "RecordItemType", Return = "bool", Notes = "Plays the specified Record. Return false if the parameter isn't a playable Record (E_ITEM_XXX_DISC). If there is a record already playing, ejects it first." },
			SetRecord = { Params = "number", Return = "", Notes = "Sets the currently present record. Use zero for no record, or E_ITEM_*_DISC for records." },
		},
	},  -- cJukeboxEntity

	cNoteEntity =
	{
		Desc = [[
			This class represents a note block entity in the world. It takes care of the note block's pitch,
			and also can play the sound, either when the {{cPlayer|player}} right-clicks it, redstone activates
			it, or upon a plugin's request.</p>
			<p>
			The pitch is stored as an integer between 0 and 24.
		]],
		Functions =
		{
			GetPitch = { Params = "", Return = "number", Notes = "Returns the current pitch set for the block" },
			IncrementPitch = { Params = "", Return = "", Notes = "Adds 1 to the current pitch. Wraps around to 0 when the pitch cannot go any higher." },
			MakeSound = { Params = "", Return = "", Notes = "Plays the sound for all {{cClientHandle|clients}} near this block." },
			SetPitch = { Params = "Pitch", Return = "", Notes = "Sets a new pitch for the block." },
		},
		Inherits = "cBlockEntity",
	},  -- cNoteEntity

	cSignEntity =
	{
		Desc = [[
			A sign entity represents a sign in the world. This class is only used when generating chunks, so
			that the plugins may generate signs within new chunks. See the code example in {{cChunkDesc}}.
		]],
		Functions =
		{
			GetLine = { Params = "LineIndex", Return = "string", Notes = "Returns the specified line. LineIndex is expected between 0 and 3. Returns empty string and logs to server console when LineIndex is invalid." },
			SetLine = { Params = "LineIndex, LineText", Return = "", Notes = "Sets the specified line. LineIndex is expected between 0 and 3. Logs to server console when LineIndex is invalid." },
			SetLines = { Params = "Line1, Line2, Line3, Line4", Return = "", Notes = "Sets all the sign's lines at once." },
		},
		Inherits = "cBlockEntity";
	},  -- cSignEntity
}




