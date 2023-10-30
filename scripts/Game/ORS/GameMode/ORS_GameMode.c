//------------------------------------------------------------------------------------------------
class ORS_GameModeClass : SCR_GameModeCampaignClass
{
};

//------------------------------------------------------------------------------------------------
class ORS_GameMode : SCR_GameModeCampaign
{
	[Attribute(desc: "Names of the main base entity", category: "Operation Red Star")]
	protected string m_sMainBaseEntityName;
	protected vector m_vMainBasePos;
	
	[Attribute(desc: "Names of objective area entities", category: "Operation Red Star")]
	protected ref array<string> m_aObjectiveAreaNames;
	protected ref SCR_SortedArray<ORS_ObjectiveArea> m_aObjectiveAreas = new SCR_SortedArray<ORS_ObjectiveArea>();
	
	[RplProp(onRplName: "OnObjectiveAreaChangedProxy")]
	protected RplId m_iCurrentObjectiveAreaId = Replication.INVALID_ID;
	protected ORS_ObjectiveArea m_pCurrentObjectiveArea;
	
	void ORS_GameMode(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;
		
		// Get position of the main base
		IEntity mainBaseEntity = GetGame().GetWorld().FindEntityByName(m_sMainBaseEntityName);
		if (!mainBaseEntity)
			return;
		
		m_vMainBasePos = mainBaseEntity.GetOrigin();
		
		// Find and store all objectivess sorted by distance from the main base
		foreach (string areaName : m_aObjectiveAreaNames)
		{
			ORS_ObjectiveArea area = ORS_ObjectiveArea.Cast(GetGame().GetWorld().FindEntityByName(areaName));
			if (!area)
				continue;
			
			int order = vector.DistanceSqXZ(m_vMainBasePos, area.GetOrigin());
			m_aObjectiveAreas.Insert(order, area);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when RplId of current objective area is updated
	void OnObjectiveAreaChangedProxy()
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_iCurrentObjectiveAreaId));
		if (!rpl)
			return;
		
		m_pCurrentObjectiveArea = ORS_ObjectiveArea.Cast(rpl.GetEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveArea GetCurrentObjectiveArea()
	{
		return m_pCurrentObjectiveArea;
	}
	
	//------------------------------------------------------------------------------------------------
	void GenerateNextObjective()
	{
		// Clean up previous objective
		if (m_pCurrentObjectiveArea)
			m_pCurrentObjectiveArea.ScheduleCleanUp();
		
		// No more objective left => End game mode
		if (m_aObjectiveAreas.IsEmpty())
			EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.COMBATPATROL_VICTORY));
		
		// Generate next objective
		m_pCurrentObjectiveArea = m_aObjectiveAreas.Get(0);
		m_aObjectiveAreas.Remove(0);
		m_pCurrentObjectiveArea.Init();
		
		// Update objective area on clients
		RplComponent rpl = RplComponent.Cast(m_pCurrentObjectiveArea.FindComponent(RplComponent));
		if (!rpl)
			return;
		
		m_iCurrentObjectiveAreaId = rpl.Id();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnGameStart()
	{
		super.OnGameStart();
		
		if (!Replication.IsServer())
			return;
		
		GenerateNextObjective();
	}
	
	//------------------------------------------------------------------------------------------------
	void ScheduleTaskDefendHQ()
	{
	}
}
