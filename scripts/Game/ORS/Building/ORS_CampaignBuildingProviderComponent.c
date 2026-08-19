[EntityEditorProps(category: "GameScripted/Building", description: "Component attached to a provider, responsible for basic provider behaviour.")]
class ORS_CampaignBuildingProviderComponentClass : SCR_CampaignBuildingProviderComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! A provider that is always the player's faction
class ORS_CampaignBuildingProviderComponent : SCR_CampaignBuildingProviderComponent
{
	//------------------------------------------------------------------------------------------------
	override protected Faction GetEntityFaction(notnull IEntity ent)
	{
		if (ent != GetOwner())
			return super.GetEntityFaction(ent);
		
		ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return null;
		
		return factionManager.GetPlayerFaction();
	}
}
