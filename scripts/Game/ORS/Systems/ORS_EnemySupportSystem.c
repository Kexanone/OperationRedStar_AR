//------------------------------------------------------------------------------------------------
class ORS_EnemySupportSystem : BaseSystem
{
	[Attribute(defvalue: "15", desc: "Timeout between updates [s]")]
	protected float m_fUpdateTimeout;
	protected float m_fUpdateTimer = 0;
	
	protected ref array<ORS_EnemySupportComponent> m_aComponents = {};
			
	//------------------------------------------------------------------------------------------------
	static ORS_EnemySupportSystem GetInstance()
	{
		ChimeraWorld world = GetGame().GetWorld();
		return ORS_EnemySupportSystem.Cast(world.FindSystem(ORS_EnemySupportSystem));
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
		
		foreach (ORS_EnemySupportComponent component : m_aComponents)
		{
			OnUpdateComponent(component);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUpdateComponent(ORS_EnemySupportComponent component)
	{
		RemoveDeadSupports(component);
		
		array<vector> positions = {};
		array<float> weights = {};
		ComputePositionsToAttack(ORS_ObjectiveArea.Cast(component.GetOwner()), positions, weights);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveDeadSupports(ORS_EnemySupportComponent component)
	{
		foreach (ORS_EnemySupportContext context : component.GetAllSupports())
		{
			if (!context.IsAlive())
				component.RemoveSupport(context);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ComputePositionsToAttack(ORS_ObjectiveArea area, out notnull array<vector> positions, out notnull array<float> weights)
	{
		map<IEntity, ref SCR_AITargetInfo> compiledTargets = new map<IEntity, ref SCR_AITargetInfo>();
		
		foreach (AIGroup group : area.GetAIGroups())
		{
			if (!group)
				continue;
			
			SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
			if (!utility || !utility.m_Perception)
				continue;
			
			foreach (SCR_AITargetInfo targetInfo : utility.m_Perception.m_aTargets)
			{
				ChimeraCharacter targetChar = ChimeraCharacter.Cast(targetInfo.m_Entity);
				if (!targetChar || targetChar.GetCharacterController().GetLifeState() != ECharacterLifeState.ALIVE)
					continue;
				
				SCR_AITargetInfo latestTargetInfo;
				if (compiledTargets.Find(targetInfo.m_Entity, latestTargetInfo) && latestTargetInfo.m_fTimestamp > targetInfo.m_fTimestamp)
					continue;
				
				compiledTargets[targetInfo.m_Entity] = targetInfo;
			}
		}
		
		array<vector> targetPositions = {};
		
		foreach (SCR_AITargetInfo targetInfo : compiledTargets)
		{
			targetPositions.Insert(targetInfo.m_vWorldPos);
		}
		
		KSC_DBSCAN<vector> dbscan = new KSC_DBSCAN<vector>(eps: 50, minSamples: 2);
		array<int> labels = dbscan.FitPredict(targetPositions);
		int numClusters = dbscan.GetNumClusters();
		
		array<int> entityCounts = {};
		entityCounts.Resize(numClusters);
		positions.Resize(numClusters);
		weights.Resize(numClusters);
		int totEntityCount = 0;
		
		foreach (int i, int label : labels)
		{
			// Insert points that did not get assigned to a cluster
			if (label < 0)
			{
				positions.Insert(targetPositions[i]);
				entityCounts.Insert(1);
				weights.Insert(0);
				++totEntityCount;
				continue;
			}
			
			positions[label] = positions[label] + targetPositions[i];
			entityCounts[label] = entityCounts[label] + 1;
			++totEntityCount;
		}
		
		foreach (int i, int count : entityCounts)
		{
			positions[i] = positions[i] / count;
			weights[i] = count;
		}
		
	#ifdef WORKBENCH
		SCR_MapMarkerManagerComponent markerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!markerManager)
			return;
		
		for (int i = m_aMarkers.Count() - 1; i >= 0; --i)
		{
			markerManager.RemoveStaticMarker(m_aMarkers[i]);
			m_aMarkers.Remove(i);
		}
		
		foreach (int i, int label : labels)
		{
			SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
			marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
			marker.SetIconEntry(SCR_EScenarioFrameworkMarkerCustom.DOT);
			marker.SetColorEntry((label + 1) % (SCR_EScenarioFrameworkMarkerCustomColor.DARK_PINK + 1));
			marker.SetWorldPos(targetPositions[i][0], targetPositions[i][2]);
			markerManager.InsertStaticMarker(marker, false, true);
			m_aMarkers.Insert(marker);
		}
	#endif
	}
	
#ifdef WORKBENCH
	ref array<ref SCR_MapMarkerBase> m_aMarkers = {};
#endif
		
	//------------------------------------------------------------------------------------------------
	void Register(ORS_EnemySupportComponent component)
	{
		m_aComponents.Insert(component);
		
		if (!IsEnabled())
			Enable(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(ORS_EnemySupportComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
}
