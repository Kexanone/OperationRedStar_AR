
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
	
	/*	//------------------------------------------------------------------------------------------------
	//! Only called on the server
	void Init(bool isFinished = false)
	{
		if (isFinished)
			OnFinished();
	};
	
	//------------------------------------------------------------------------------------------------
	void CreateAreaMarkerLocal()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		if (m_bIsFinished)
			m_pAreaMarker = ORS_AreaMarker(m_vAreaCenter, m_fAreaRadius, m_AreaMarkerColorFinished);
		else
			m_pAreaMarker = ORS_AreaMarker(m_vAreaCenter, m_fAreaRadius, m_AreaMarkerColor);
		
		mapEntity.ORS_AddAreaMarker(m_pAreaMarker);
	}
	
	//------------------------------------------------------------------------------------------------
	void Spawn()
	{
		if (!s_PlayerFaction)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			array<Faction> factions = {};
			factionManager.GetFactionsList(factions);
			
			foreach (Faction faction : factions)
			{
				SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(faction);
				if (!campaignFaction)
					continue;
				
				if (campaignFaction.IsPlayable())
				{
					s_PlayerFaction = campaignFaction;
					break;
				}
			}
			
			s_EnemyFaction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey("FIA"));
			s_sFobPrefabName = s_PlayerFaction.GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
		};
		
		
		m_pAiManager = ORS_AiManagerComponent.Cast(FindComponent(ORS_AiManagerComponent));
		m_pAiManager.Start();
		
		m_pSlotsManager = COE_SlotsManagerComponent.Cast(FindComponent(COE_SlotsManagerComponent));
		if (!m_pSlotsManager)
			return;
		
		m_pSlotsManager.Init(m_vAreaCenter, m_fAreaRadius);
		
		
		SpawnMainTask(ORS_BuildFobTaskSupportEntity);
		
		GetGame().GetCallqueue().CallLater(HandleInitialFobBuilt, 10000, true);
		SpawnHQ();
		SpawnAssetsToDestroy();
		SpawnSupplyCache();
		SpawnRoadblocks();
		//m_pSlotsManager.PopulateTurrets();
		SpawnPatrols();
		SpawnMarksmen();
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetDefaultMainTaskPosition()
	{
		IEntity child = GetChildren();
		if (child)
			return child.GetOrigin();
		else
			return GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnMainTask(typename supportEntityType, vector pos = vector.Zero)
	{
		if (pos == vector.Zero)
			pos = GetDefaultMainTaskPosition();
		
		ORS_LocationBaseTaskSupportEntity supportEntity = ORS_LocationBaseTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(supportEntityType));
		if (!supportEntity)
			return;
		
		m_pMainTask = supportEntity.CreateTask(pos);
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
				
		for (int i = 0; i < 2; i++)
		{
			COE_WorldTools.SampleTransformInArea(transform, sampleArea, {}, params);
			AIGroup group = COE_GameTools.SpawnGroupPrefab("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", transform[3]);
			COE_AITasks.Patrol(group, GetDefaultMainTaskPosition(), patrolRadius);
			m_pAiManager.AddGroup(group);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnMarksmen()
	{
		Math.Randomize(-1);
		int count = Math.RandomInt(3, 5);		
				
		for (int i = 0; i < count; i++)
		{
			COE_AISlotConfig config = m_pSlotsManager.GetRandomMarksmanSlot();
			if (!config)
				return;
			
			SCR_AIGroup group = SCR_AIGroup.Cast(COE_GameTools.SpawnGroupPrefab("{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et"));
			if (!group)
				return;
			
			group.SetFaction(GetGame().GetFactionManager().GetFactionByKey("FIA"));
			
			vector transform[4];
			config.GetTransform(transform);
			IEntity entity = COE_GameTools.SpawnPrefab("{CE33AB22F61F3365}Prefabs/Characters/Factions/INDFOR/FIA/Character_FIA_Sharpshooter.et", transform);
			
			AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{4C919D3F6F1CD1B8}Prefabs/AI/Waypoints/COE_AIWaypoint_Defend_0m.et", transform[3]);
			group.AddWaypoint(wp);
			m_pAiManager.AddGroup(group);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnHQ()
	{
		SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(m_pSlotsManager.GetRandomMediumFlatSlot());
		if (!slot)
			return;
		
		m_aExcludedAreas.Insert(COE_CircleArea(slot.GetOrigin(), m_fMinDistanceBetweenTasks));
		IEntity hq = slot.SpawnEntityInSlot(Resource.Load("{FA39C56DECF3FB1F}Prefabs/Compositions/Slotted/SlotFlatSmall/ORS_LivingArea_FIA_01.et"));
		
		AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{05C25B8FADA10C69}Prefabs/AI/Waypoints/ORS_AIWaypoint_Defend_25m.et", hq.GetOrigin());
		AIGroup group = COE_GameTools.SpawnGroupPrefab("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", hq.GetOrigin());
		group.AddWaypoint(wp);
		m_pAiManager.AddGroup(group);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnSupplyCache()
	{
		SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(m_pSlotsManager.GetRandomMediumFlatSlot());
		if (!slot)
			return;
		
		m_aExcludedAreas.Insert(COE_CircleArea(slot.GetOrigin(), m_fMinDistanceBetweenTasks));
		IEntity cache = slot.SpawnEntityInSlot(Resource.Load("{47F9ED960D72C24E}Prefabs/Compositions/Slotted/SlotFlatSmall/ORS_SupplyCache_S_FIA_01.et"));
		
		AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{05C25B8FADA10C69}Prefabs/AI/Waypoints/ORS_AIWaypoint_Defend_25m.et", cache.GetOrigin());
		AIGroup group = COE_GameTools.SpawnGroupPrefab("{2CC26054775FBA2C}Prefabs/Groups/INDFOR/Group_FIA_Team_AT.et", cache.GetOrigin());
		group.AddWaypoint(wp);
		m_pAiManager.AddGroup(group);
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
			m_pAiManager.AddGroup(group);
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
			
			m_aTargetsToDestroy.Insert(ORS_TargetToDestroyWrapper(this, target, s_PlayerFaction));
									
			vector pos;
			SCR_WorldTools.FindEmptyTerrainPosition(pos, target.GetOrigin(), 10);
			AIWaypoint wp = COE_GameTools.SpawnWaypointPrefab("{05C25B8FADA10C69}Prefabs/AI/Waypoints/ORS_AIWaypoint_Defend_25m.et", target.GetOrigin());
			AIGroup group = COE_GameTools.SpawnGroupPrefab("{2CC26054775FBA2C}Prefabs/Groups/INDFOR/Group_FIA_Team_AT.et", pos);
			group.AddWaypoint(wp);
			m_pAiManager.AddGroup(group);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTargetDestroyed(IEntity target)
	{
		m_iNumberOfDestroyedTargets++;
		
		if (m_iNumberOfDestroyedTargets != m_aTargetsToDestroy.Count())
			return;
		
		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(ORS_DestroyTaskSupportEntity);
		if (!supportEntity)
			return;

		supportEntity.FinishTask(m_pMainTask);
		StartFinalDefense();
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
	array<vector> GetPositionsToDefend()
	{
		array<vector> positions = {GetDefaultMainTaskPosition()};
		
		foreach (ORS_TargetToDestroyWrapper targetWrapper : m_aTargetsToDestroy)
		{
			if (!targetWrapper.GetAsset() || !targetWrapper.IsAssetAlive())
				continue;
			
			positions.Insert(targetWrapper.GetAssetOrigin());
		};
		
		return positions;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetPositionToAttack()
	{
		IEntity fob = GetFob();
		if (fob)
			return fob.GetOrigin();
		else
			return GetDefaultMainTaskPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search for FOB and complete build task if one has been found
	protected void HandleInitialFobBuilt()
	{
		IEntity fob = GetFob();
		if (!fob)
			return;
		
		SCR_BaseTaskSupportEntity buildSupportEntity = GetTaskManager().FindSupportEntity(ORS_BuildFobTaskSupportEntity);
		if (!buildSupportEntity)
			return;

		buildSupportEntity.FinishTask(m_pMainTask);
		GetGame().GetCallqueue().Remove(HandleInitialFobBuilt);
		SpawnMainTask(ORS_SeizeTaskSupportEntity);
		
		foreach (ORS_TargetToDestroyWrapper targetWrapper : m_aTargetsToDestroy)
		{
			targetWrapper.CreateTask();
		};
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the currenty active FOB in the area
	protected IEntity GetFob()
	{
		float offset = GetBuildingPlacingRadius();
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		mins = Vector(m_vAreaCenter[0] - offset, mins[1], m_vAreaCenter[2] - offset);
		maxs = Vector(m_vAreaCenter[0] + offset, maxs[1], m_vAreaCenter[2] + offset);
		m_pFob = null;
		GetGame().GetWorld().QueryEntitiesByAABB(mins, maxs, QueryGetFobCallback);
		return m_pFob;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryGetFobCallback(IEntity entity)
	{
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return true;
		
		if (prefabData.GetPrefabName() != s_sFobPrefabName)
			return true;
		
		m_pFob = entity;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void StartFinalDefense()
	{
		vector pos = vector.Zero;
		
		// Assign defend position to FOB if one exists
		IEntity fob = GetFob();
		if (fob)
			pos = fob.GetOrigin();
		
		m_pAiManager.SetMode(ORS_EAiManagementMode.ATTACK);
		SpawnMainTask(ORS_DefendTaskSupportEntity, pos);		
		GetGame().GetCallqueue().CallLater(Finish, m_iFinalDefenseDuration*1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void Finish()
	{
		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(ORS_DefendTaskSupportEntity);
		if (!supportEntity)
			return;

		supportEntity.FinishTask(m_pMainTask);
		OnFinished();
		m_pAiManager.Stop();
		ScheduleCleanUp();
		
		ORS_GameMode gameMode = ORS_GameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.SpawnNextObjective();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFinished()
	{
		m_bIsFinished = true;
		Replication.BumpMe();
		
		m_pAreaMarker.SetColor(m_AreaMarkerColorFinished);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void ScheduleCleanUp()
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!player)
				continue;
			
			if (vector.DistanceXZ(m_vAreaCenter, player.GetOrigin()) > 1000)
				continue;
			
			// There are still players nearby => Delay clean-up
			GetGame().GetCallqueue().CallLater(ScheduleCleanUp, 60000);
			return;
		};
		
		// All players have left => Start clean-up
		m_pAiManager.DeleteGroups();
	}
*/
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

//------------------------------------------------------------------------------------------------
enum ORS_EObjectiveAreaState
{
	LOCKED,
	CONTESTED,
	CAPTURED
}
