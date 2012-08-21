-- Some HTML helper functions
local function HTML_Option( value, text, selected )
	if( selected == true ) then
		return [[<option value="]] .. value .. [[" selected>]] .. text .. [[</option>]]
	else
		return [[<option value="]] .. value .. [[">]] .. text .. [[</option>"]]
	end
end

local function HTML_Select_On_Off( name, defaultValue )
	return [[<select name="]] .. name .. [[">]]
	.. HTML_Option("1", "On",  defaultValue == 1 )
	.. HTML_Option("0", "Off", defaultValue == 0 )
	.. [[</select>]]
end


local function ShowGeneralSettings( Request )
	local Content = ""
	local InfoMsg = nil
	
	local SettingsIni = cIniFile("settings.ini")
	if( SettingsIni:ReadFile() == false ) then
		InfoMsg = "<b style=\"color: red;\">ERROR: Could not read settings.ini!</b>"
	end
	
	if( Request.PostParams["general_submit"] ~= nil ) then
		
		SettingsIni:SetValue("Server", "Description",Request.PostParams["Server_Description"],false )
		if( tonumber( Request.PostParams["Server_MaxPlayers"] ) ~= nil ) then
			SettingsIni:SetValue("Server", "MaxPlayers", Request.PostParams["Server_MaxPlayers"], false )
		end
		if( tonumber( Request.PostParams["Server_Port"] ) ~= nil ) then
			SettingsIni:SetValue("Server", "Port",       Request.PostParams["Server_Port"],       false )
		end
		if( tonumber( Request.PostParams["Authentication_Authenticate"] ) ~= nil ) then
			SettingsIni:SetValue("Authentication", "Authenticate", Request.PostParams["Authentication_Authenticate"], false )
		end
		if( SettingsIni:WriteFile() == false ) then
			InfoMsg =  [[<b style="color: red;">ERROR: Could not write to settings.ini!</b>]]
		else
			InfoMsg = [[<b style="color: green;">INFO: Successfully saved changes to settings.ini</b>]]
		end
	end
	
	
	Content = Content .. [[
	<form method="POST">
	<h4>General Settings</h4>]]
	
	if( InfoMsg ~= nil ) then
		Content = Content .. "<p>" .. InfoMsg .. "</p>"
	end
	Content = Content .. [[
	<table>
	<th colspan="2">Server</th>
	<tr><td style="width: 50%;">Description:</td>
	<td><input type="text" name="Server_Description" value="]] .. SettingsIni:GetValue("Server", "Description") .. [["></td></tr>
	<tr><td>Max Players:</td>
	<td><input type="text" name="Server_MaxPlayers" value="]] .. SettingsIni:GetValue("Server", "MaxPlayers") .. [["></td></tr>
	<tr><td>Port:</td>
	<td><input type="text" name="Server_Port" value="]] .. SettingsIni:GetValue("Server", "Port") .. [["></td></tr>
	</table><br>
	
	<table>
	<th colspan="2">Authentication</th>
	<tr><td style="width: 50%;">Authenticate:</td>
	<td>]] .. HTML_Select_On_Off("Authentication_Authenticate", SettingsIni:GetValueI("Authentication", "Authenticate") ) .. [[</td></tr>
	</table><br>
	
	<input type="submit" value="Save Settings" name="general_submit"> WARNING: Any changes made here might require a server restart in order to be applied!
	</form>]]
	
	return Content
end


local function ShowMonstersSettings( Request )
	local Content = ""
	local InfoMsg = nil
	
	local SettingsIni = cIniFile("settings.ini")
	if( SettingsIni:ReadFile() == false ) then
		InfoMsg = "<b style=\"color: red;\">ERROR: Could not read settings.ini!</b>"
	end
	
	if( Request.PostParams["monsters_submit"] ~= nil ) then
		
		if( tonumber( Request.PostParams["Monsters_AnimalsOn"] ) ~= nil ) then
			SettingsIni:SetValue("Monsters", "AnimalsOn", Request.PostParams["Monsters_AnimalsOn"], false )
		end
		if( tonumber( Request.PostParams["Monsters_AnimalSpawnInterval"] ) ~= nil ) then
			SettingsIni:SetValue("Monsters", "AnimalSpawnInterval", Request.PostParams["Monsters_AnimalSpawnInterval"], false )
		end
		SettingsIni:SetValue("Monsters", "Types", Request.PostParams["Monsters_Types"], false )
		if( SettingsIni:WriteFile() == false ) then
			InfoMsg =  "<b style=\"color: red;\">ERROR: Could not write to settings.ini!</b>"
		else
			InfoMsg = "<b style=\"color: green;\">INFO: Successfully saved changes to settings.ini</b>"
		end
	end
	
	
	Content = Content .. "<form method=\"POST\">"
	
	Content = Content .. "<h4>Monsters Settings</h4>"
	if( InfoMsg ~= nil ) then
		Content = Content .. "<p>" .. InfoMsg .. "</p>"
	end
	
	Content = Content .. [[
	<table>
	<th colspan="2">Monsters</th>
	<tr><td style="width: 50%;">Animals On:</td>
	<td>]] .. HTML_Select_On_Off("Monsters_AnimalsOn", SettingsIni:GetValueI("Monsters", "AnimalsOn") ) .. [[</td></tr>
	<tr><td>Animal Spawn Interval:</td>
	<td><input type="text" name="Monsters_AnimalSpawnInterval" value="]] .. SettingsIni:GetValue("Monsters", "AnimalSpawnInterval") .. [["></td></tr>
	<tr><td>Monster Types:</td>
	<td><input type="text" name="Monsters_Types" value="]] .. SettingsIni:GetValue("Monsters", "Types") .. [[\"></td></tr>
	</table><br>
	<input type="submit" value="Save Settings" name="monsters_submit"> WARNING: Any changes made here might require a server restart in order to be applied!
	</form>]]
	
	return Content
end

local function ShowWorldsSettings( Request )
	return "<p><b>Worlds Settings</b></p>"
end

function HandleRequest_ServerSettings( Request )
	local Content = ""

	Content = Content .. [[
	<p><b>Server Settings</b></p>
	<table>
	<tr>
	<td><a href="?tab=General">General</a></td>
	<td><a href="?tab=Monsters">Monsters</a></td>
	<td><a href="?tab=Worlds">Worlds</a></td>
	</tr>
	</table>
	<br>]]
	
	if( Request.Params["tab"] == "Monsters" ) then
		Content = Content .. ShowMonstersSettings( Request )
	elseif( Request.Params["tab"] == "Worlds" ) then
		Content = Content .. ShowWorldsSettings( Request )
	else
		Content = Content .. ShowGeneralSettings( Request ) -- Default to general settings
	end
	
	return Content
end