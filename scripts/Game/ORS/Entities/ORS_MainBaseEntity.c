//------------------------------------------------------------------------------------------------
class ORS_MainBaseEntityClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_MainBaseEntity : GenericEntity
{
	protected static ref array<ORS_MainBaseEntity> s_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	static array<ORS_MainBaseEntity> GetInstances()
	{
		return s_aEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	void ORS_MainBaseEntity(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		s_aEntities.Insert(this);
		
		if (Replication.IsServer())
			SetEventMask(EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			SetFaction(factionManager.GetPlayerFaction());
		
		CreateMapMarker();
	}

	//------------------------------------------------------------------------------------------------
	void ~ORS_MainBaseEntity()
	{
		s_aEntities.RemoveItem(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFaction(SCR_Faction faction)
	{
		IEntity child = GetChildren();
		while (child)
		{
			SCR_FactionAffiliationComponent.SetFaction(child, faction);
			
			SCR_EditableSpawnPointComponent editableSpawn = SCR_EditableSpawnPointComponent.Cast(child.FindComponent(SCR_EditableSpawnPointComponent));
			if (editableSpawn)
				editableSpawn.UpdateNearestLocation(child.GetOrigin());
			
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateMapMarker()
	{
		SCR_MapMarkerManagerComponent markerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!markerManager)
			return;
		
		
		
		SCR_MapMarkerBase marker = markerManager.PrepareMilitaryMarker(GetMilitarySymbolIdentity(), EMilitarySymbolDimension.INSTALLATION, EMilitarySymbolIcon.RESPAWN);
		vector pos = GetOrigin();
		marker.SetWorldPos(pos[0], pos[2]);
		markerManager.InsertStaticMarker(marker, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected EMilitarySymbolIdentity GetMilitarySymbolIdentity()
	{
		EMilitarySymbolIdentity identity = EMilitarySymbolIdentity.UNKNOWN;
		
		SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
		if (!core)
			return identity;
		
		SCR_MilitarySymbolRuleSet ruleSet = core.GetSymbolRuleSet();
		if (!ruleSet)
			return identity;

		ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return identity;
		
		SCR_MilitarySymbol symbol = new SCR_MilitarySymbol();
		ruleSet.UpdateSymbol(symbol, factionManager.GetPlayerFaction());
		return symbol.GetIdentity();
	}
}
