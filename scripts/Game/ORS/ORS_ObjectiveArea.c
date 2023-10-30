//------------------------------------------------------------------------------------------------
class ORS_ObjectiveAreaClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class ORS_ObjectiveArea : GenericEntity
{
	[Attribute(defvalue: "300", desc: "Radius of the objective area", category: "Area")]
	protected float m_fAreaRadius;
	
	[Attribute(desc: "Initial color of the area marker", category: "Area")]
	protected ref Color m_AreaMarkerColor;
	
	[Attribute(desc: "Color of the area marker when finished", category: "Area")]
	protected ref Color m_AreaMarkerColorFinished;
	
	[Attribute(defvalue: "6", desc: "Number of destroy tasks", category: "Tasks")]
	protected int m_iNumberOfDestroyTasks;
	protected int m_iNumberOfDestroyedTargets = 0;
	
	[Attribute(defvalue: "50", desc: "Minimum distance between tasks in meters", category: "Tasks")]
	protected float m_fMinDistanceBetweenTasks;
	
#ifdef WORKBENCH
	[Attribute(defvalue: "1", desc: "Show the debug shapes in Workbench", category: "Debug")];
	protected bool m_bShowDebugShapesInWorkbench;
#endif
	
	[RplProp(onRplName: "OnFinished")]
	bool m_bIsFinished = false;
	
	// By how many meters the fortifications can be constructed outside the objective area 
	static const float ORS_BUILDING_PLACING_RADIUS_EXTENSION = 150;
		
	vector m_vAreaCenter;
	protected ref COE_AreaBase m_pArea;
	protected ref array<ref COE_AreaBase> m_aExcludedAreas = {};
	protected SCR_BaseTask m_pMainTask;
	protected ref MapItem m_pMapMarker;
	protected static SCR_CampaignFaction s_PlayerFaction;
	protected static SCR_CampaignFaction s_EnemyFaction;
	protected static string s_sFobPrefabName;
	protected COE_SlotsManagerComponent m_pSlotsManager;
	protected ref array<IEntity> m_aTargetsToDestroy = {};
	protected ref array<AIGroup> m_aManagedGroups = {};
	
	//------------------------------------------------------------------------------------------------
	void ORS_ObjectiveArea(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_vAreaCenter = GetOrigin();
		m_pArea = COE_CircleArea(m_vAreaCenter, m_fAreaRadius);
		CreateAreaMarkerLocal();
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateAreaMarkerLocal()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		m_pMapMarker = mapEntity.CreateCustomMapItem();
		m_pMapMarker.SetPos(m_vAreaCenter[0], m_vAreaCenter[2]);
		m_pMapMarker.SetBaseType(EMapDescriptorType.MDT_TASK);
		MapDescriptorProps props = m_pMapMarker.GetProps();
		
		if (m_bIsFinished)
			props.SetFrontColor(m_AreaMarkerColorFinished);
		else
			props.SetFrontColor(m_AreaMarkerColor);
		// Circumscribe area with a square => Sqrt(2)
		// Empirical scaling factor: 1.3 / 50
		float size = m_fAreaRadius * 1.3 / 50 * Math.Sqrt(2);
		props.SetIconSize(size, size, size);
		props.Activate(true);
		m_pMapMarker.SetProps(props);
	}
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (!s_PlayerFaction)
		{
			s_PlayerFaction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey("USSR"));
			s_EnemyFaction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey("FIA"));
			s_sFobPrefabName = s_PlayerFaction.GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
		};
		
		m_pSlotsManager = COE_SlotsManagerComponent.Cast(FindComponent(COE_SlotsManagerComponent));
		if (!m_pSlotsManager)
			return;
		
		m_pSlotsManager.Init(m_vAreaCenter, m_fAreaRadius);
		
		SpawnMainTask(ORS_BuildFobTaskSupportEntity);
		
		GetGame().GetCallqueue().CallLater(QueryFob, 10000, true);
		SpawnHQ();
		SpawnAssetsToDestroy();
		SpawnRoadblocks();
		m_pSlotsManager.PopulateTurrets();
		SpawnPatrols();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnMainTask(typename supportEntityType)
	{
		ORS_BaseTaskSupportEntity supportEntity = ORS_BaseTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(supportEntityType));
		if (!supportEntity)
			return;
		
		m_pMainTask = supportEntity.CreateTask(m_vAreaCenter);
		supportEntity.SetTargetFaction(m_pMainTask, s_PlayerFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnPatrols()
	{
		float patrolRadius = Math.Max(m_fAreaRadius/2, 100);
		COE_CircleArea sampleArea = COE_CircleArea(m_vAreaCenter, patrolRadius);
		COE_SamplePosParams params = COE_SamplePosParams();
		params.EmptyRadius = 0.5;
		params.MaxSlopeAngle = 90;
		vector transform[4];
				
		for (int i = 0; i < 3; i++)
		{
			COE_WorldTools.SampleTransformInArea(transform, sampleArea, {}, params);
			AIGroup group = COE_GameTools.SpawnGroupPrefab("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", transform[3]);
			COE_AITasks.Patrol(group, m_vAreaCenter, patrolRadius);
			m_aManagedGroups.Insert(group);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnHQ()
	{
		SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(m_pSlotsManager.GetRandomMediumFlatSlot());
		if (!slot)
			return;
		
		m_aExcludedAreas.Insert(COE_CircleArea(slot.GetOrigin(), m_fMinDistanceBetweenTasks));
		IEntity hq = slot.SpawnEntityInSlot(Resource.Load("{FA39C56DECF3FB1F}ORS_LivingArea_FIA_01.et"));
		
		AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{05C25B8FADA10C69}Prefabs/AI/Waypoints/ORS_AIWaypoint_Defend_25m.et", hq.GetOrigin());
		AIGroup group = COE_GameTools.SpawnGroupPrefab("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", hq.GetOrigin());
		group.AddWaypoint(wp);
		m_aManagedGroups.Insert(group);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnRoadblocks()
	{
		array<ResourceName> prefabNamesForSmallRoadSlot = {
			"{83A10E8547281E58}Prefabs/Compositions/Slotted/SlotRoadSmall/Barricade_S_USSR_01.et",
			"{9483333BFD9E2D0F}Prefabs/Compositions/Slotted/SlotRoadSmall/Checkpoint_S_USSR_01.et"
		};
		array<ResourceName> prefabNamesForMediumRoadSlot = {
			"{27F6A0815E6FF6EF}Prefabs/Compositions/Slotted/SlotRoadMedium/Barricade_M_USSR_01.et",
			"{8F0B0C5AB03C9D1F}Prefabs/Compositions/Slotted/SlotRoadMedium/ORS_Checkpoint_M_FIA_01.et"
		};
		array<ResourceName> prefabNamesForLargeRoadSlot = {
			"{2A27606856B8A914}Prefabs/Compositions/Slotted/SlotRoadLarge/Barricade_L_USSR_01.et",
			"{0E714B8AC19209FB}Prefabs/Compositions/Slotted/SlotRoadLarge/ORS_Checkpoint_L_FIA_01.et"
		};
		
		Math.Randomize(-1);
		int count = Math.RandomIntInclusive(3, 5);
						
		for (int i = 0; i < count; i++)
		{
			array<ResourceName> prefabNames;
			
			SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(m_pSlotsManager.GetRandomRoadSlot());
			if (!slot)
				return;
			
			switch (m_pSlotsManager.GetSiteSlotLabel(slot))
			{
				case EEditableEntityLabel.SLOT_ROAD_SMALL: {prefabNames = prefabNamesForSmallRoadSlot; break;};
				case EEditableEntityLabel.SLOT_ROAD_MEDIUM: {prefabNames = prefabNamesForMediumRoadSlot; break;};
				case EEditableEntityLabel.SLOT_ROAD_LARGE: {prefabNames = prefabNamesForLargeRoadSlot; break;};
			}
			IEntity roadblock = slot.SpawnEntityInSlot(Resource.Load(prefabNames[Math.RandomInt(0, prefabNames.Count())]));
			
			if (!roadblock.FindComponent(SCR_AISmartActionComponent))
				continue;
			
			AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{05C25B8FADA10C69}Prefabs/AI/Waypoints/ORS_AIWaypoint_Defend_25m.et", roadblock.GetOrigin());
			AIGroup group = COE_GameTools.SpawnGroupPrefab("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", roadblock.GetOrigin());
			group.AddWaypoint(wp);
			m_aManagedGroups.Insert(group);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnAssetsToDestroy()
	{
		array<ResourceName> prefabNamesForAssetsToDestroy = {
			"{53E5D391DE04A98A}Prefabs/Compositions/Installation/FIA/ORS_R404_Antenna.et",
			"{5C907FF76A38F3D6}Prefabs/Compositions/Installation/FIA/ORS_RPL5_Radar.et",
			"{C1CB2E508DC277B4}Prefabs/Compositions/Installation/FIA/ORS_Ural4320_FIA_command.et",
			"{2E370B9717DF71B8}Prefabs/Compositions/Installation/FIA/ORS_Ural4320_FIA_tanker.et",
			"{CFC9E81A7F5734A4}Prefabs/Compositions/Installation/FIA/ORS_Ural4320_repair_FIA.et",
			"{B47110AA1A806556}Prefabs/Vehicles/Wheeled/BTR70/BTR70_FIA.et"
		};
		
		SCR_ArrayHelperT<ResourceName>.Shuffle(prefabNamesForAssetsToDestroy);
		
		for (int i = 0; i < m_iNumberOfDestroyTasks; i++)
		{
			SCR_SiteSlotEntity slot;
			
			while (true)
			{
				slot = SCR_SiteSlotEntity.Cast(m_pSlotsManager.GetRandomFlatSlot());
				if (!slot || IsTaskPositionValid(slot.GetOrigin()))
					break;
			};
			
			if (!slot)
				return;
			
			vector transform[4];
			slot.GetWorldTransform(transform);
			Math.Randomize(-1);
			COE_WorldTools.SetTransformRotation(transform, Math.RandomFloat(0, 360));
			SCR_TerrainHelper.SnapAndOrientToTerrain(transform);
			ResourceName prefabName = prefabNamesForAssetsToDestroy[i % prefabNamesForAssetsToDestroy.Count()];
			IEntity target = COE_GameTools.SpawnStructurePrefab(prefabName, transform);
			
			if (slot)
				slot.SetOccupant(target);
			
			m_aExcludedAreas.Insert(COE_CircleArea(slot.GetOrigin(), m_fMinDistanceBetweenTasks));
			
			ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(target.FindComponent(ScriptedDamageManagerComponent));
		
			// Set subjet to child if parent is not destructible
			if (!objectDmgManager)
			{
				target = target.GetChildren();
				objectDmgManager = ScriptedDamageManagerComponent.Cast(target.FindComponent(ScriptedDamageManagerComponent));
			};
			
			if (objectDmgManager)
				objectDmgManager.GetOnDamageStateChanged().Insert(OnAssetDamageStateChanged);
			
			m_aTargetsToDestroy.Insert(target);
			
			vector pos;
			SCR_WorldTools.FindEmptyTerrainPosition(pos, target.GetOrigin(), 10);
			AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{05C25B8FADA10C69}Prefabs/AI/Waypoints/ORS_AIWaypoint_Defend_25m.et", target.GetOrigin());
			AIGroup group = COE_GameTools.SpawnGroupPrefab("{2CC26054775FBA2C}Prefabs/Groups/INDFOR/Group_FIA_Team_AT.et", pos);
			group.AddWaypoint(wp);
			m_aManagedGroups.Insert(group);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAssetDamageStateChanged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		m_iNumberOfDestroyedTargets++;
		
		if (m_iNumberOfDestroyedTargets < m_iNumberOfDestroyTasks)
			return;
		
		m_pMainTask.Finish();
		ORS_GameMode gameMode = ORS_GameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GenerateNextObjective();
	}
	
	protected bool IsTaskPositionValid(vector pos)
	{
		foreach (COE_AreaBase area : m_aExcludedAreas)
		{
			if (area.IsPointInArea(pos))
				return false;
		};
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAreaRadius()
	{
		return m_fAreaRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBuildingPlacingRadius()
	{
		return m_fAreaRadius + ORS_BUILDING_PLACING_RADIUS_EXTENSION;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search for FOBs and complete build task if one has been found
	protected void QueryFob()
	{
		float offset = GetBuildingPlacingRadius();
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		mins = Vector(m_vAreaCenter[0] - offset, mins[1], m_vAreaCenter[2] - offset);
		maxs = Vector(m_vAreaCenter[0] + offset, maxs[1], m_vAreaCenter[2] + offset);
		GetGame().GetWorld().QueryEntitiesByAABB(mins, maxs, QueryFobCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryFobCallback(IEntity entity)
	{
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return true;
		
		if (prefabData.GetPrefabName() != s_sFobPrefabName)
			return true;
		
		m_pMainTask.Finish();
		GetGame().GetCallqueue().Remove(QueryFob);
		SpawnMainTask(ORS_SeizeTaskSupportEntity);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void Finish()
	{
		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(ORS_SeizeTaskSupportEntity);
		if (!supportEntity)
			return;

		supportEntity.FinishTask(m_pMainTask);
		OnFinished();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFinished()
	{
		m_bIsFinished = true;
		Replication.BumpMe();
			
		MapDescriptorProps props = m_pMapMarker.GetProps();
		props.SetFrontColor(m_AreaMarkerColorFinished);
		m_pMapMarker.SetProps(props);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsFinished()
	{
		return m_bIsFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	void ScheduleCleanUp()
	{
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
