enum ORS_EAiManagementMode
{
	DEFEND,
	ATTACK
}

//------------------------------------------------------------------------------------------------
class SCR_ComparePositionsToDefend : SCR_SortCompare<ref Tuple3<float, int, vector>>
{
	override static int Compare(Tuple3<float, int, vector> left, Tuple3<float, int, vector> right)
	{
		if (left.param1 == right.param1)
			return left.param2 < right.param2;
		else
			return left.param1 < right.param1;
	}
}

//------------------------------------------------------------------------------------------------
class ORS_AiManagerComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_AiManagerComponent : ScriptComponent
{
	[Attribute(defvalue: "60", desc: "Timeout for the handler in seconds")]
	protected float m_fHandlerTimeout;
	
	[Attribute(defvalue: "44", desc: "Target AI count for 20 players. The effective total count will be interpolated between 33% for 0 players and 100% for 20 players")]
	protected int m_iTargetTotalAiCount;
	
	[Attribute(defvalue: "200", desc: "AI cannot spawn below this distance to a player (in meters)")]
	protected int m_fHardBlockSpawnRadius;
	
	[Attribute(defvalue: "1000", desc: "If players are below this distance to a potential AI spawn position, a visibility check is done (in meters)")]
	protected int m_fSoftBlockSpawnRadius;
	
	protected ORS_ObjectiveArea m_pObjectiveArea;
	protected ref array<AIGroup> m_aManagedGroups = {};
	
	protected int m_aQueriedFriendlyUnitsCount = 0;
	protected int m_aQueriedEnemyUnitsCount = 0;
	protected ORS_EAiManagementMode m_iMode = ORS_EAiManagementMode.DEFEND;
	protected AIWaypoint m_pAttackWp;
	
	void AddGroup(AIGroup group)
	{
		m_aManagedGroups.Insert(group);

	}
	
	void SetMode(ORS_EAiManagementMode mode)
	{
		m_iMode = mode;
		
		if (mode == ORS_EAiManagementMode.ATTACK)
		{
			vector pos = m_pObjectiveArea.GetPositionToAttack();
			m_pAttackWp = COE_GameTools.SpawnWaypointPrefab("{1B0E3436C30FA211}Prefabs/AI/Waypoints/AIWaypoint_Attack.et", pos);
			
			foreach (AIGroup group : m_aManagedGroups)
			{
				if (!group)
					continue;
				
				array<AIWaypoint> waypoints = {};
				group.GetWaypoints(waypoints);
				foreach (AIWaypoint waypoint : waypoints)
					group.RemoveWaypoint(waypoint);
				
				group.AddWaypoint(m_pAttackWp);
			};
		};
	}
	
	void Start()
	{
		m_pObjectiveArea = ORS_ObjectiveArea.Cast(GetOwner());
		GetGame().GetCallqueue().CallLater(Handler, m_fHandlerTimeout*1000, true);
	}
	
	void Stop()
	{
		GetGame().GetCallqueue().Remove(Handler);
	}
	
	void Handler()
	{
		int currentTotalAiCount = 0;
		array<AIAgent> orphans = {};
		
		foreach (AIGroup group : m_aManagedGroups)
		{
			if (!group)
				continue;
			
			array<AIAgent> agents = {};
			group.GetAgents(agents);
			int count = agents.Count();
			
			if (count == 1)
				orphans.InsertAll(agents);
			
			currentTotalAiCount += count;
		};
		
		// Effective total count is interpolated between 33% (0 players) and 100% (20 players)
		int effectiveTargetTotalAiCount = (1.0/3)*m_iTargetTotalAiCount + (2.0/3)*Math.Lerp(0, m_iTargetTotalAiCount, GetGame().GetPlayerManager().GetPlayerCount() / 20);
			
		
		int countDiff = effectiveTargetTotalAiCount - currentTotalAiCount;
		if (countDiff < 3)
			return;
		
		AIGroup group;
		
		for (int i = 0;  i < Math.Ceil((float)countDiff/4); i++)
		{		
			switch (m_iMode)
			{
				case ORS_EAiManagementMode.DEFEND: { group = SpawnDefensiveReinforcement(); break; };
				case ORS_EAiManagementMode.ATTACK: { group = SpawnAttackingReinforcement(); break; };
			};
		};
		
		// Orphaned units hould join reinforcements
		foreach (AIAgent orphan : orphans)
		{
			group.AddAgent(orphan);
		};
	}
	
	AIGroup SpawnAttackingReinforcement()
	{
		vector spawnPos = GetSpawnPosition(m_pAttackWp.GetOrigin());
		if (spawnPos == vector.Zero)
			return null;
		
		AIGroup group = COE_GameTools.SpawnGroupPrefab("{2CC26054775FBA2C}Prefabs/Groups/INDFOR/Group_FIA_Team_AT.et", spawnPos);
		group.AddWaypoint(m_pAttackWp);
		AddGroup(group);
		return group;
	}
	
	AIGroup SpawnDefensiveReinforcement()
	{
		array<ref Tuple3<float, int, vector>> weightedPositions = {};
		
		foreach (vector pos : m_pObjectiveArea.GetPositionsToDefend())
		{
			m_aQueriedFriendlyUnitsCount = 0;
			m_aQueriedEnemyUnitsCount = 0;
			GetGame().GetWorld().QueryEntitiesBySphere(pos, 50, QueryUnitsByFactionCallback);
			
			// Set default fraction to 50% (when no one is present)
			float friendlyFraction = 0.5;
			
			if (m_aQueriedFriendlyUnitsCount > 0 || m_aQueriedEnemyUnitsCount > 0)
				friendlyFraction = m_aQueriedFriendlyUnitsCount / (m_aQueriedFriendlyUnitsCount + m_aQueriedEnemyUnitsCount);
			
			weightedPositions.Insert(new Tuple3<float, int, vector>(friendlyFraction, m_aQueriedFriendlyUnitsCount, pos));
		};
		
		if (weightedPositions.IsEmpty())
			return null;
		
		// assign tuple elements to a non-ref array for sorting
		array<Tuple3<float, int, vector>> sortedWeightedPositions = {};
		foreach (Tuple3<float, int, vector> params : weightedPositions)
				sortedWeightedPositions.Insert(params);
		
		SCR_Sorting<Tuple3<float, int, vector>, SCR_ComparePositionsToDefend>.HeapSort(sortedWeightedPositions);	
		vector targetPos = sortedWeightedPositions[0].param3;
		
		vector spawnPos = GetSpawnPosition(targetPos);
		if (spawnPos == vector.Zero)
			return null;
		
		AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", targetPos);
		AIGroup group = COE_GameTools.SpawnGroupPrefab("{2CC26054775FBA2C}Prefabs/Groups/INDFOR/Group_FIA_Team_AT.et", spawnPos);
		group.AddWaypoint(wp);
		AddGroup(group);
		return group;
	}
	
	bool QueryUnitsByFactionCallback(IEntity entity)
	{
		if (!SCR_ChimeraCharacter.Cast(entity))
			return true;
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!factionComp)
			return true;
		
		Faction faction = factionComp.GetAffiliatedFaction();
		
		if (faction.GetFactionKey() == "FIA")
			m_aQueriedFriendlyUnitsCount += 1;
		else
			m_aQueriedEnemyUnitsCount += 1;
		
		return true;
	}
	
	vector GetSpawnPosition(vector targetPos)
	{
		
		vector vectDir = vector.Zero;
		
		foreach (SCR_SpawnPoint playerSpawn : SCR_SpawnPoint.GetSpawnPoints())
		{
			vector playerSpawnPos = playerSpawn.GetOrigin();
			
			
			// ignore spawns outside 150% the soft block radius
			if (vector.Distance(playerSpawnPos, targetPos) > 1.5*m_fSoftBlockSpawnRadius)
				continue;
			
			vector playerSpawnVectDir = targetPos - playerSpawnPos;
			vectDir += playerSpawnVectDir / playerSpawnVectDir.LengthSq();
		};
		
		if (vectDir == vector.Zero)
		{
			Math.Randomize(-1);
			vectDir = vector.FromYaw(Math.RandomFloat(0, 360));
		}
		else
		{
			float dir = vectDir.ToYaw();
			Math.Randomize(-1);
			dir += Math.RandomFloatInclusive(-50, 50);
			vectDir = vector.FromYaw(dir);
		};
		
		int stepSize = (m_fSoftBlockSpawnRadius - m_fHardBlockSpawnRadius) / 10;
		
		for (int i = m_fHardBlockSpawnRadius; i < m_fSoftBlockSpawnRadius; i += stepSize)
		{
			vector pos = targetPos + vectDir * (float)i;
			pos[1] = SCR_TerrainHelper.GetTerrainY(pos) + 1.0;
			
			if (IsSpawnPositionValid(pos))
				return pos;
		};
		
		return vector.Zero;
	}
	
	bool IsSpawnPositionValid(vector pos)
	{
		if (!SCR_WorldTools.TraceCylinder(pos))
			return false;
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			autoptr TraceParam trace = new TraceParam();
			trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.OCEAN;
			trace.Start = pos;
			
			SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
			if (!player)
				continue;
			
			trace.End = player.EyePosition();
			
			if (vector.Distance(pos, trace.End) > m_fSoftBlockSpawnRadius)
				continue;
										
			if (GetGame().GetWorld().TraceMove(trace, null) >= 0.999)
				return false;
		};
		
		return true;
	}
	
	void DeleteGroups()
	{
		foreach (AIGroup group : m_aManagedGroups)
			SCR_EntityHelper.DeleteEntityAndChildren(group);
	}
}
