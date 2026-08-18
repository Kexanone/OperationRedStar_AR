//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Groups", description: "Player groups manager, attach to game mode entity!.")]
class ORS_GroupsManagerComponentClass : SCR_GroupsManagerComponentClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_GroupsManagerComponent : SCR_GroupsManagerComponent
{
	[Attribute(desc: "Group preset for predefined groups roles applied to all factions")]
	protected ref array<ref SCR_GroupRolePresetConfig> m_aGroupRolePresetConfigs;

	[Attribute(desc: "Group preset for predefined groups applied to all factions")]
	protected ref array<ref SCR_GroupPreset> m_aPredefinedGroups;
	
	//------------------------------------------------------------------------------------------------
	void GetGroupRolePresetConfigs(notnull array<SCR_GroupRolePresetConfig> groupArray)
	{
		foreach (SCR_GroupRolePresetConfig preset : m_aGroupRolePresetConfigs)
		{
			if (preset.IsEnabled())
				groupArray.Insert(preset);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void GetPredefinedGroups(notnull array<ref SCR_GroupPreset> groupArray)
	{
		for (int i = 0, count = m_aPredefinedGroups.Count(); i < count; i++)
		{
			if (m_aPredefinedGroups[i].IsEnabled())
				groupArray.Insert(m_aPredefinedGroups[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disable rank requirement
	override bool HasPlayerRequiredRank(SCR_GroupRolePresetConfig preset, int playerId, bool ignoreGroupRequiredRank)
	{
		return true;
	}
}
