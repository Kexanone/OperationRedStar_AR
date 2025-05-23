//------------------------------------------------------------------------------------------------
class ORS_ReinforcementSystem : BaseSystem
{
	[Attribute(defvalue: "15", desc: "Timeout between updates [s]")]
	protected float m_fUpdateTimeout;
	protected float m_fUpdateTimer = 0;
	
	[Attribute(defvalue: "600", desc: "Maximum distance AI can spawn from their target position [m]")]
	protected int m_fMaxSpawnDistance;
	
	[Attribute(defvalue: "200", desc: "AI cannot spawn below this distance to a player [m]")]
	protected int m_fHardBlockSpawnRadius;
	
	[Attribute(defvalue: "1000", desc: "If players are below this distance to a potential AI spawn position, a visibility check is done [m]")]
	protected int m_fSoftBlockSpawnRadius;
	
	protected ref array<ORS_ReinforcementComponent> m_aComponents = {};
	protected ref map<IEntity, ref ORS_HarassementData> m_aHarassedPlayers = new map<IEntity, ref ORS_HarassementData>();
	
	static private ORS_ReinforcementSystem s_pInstance; 
		
	//------------------------------------------------------------------------------------------------
	static ORS_ReinforcementSystem GetInstance()
	{
		if (!s_pInstance)
		{
			World world = GetGame().GetWorld();
			s_pInstance = ORS_ReinforcementSystem.Cast(world.FindSystem(ORS_ReinforcementSystem));
		}
		
		return s_pInstance;
	}
	
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.AddPoint(ESystemPoint.Frame);
		outInfo.SetLocation(ESystemLocation.Server);
		outInfo.SetAbstract(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Turn off if no AO is managed
	override protected void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		m_fUpdateTimer += GetGame().GetWorld().GetTimeSlice();
		if (m_fUpdateTimer < m_fUpdateTimeout)
			return;
		
		m_fUpdateTimer = 0;
		
		foreach (ORS_ReinforcementComponent component : m_aComponents)
		{
			
		}
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	protected void ComputePositionsToAttack(ORS_ReinforcementComponent component, out notnull array<vector> positions, out notnull array<float> weights)
	{
		map<IEntity, ref SCR_AITargetInfo> compiledTargets = new map<IEntity, ref SCR_AITargetInfo>();
		
		foreach (AIGroup group : component.GetManagedGroups())
		{
			SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
			if (!utility || !utility.m_Perception)
				continue;
			
			foreach (SCR_AITargetInfo targetInfo : utility.m_Perception.m_aTargets)
			{
				SCR_AITargetInfo latestTargetInfo;
				if (!compiledTargets.Find(targetInfo.m_Entity, latestTargetInfo) || latestTargetInfo.m_fTimestamp > targetInfo.m_fTimestamp)
					compiledTargets[targetInfo.m_Entity] = targetInfo;
			}
		}
		
		array<vector> targetPositions = {};
		
		foreach (SCR_AITargetInfo targetInfo : compiledTargets)
		{
			targetPositions.Insert(targetInfo.m_vWorldPos);
		}
		
		KSC_DBSCAN<vector> dbscan = new KSC_DBSCAN<vector>(eps: 14, minSamples: 1);
		array<int> labels = dbscan.FitPredict(targetPositions);
		int numClusters = dbscan.GetNumClusters();
		
		array<int> entityCounts = {};
		entityCounts.Resize(numClusters);
		positions.Resize(numClusters);
		weights.Resize(numClusters);
		int totEntityCount = labels.Count();
		
		foreach (int i, int label : labels)
		{
			positions[label - 1] = positions[label - 1] + targetPositions[i];
			entityCounts[label - 1] = entityCounts[label - 1] + 1;
		}
		
		foreach (int i, int count : entityCounts)
		{
			positions[i] = positions[i] / count;
			weights[i] = count / totEntityCount;
		}
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! If anyAttackDirection is false, the attack direction will be in opposite direction to player spawns
	protected bool GetSpawnPosition(vector targetPos, out vector spawnPos, bool anyAttackDirection = false)
	{
		vector vectDir = vector.Zero;
		
		if (!anyAttackDirection)
		{
			foreach (SCR_SpawnPoint playerSpawn : SCR_SpawnPoint.GetSpawnPoints())
			{
				vector playerSpawnPos = playerSpawn.GetOrigin();
				
				
				// ignore spawns outside 150% the soft block radius
				if (vector.Distance(playerSpawnPos, targetPos) > 1.5*m_fSoftBlockSpawnRadius)
					continue;
				
				vector playerSpawnVectDir = targetPos - playerSpawnPos;
				
				float lengthSq = playerSpawnVectDir.LengthSq();
				if (lengthSq < 1)
					lengthSq = 1;
				
				vectDir += playerSpawnVectDir / lengthSq;
			}
		}
		
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
		}
		
		int stepSize = (m_fMaxSpawnDistance) / 20;
		
		for (int i = 0; i < m_fMaxSpawnDistance; i += stepSize)
		{
			spawnPos = targetPos + vectDir * (float)i;
			spawnPos[1] = SCR_TerrainHelper.GetTerrainY(spawnPos) + 1.0;
			
			if (IsSpawnPositionValid(spawnPos))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsSpawnPositionValid(vector pos)
	{
		if (!SCR_WorldTools.TraceCylinder(pos))
			return false;
		
		if (KSC_TerrainHelper.SurfaceIsWater(pos))
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
			
			float distance = vector.Distance(pos, trace.End);
			
			// Invalid if too close to a player
			if (distance <= m_fHardBlockSpawnRadius)
				return false;
			
			if (vector.Distance(pos, trace.End) > m_fSoftBlockSpawnRadius)
				continue;
			
			// Invalid if player has LoSs
			if (GetGame().GetWorld().TraceMove(trace, null) >= 0.999)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Register(ORS_ReinforcementComponent component)
	{
		m_aComponents.Insert(component);
		
		if (!IsEnabled())
			Enable(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(ORS_ReinforcementComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
}
