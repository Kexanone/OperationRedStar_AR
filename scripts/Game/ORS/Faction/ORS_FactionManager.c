//------------------------------------------------------------------------------------------------
class ORS_FactionManagerClass: KSC_FactionManagerClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_FactionManager : KSC_FactionManager
{
	[RplProp(onRplName: "OnPlayerFactionChanged")]
	protected int m_iPlayerFactionIdx = -1;
	protected SCR_Faction m_pPlayerFaction;
	
	[RplProp(onRplName: "OnEnemyFactionChanged")]
	protected int m_iEnemyFactionIdx = -1;
	protected SCR_Faction m_pEnemyFaction;
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;

		ORS_MissionHeader missionHeader = ORS_MissionHeader.GetInstance();
		
		m_pPlayerFaction = SCR_Faction.Cast(GetFactionByKey(missionHeader.m_sPlayerFactionKey));
		if (!m_pPlayerFaction)
		{
			PrintFormat("ORS_FactionManager.EOnInit: Failed to find faction %1", missionHeader.m_sPlayerFactionKey, level: LogLevel.ERROR);
			return;
		}
		
		m_iPlayerFactionIdx = GetFactionIndex(m_pPlayerFaction);
		
		m_pPlayerFaction.SetIsPlayable(true);
		m_pEnemyFaction = SCR_Faction.Cast(GetFactionByKey(missionHeader.m_sEnemyFactionKey));
		if (!m_pEnemyFaction)
		{
			PrintFormat("ORS_FactionManager.EOnInit: Failed to find faction %1", missionHeader.m_sEnemyFactionKey, level: LogLevel.ERROR);
			return;
		}
		
		m_iEnemyFactionIdx = GetFactionIndex(m_pEnemyFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionChanged()
	{
		m_pPlayerFaction = SCR_Faction.Cast(GetFactionByIndex(m_iPlayerFactionIdx));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnemyFactionChanged()
	{
		m_pEnemyFaction = SCR_Faction.Cast(GetFactionByIndex(m_iEnemyFactionIdx));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Faction GetPlayerFaction()
	{
		return m_pPlayerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Faction GetEnemyFaction()
	{
		return m_pEnemyFaction;
	}
}
