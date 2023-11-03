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
	protected RplId m_iCurrentObjectiveAreaRplId = Replication.INVALID_ID;
	protected ORS_ObjectiveArea m_pCurrentObjectiveArea;
	protected int m_iCurrentObjectiveAreaIdx = 0;
	
	protected ref SCR_MissionHeader m_pMissionHeader;
	
	//------------------------------------------------------------------------------------------------
	void ORS_GameMode(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		// Create a mission header for workbench sessions
	#ifdef WORKBENCH
		m_pMissionHeader = new SCR_MissionHeader();
		m_pMissionHeader.m_sSaveFileName = FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
		m_pMissionHeader.m_bIsSavingEnabled = true;
	#else
		m_pMissionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnGameStart()
	{
		super.OnGameStart();
		
		if (!IsMaster())
			return;
		
		LoadSession();
		
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
		
		// Initialize areas;
		for (int i = 0; i < m_aObjectiveAreas.Count(); i++)
		{
			m_aObjectiveAreas[i].Init(i < m_iCurrentObjectiveAreaIdx);
		}
		
		SpawnObjective();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads the state of the previous session
	protected void LoadSession()
	{
		string saveName = m_pMissionHeader.GetSaveFileName();
		
		if (!GetGame().GetSaveManager().HasLatestSave(saveName))
			return;
		
		string fileName;
		GetGame().GetSaveManager().FindLatestSave(saveName, fileName);
		GetGame().GetSaveManager().Load(fileName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads the state of the previous session
	void LoadSessionData(ORS_SessionStruct data)
	{
		m_iCurrentObjectiveAreaIdx = data.GetCurrentObjectiveAreaIdx();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads the state of the previous session
	void SaveSessionData(out ORS_SessionStruct data)
	{
		data.SetCurrentObjectiveAreaIdx(m_iCurrentObjectiveAreaIdx);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delete auto save files when mission is finished
	override void EndGameMode(SCR_GameModeEndData endData)
	{
		super.EndGameMode(endData);
		
		if (!IsMaster())
			return;
		
		GetGame().GetSaveManager().Delete(ESaveType.AUTO);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when RplId of current objective area is updated
	void OnObjectiveAreaChangedProxy()
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_iCurrentObjectiveAreaRplId));
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
	void SpawnObjective()
	{
		// No more objective left => End game mode
		if (m_iCurrentObjectiveAreaIdx >= m_aObjectiveAreas.Count())
			EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.COMBATPATROL_VICTORY));
		
		m_pCurrentObjectiveArea = m_aObjectiveAreas[m_iCurrentObjectiveAreaIdx];
		m_pCurrentObjectiveArea.Spawn();
		
		// Update objective area on clients
		RplComponent rpl = RplComponent.Cast(m_pCurrentObjectiveArea.FindComponent(RplComponent));
		if (!rpl)
			return;
		
		m_iCurrentObjectiveAreaRplId = rpl.Id();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void ScheduleObjectiveCleanUp(int objectiveAreaIdx)
	{
		ORS_ObjectiveArea objectiveArea = m_aObjectiveAreas[objectiveAreaIdx];
		objectiveArea.ScheduleCleanUp();
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnNextObjective()
	{
		// Clean up previous objective
		ScheduleObjectiveCleanUp(m_iCurrentObjectiveAreaIdx);
		
		// Generate next objective
		m_iCurrentObjectiveAreaIdx++;
		SCR_SaveManagerCore.Cast(GetGame().GetSaveManager()).Save(ESaveType.AUTO);
		SpawnObjective();
	}
}
