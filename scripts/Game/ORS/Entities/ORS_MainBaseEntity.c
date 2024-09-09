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
		ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			SetFaction(factionManager.GetPlayerFaction());
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
}
