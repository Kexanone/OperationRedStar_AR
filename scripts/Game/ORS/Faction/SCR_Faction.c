//------------------------------------------------------------------------------------------------
//! Insert shared ORS group configs
modded class SCR_Faction : ScriptedFaction
{
	//------------------------------------------------------------------------------------------------
	override void GetGroupRolePresetConfigs(notnull array<SCR_GroupRolePresetConfig> groupArray)
	{
		super.GetGroupRolePresetConfigs(groupArray);
		
		ORS_GroupsManagerComponent groupManager = ORS_GroupsManagerComponent.Cast(SCR_GroupsManagerComponent.GetInstance());
		if (groupManager)
			groupManager.GetGroupRolePresetConfigs(groupArray);
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetPredefinedGroups(notnull array<ref SCR_GroupPreset> groupArray)
	{
		super.GetPredefinedGroups(groupArray);
		
		ORS_GroupsManagerComponent groupManager = ORS_GroupsManagerComponent.Cast(SCR_GroupsManagerComponent.GetInstance());
		if (groupManager)
			groupManager.GetPredefinedGroups(groupArray);
	}
}
