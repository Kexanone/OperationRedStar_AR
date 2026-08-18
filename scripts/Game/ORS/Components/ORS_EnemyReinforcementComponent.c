//------------------------------------------------------------------------------------------------
class ORS_EnemyReinforcementComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Enemy reinfocement data for an ORS_ObjectiveArea
class ORS_EnemyReinforcementComponent : ScriptComponent
{
}

//------------------------------------------------------------------------------------------------
/*
class ORS_ReinforcementComponent : ScriptComponent
{
	protected ref array<AIGroup> m_aManagedGroups = {};
	protected ref array<vector> m_aPositionsToDefend = {};
	protected ref array<ref ORS_Reinforcement_EntityToDefendWrapper> m_aEntitiesToDefend = {};
	protected int m_iTargetAICount;
	
	//------------------------------------------------------------------------------------------------
	void AddEntityToDefend(notnull SCR_DamageManagerComponent damageManager)
	{
		m_aEntitiesToDefend.Insert(new ORS_Reinforcement_EntityToDefendWrapper(this, damageManager));
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntityToDefend(IEntity entity)
	{
		for (int i = m_aEntitiesToDefend.Count() - 1; i > 0; i--)
		{
			if (m_aEntitiesToDefend[i].GetEntity() == entity)
				m_aEntitiesToDefend.RemoveOrdered(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPositionToDefend(vector pos)
	{
		m_aPositionsToDefend.Insert(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemovePositionToDefend(vector pos)
	{
		m_aPositionsToDefend.RemoveItem(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddGroupToManage(AIGroup group)
	{
		m_aManagedGroups.Insert(group);
	}
		
	//------------------------------------------------------------------------------------------------
	array<AIGroup> GetManagedGroups()
	{
		return m_aManagedGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	int ComputeCurrentAICount()
	{
		int totalCount = 0;
		
		foreach (AIGroup group : m_aManagedGroups)
		{
			if (!group)
				continue;
			
			array<AIAgent> agents = {};
			group.GetAgents(agents);
			totalCount += agents.Count();
		}
		
		return totalCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetAICount(int count)
	{
		m_iTargetAICount = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTargetAICount()
	{
		return m_iTargetAICount;
	}
}

//------------------------------------------------------------------------------------------------
class ORS_Reinforcement_Task : Managed
{
	protected vector m_vPos;
	
	//------------------------------------------------------------------------------------------------
	void ORS_Reinforcement_Task(vector pos)
	{
		m_vPos = pos;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetPos()
	{
		return m_vPos;
	}
}

//------------------------------------------------------------------------------------------------
class ORS_Reinforcement_TaskDefend : ORS_Reinforcement_Task
{	

}

//------------------------------------------------------------------------------------------------
class ORS_Reinforcement_TaskAttack : ORS_Reinforcement_Task
{
}

//------------------------------------------------------------------------------------------------
class ORS_Reinforcement_EntityToDefendWrapper : Managed
{
	protected IEntity m_pEntity;
	protected SCR_DamageManagerComponent m_pDamageManager;
	protected ORS_ReinforcementComponent m_pManager;
			
	//------------------------------------------------------------------------------------------------
	void ORS_Reinforcement_EntityToDefendWrapper(notnull ORS_ReinforcementComponent manager, notnull SCR_DamageManagerComponent damageManager)
	{
		m_pManager = manager;
		m_pEntity = damageManager.GetOwner().GetRootParent();
		m_pDamageManager = damageManager;
		m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDamageStateChanged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
				
		m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
		m_pManager.RemoveEntityToDefend(m_pEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		return m_pEntity;
	}
}
*/

//------------------------------------------------------------------------------------------------
class ORS_HarassementData : Managed
{
	protected ref array<AIGroup> m_aEnemyGroups;
	protected AIWaypoint m_pWaypoint;
	
	//------------------------------------------------------------------------------------------------
	int ComputeEnemyCount()
	{
		int count = 0;
		
		foreach (AIGroup group : m_aEnemyGroups)
		{
			if (!group)
				continue;
			
			array<AIAgent> agents = {};
			group.GetAgents(agents);			
			count += agents.Count();
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetPos(vector pos)
	{
		m_pWaypoint.SetOrigin(pos);
	}
}

/*
//------------------------------------------------------------------------------------------------
modded class SCR_AIGroup : ChimeraAIGroup
{
	override void OnLODChanged(int oldLOD, int newLOD)
	{
		super.OnLODChanged(oldLOD, newLOD);
		Print(newLOD);
	}
}
*/
