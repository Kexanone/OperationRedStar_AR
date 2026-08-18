
//------------------------------------------------------------------------------------------------
class ORS_ObjectiveAreaClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_ObjectiveArea : GenericEntity
{
	[Attribute(defvalue: "300", desc: "Radius of the objective area", category: "Area")]
	protected float m_fAreaRadius;
	
	protected static ref array<ORS_ObjectiveArea> s_aInstances = {};
	
	[RplProp()]
	protected ORS_EObjectiveAreaState m_eState = ORS_EObjectiveAreaState.LOCKED;
			
	protected ref KSC_AreaBase m_pArea;
	protected ref ScriptInvokerInt m_OnStateChanged = new ScriptInvokerInt();
	protected ref array<AIGroup> m_aAIGroups = {};
	protected ref array<SCR_MapMarkerBase> m_aMarkers = {};
	
#ifdef WORKBENCH
	[Attribute(defvalue: "1", desc: "Show the debug shapes in Workbench", category: "Debug")];
	protected bool m_bShowDebugShapesInWorkbench;
#endif

	//------------------------------------------------------------------------------------------------
	void ORS_ObjectiveArea(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_pArea = KSC_CircleArea(GetOrigin(), m_fAreaRadius);
		s_aInstances.Insert(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnStateChanged(ORS_EObjectiveAreaState newState)
	{
		m_OnStateChanged.Invoke(newState);
		
		if (newState != ORS_EObjectiveAreaState.CAPTURED)
			return;
		
		SCR_MapMarkerManagerComponent markerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!markerManager)
			return;
		
		foreach (SCR_MapMarkerBase marker : m_aMarkers)
		{
			markerManager.RemoveStaticMarker(marker);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static array<ORS_ObjectiveArea> GetInstances()
	{
		return s_aInstances;
	}
	
	//------------------------------------------------------------------------------------------------
	static ORS_ObjectiveArea GetClosestArea(vector pos, ORS_EObjectiveAreaState state)
	{
		float closestDistanceSq = float.INFINITY;
		ORS_ObjectiveArea closestArea = null;
		
		foreach (ORS_ObjectiveArea area : s_aInstances)
		{
			if (area.GetState() != state)
				continue;
			
			float distanceSq = vector.DistanceSqXZ(pos, area.GetOrigin());
				
			if (distanceSq < closestDistanceSq)
			{
				closestDistanceSq = distanceSq;
				closestArea = area;
			}
		}
		
		return closestArea;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAreaRadius()
	{
		return m_fAreaRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetState(ORS_EObjectiveAreaState state)
	{
		m_eState = state;
		OnStateChanged(state);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_EObjectiveAreaState GetState()
	{
		return m_eState;
	}
	
	//------------------------------------------------------------------------------------------------
	KSC_AreaBase GetArea()
	{
		return m_pArea;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAIGroup(AIGroup group)
	{
		m_aAIGroups.Insert(group);
	}
	
	//------------------------------------------------------------------------------------------------
	array<AIGroup> GetAIGroups()
	{
		return m_aAIGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnStateChanged()
	{
		return m_OnStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~ORS_ObjectiveArea()
	{
		s_aInstances.RemoveItem(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void RevealEnemyPositions()
	{
		SCR_MapMarkerManagerComponent markerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!markerManager)
			return;
		
		foreach (AIGroup group : m_aAIGroups)
		{
			if (!group)
				continue;
			
			IEntity leader = group.GetLeaderEntity();
			if (!leader)
				continue;
			
			ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager)
				continue;
						
			SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
			marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
			marker.SetIconEntry(SCR_EScenarioFrameworkMarkerCustom.MARK_QUESTION);
			marker.SetColorEntry(factionManager.GetMarkerCustomColorEntry(factionManager.GetEnemyFaction()));
			vector pos = leader.GetOrigin();
			marker.SetWorldPos(pos[0], pos[2]);
			markerManager.InsertStaticMarker(marker, false, true);
			m_aMarkers.Insert(marker);
		}
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShape(bool draw)
	{
		if (!draw)
			return;

		
		Shape.CreateSphere(
			ARGB(100, 0x99, 0x10, 0xF2),
			ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
			GetOrigin(),
			m_fAreaRadius
		);
	}
	
	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_ALWAYS;
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);
		
		return false;
	}
#endif	
}

//------------------------------------------------------------------------------------------------
enum ORS_EObjectiveAreaState
{
	LOCKED,
	CONTESTED,
	CAPTURED
}
