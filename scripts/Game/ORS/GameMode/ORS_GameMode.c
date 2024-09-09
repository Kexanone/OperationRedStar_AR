//------------------------------------------------------------------------------------------------
class ORS_GameModeClass : SCR_BaseGameModeClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_GameMode : SCR_BaseGameMode
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Names of the main base entity", params: "et", category: "Carrier Main Base")]
	protected ResourceName m_sCarrierPrefabName;
	
	[Attribute(defvalue: "CarrierSpawnInnerBorder", desc: "Inner border for carrier spawn positions", category: "Carrier Main Base")]
	protected string m_sCarrierSpawnInnerBorderPickerName;
	
	[Attribute(defvalue: "CarrierSpawnOuterBorder", desc: "Outer border for carrier spawn positions", category: "Carrier Main Base")]
	protected string m_sCarrierSpawnOuterBorderPickerName;
	
	[Attribute(defvalue: "1500", desc: "Maximum distance for a FOB to unlock an AO", category: "FOB")]
	protected float m_fMaxAOUnlockDistance;
	protected float m_fMaxAOUnlockDistanceSq;
			
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
	
	protected ORS_MissionHeader m_pMissionHeader;
	protected ORS_FactionManager m_pFactionManager;
	protected SCR_BaseTaskManager m_pTaskManager;
	
	protected ResourceName m_sCommandPostPrefabName;
	protected ref array<vector> m_aFobPositions = {};
	protected bool m_bIsSessionLoadedFromSave = false;
	protected KSC_CounterTask m_pDestroyCommNodesTask;
	protected ref array<ref ORS_TargetToDestroyWrapper> m_aCommNodes;
	
	protected static ORS_GameMode s_pInstance;
		
	//------------------------------------------------------------------------------------------------
	static ORS_GameMode GetInstance()
	{
		if (!s_pInstance)
		{
			s_pInstance = ORS_GameMode.Cast(GetGame().GetGameMode());
		}
		
		return s_pInstance;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnGameStart()
	{
		super.OnGameStart();
		
		if (!IsMaster())
			return;
		
		m_pMissionHeader = ORS_MissionHeader.GetInstance();
		m_pFactionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		m_pTaskManager = GetTaskManager();
		m_fMaxAOUnlockDistanceSq = Math.Pow(m_fMaxAOUnlockDistance, 2);
		
		KSC_GameTools.SetAISkill(m_pMissionHeader.m_eEnemyAISkill);
		
		array<ResourceName> prefabNames = {};
		m_pFactionManager.GetFactionEntityListWithLabel(m_pFactionManager.GetPlayerFaction(), EEntityCatalogType.COMPOSITION, EEditableEntityLabel.SERVICE_HQ, prefabNames);
		if (prefabNames.IsEmpty())
		{
			Print("ORS_GameMode.OnGameStart: No command post prefab found", LogLevel.ERROR);
			return;
		}
		
		m_sCommandPostPrefabName = prefabNames[0];
		SCR_CampaignBuildingCompositionComponent.KSC_GetOnCompositionSpawnedServer().Insert(OnCompositionBuilt);
		
		if (m_bIsSessionLoadedFromSave)
		{
			if (m_pCurrentObjectiveArea)
			{
				CreateSeizeAreaTask();
				CreateDestroyCommNodesTask();
			}
			else
			{
				SetUpNextArea();
			}
		}
		// Starting a new game
		else
		{
			SetUpStartPosition();
			SetUpNextArea(ignoreDistanceLimit: true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCompositionBuilt(IEntity entity)
	{
		EntityPrefabData data = entity.GetPrefabData();
		if (!data)
			return;
		
		if (data.GetPrefabName() != m_sCommandPostPrefabName)
			return;
		
		m_aFobPositions.Insert(entity.GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetUpStartPosition()
	{
		PolylineArea outerBorderPicker = PolylineArea.Cast(GetGame().GetWorld().FindEntityByName(m_sCarrierSpawnOuterBorderPickerName));
		if (!outerBorderPicker)
			return;
		
		PolylineArea innerBorderPicker = PolylineArea.Cast(GetGame().GetWorld().FindEntityByName(m_sCarrierSpawnInnerBorderPickerName));
		if (!innerBorderPicker)
			return;
		
		KSC_PolygonArea outerBorder = KSC_PolygonArea.Cast(outerBorderPicker.KSC_GetPolygonArea());
		KSC_PolygonArea innerBorder = KSC_PolygonArea.Cast(innerBorderPicker.KSC_GetPolygonArea());
		Math.Randomize(-1);
		KSC_GameTools.SpawnPrefab(m_sCarrierPrefabName, KSC_AreaBase.SamplePointInArea(outerBorder, innerBorder), Math.RandomFloat(0, 360));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Select next area based on proximity to bases
	//! If distance limit is enabled, a FOB has to be in range, otherwise a build task is created instead
	protected void SetUpNextArea(bool ignoreDistanceLimit = false)
	{
		array<ORS_ObjectiveArea> areas = ORS_ObjectiveArea.GetInstances();
		
		m_pCurrentObjectiveArea = null;
		float closestDistanceSq = float.INFINITY;
		
		// Find area closest to a FOB
		foreach (vector fobPos : m_aFobPositions)
		{
			ORS_ObjectiveArea area = ORS_ObjectiveArea.GetClosestArea(fobPos, ORS_EObjectiveAreaState.LOCKED);
			if (!area)
				continue;			
			
			float distanceSq = vector.DistanceSqXZ(fobPos, area.GetOrigin());
			
			if (distanceSq < closestDistanceSq)
			{
				closestDistanceSq = distanceSq;
				m_pCurrentObjectiveArea = area;
			}
		}
		
		// If none could be found, find area closest to the main base
		if (!m_pCurrentObjectiveArea)
		{
			foreach (ORS_MainBaseEntity base : ORS_MainBaseEntity.GetInstances())
			{
				vector basePos = base.GetOrigin();
				
				ORS_ObjectiveArea area = ORS_ObjectiveArea.GetClosestArea(basePos, ORS_EObjectiveAreaState.LOCKED);
				if (!area)
					continue;
				
				float distanceSq = vector.DistanceSqXZ(basePos, base.GetOrigin());
				
				if (distanceSq < closestDistanceSq)
				{
					closestDistanceSq = distanceSq;
					m_pCurrentObjectiveArea = area;
				}
			}
			
			// Reset distance, so we are forced to build first
			closestDistanceSq = float.INFINITY;
		}
		
		// End game when no area is left
		if (!m_pCurrentObjectiveArea)
		{
			GetGame().GetCallqueue().CallLater(EndGameMode, 10000, false, SCR_GameModeEndData.CreateSimple(EGameOverTypes.COMBATPATROL_VICTORY));
			return;
		}
		
		// Build FOB first if none is in proximity of next area
		if (!ignoreDistanceLimit && m_fMaxAOUnlockDistanceSq < closestDistanceSq)
		{
			// Select closest unlocked area as location for build task
			m_pCurrentObjectiveArea = ORS_ObjectiveArea.GetClosestArea(m_pCurrentObjectiveArea.GetOrigin(), ORS_EObjectiveAreaState.CAPTURED);
			CreateFOBBuildTask(m_pCurrentObjectiveArea.GetOrigin());
			return;
		}
		
		m_pCurrentObjectiveArea.SetState(ORS_EObjectiveAreaState.CONTESTED);
		
		ORS_ObjectiveAreaBuilder builder = new ORS_ObjectiveAreaBuilder(m_pCurrentObjectiveArea);
		builder.SpawnFireDirectionCenter()
			.SpawnArmoredVehicle()
			.SpawnArmoredVehicle()
			.SpawnArmoredVehicle()
			.SpawnCommNode()
			.SpawnCommNode()
			.SpawnCommNode()
			.SpawnServiceStructure()
			.SpawnServiceStructure()
			.SpawnServiceStructure()
			.SpawnCheckpoint()
			.SpawnCheckpoint()
			.SpawnCheckpoint()
			.SpawnCheckpoint();
		
		CreateSeizeAreaTask();
		CreateDestroyCommNodesTask();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateFOBBuildTask(vector pos)
	{
		KSC_BuildTaskSupportEntity buildTaskSupportEntity = KSC_BuildTaskSupportEntity.Cast(m_pTaskManager.FindSupportEntity(KSC_BuildTaskSupportEntity));
		KSC_BaseTask task = KSC_BaseTask.Cast(buildTaskSupportEntity.CreateTask(m_pFactionManager.GetPlayerFaction(), pos, m_sCommandPostPrefabName));
		task.GetOnStateChanged().Insert(OnFOBBuildTaskCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFOBBuildTaskCompleted(KSC_BaseTask task, SCR_TaskState previousState, SCR_TaskState newState)
	{
		if (newState != SCR_TaskState.FINISHED)
			return;
		
		task.GetOnStateChanged().Remove(OnFOBBuildTaskCompleted);
		SetUpNextArea();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateSeizeAreaTask()
	{
		KSC_ClearAreaTaskSupportEntity seizeTaskSupportEntity = KSC_ClearAreaTaskSupportEntity.Cast(m_pTaskManager.FindSupportEntity(KSC_ClearAreaTaskSupportEntity));
		KSC_BaseTask task = KSC_BaseTask.Cast(seizeTaskSupportEntity.CreateTask(m_pFactionManager.GetPlayerFaction(), m_pCurrentObjectiveArea.GetOrigin(), m_pCurrentObjectiveArea.GetAreaRadius(), 0.75));
		task.GetOnStateChanged().Insert(OnObjectiveAreaSeized);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectiveAreaSeized(KSC_BaseTask task, SCR_TaskState previousState, SCR_TaskState newState)
	{
		if (newState != SCR_TaskState.FINISHED)
			return;
		
		task.GetOnStateChanged().Remove(OnObjectiveAreaSeized);
		m_pCurrentObjectiveArea.SetState(ORS_EObjectiveAreaState.CAPTURED);
		SetUpNextArea();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateDestroyCommNodesTask()
	{
		array<SCR_DamageManagerComponent> damageManagers = {};
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		core.GetAllEntities(entities, true);
		
		foreach (SCR_EditableEntityComponent entity : entities)
		{
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
			if (!info)
				continue;
			
			if (!info.HasEntityLabel(EEditableEntityLabel.KSC_TRAIT_HVT) && !(info.HasEntityLabel(EEditableEntityLabel.VEHICLE_TRUCK) && info.HasEntityLabel(EEditableEntityLabel.TRAIT_RADIO)))
				continue;
			
			if (vector.DistanceXZ(entity.GetOwner().GetOrigin(), m_pCurrentObjectiveArea.GetOrigin()) > m_pCurrentObjectiveArea.GetAreaRadius())
				continue;
			
			SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(entity.GetOwner().FindComponent(SCR_DamageManagerComponent));
			if (!damageManager)
			{
				IEntity child = entity.GetOwner().GetChildren();
				while (child)
				{
					damageManager = SCR_DamageManagerComponent.Cast(child.FindComponent(SCR_DamageManagerComponent));
					if (damageManager)
						break;
					
					child = child.GetSibling();
				}
			}
						
			if (!damageManager)
				continue;
			
			SCR_DestructionMultiPhaseComponent destructionManager = SCR_DestructionMultiPhaseComponent.Cast(damageManager);
			if (!destructionManager && damageManager.IsDestroyed())
				continue;
			
			if (destructionManager && destructionManager.GetDamagePhase() > 0)
				continue;
			
			damageManagers.Insert(damageManager);
		}
		
		if (damageManagers.IsEmpty())
			return;
		
		ORS_DestroyCommNodesTaskSupportEntity taskSupportEntity = ORS_DestroyCommNodesTaskSupportEntity.Cast(m_pTaskManager.FindSupportEntity(ORS_DestroyCommNodesTaskSupportEntity));
		if (!taskSupportEntity)
			return;
		
		m_pDestroyCommNodesTask = KSC_CounterTask.Cast(taskSupportEntity.CreateTask(m_pFactionManager.GetPlayerFaction(), m_pCurrentObjectiveArea.GetOrigin() + Vector(150, 0, 0), damageManagers.Count()));
		m_aCommNodes = {};
		
		foreach (SCR_DamageManagerComponent damageManager : damageManagers)
		{
			m_aCommNodes.Insert(new ORS_TargetToDestroyWrapper(damageManager, m_pDestroyCommNodesTask));
		}
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
		array<ORS_EObjectiveAreaState> states = data.GetObjectiveAreaStates();
		
		foreach (int i, ORS_ObjectiveArea area : ORS_ObjectiveArea.GetInstances())
		{
			area.SetState(states[i]);
			
			if (states[i] == ORS_EObjectiveAreaState.CONTESTED)
				m_pCurrentObjectiveArea = area;
		}
		
		m_bIsSessionLoadedFromSave = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads the state of the previous session
	void SaveSessionData(out ORS_SessionStruct data)
	{
		array<ORS_ObjectiveArea> areas = ORS_ObjectiveArea.GetInstances();
		array<ORS_EObjectiveAreaState> states = {};
		states.Reserve(areas.Count());
		
		foreach (ORS_ObjectiveArea area : areas)
		{
			states.Insert(area.GetState());
		}
		
		data.SetObjectiveAreaStates(states);
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
		{
			GetGame().GetCallqueue().CallLater(EndGameMode, 10000, false, SCR_GameModeEndData.CreateSimple(EGameOverTypes.COMBATPATROL_VICTORY));
			return;
		};
		
		m_pCurrentObjectiveArea = m_aObjectiveAreas[m_iCurrentObjectiveAreaIdx];
		/*
		m_pCurrentObjectiveArea.Spawn();
		*/
		
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
		/*
		objectiveArea.ScheduleCleanUp();
		*/
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

//------------------------------------------------------------------------------------------------
class ORS_TargetToDestroyWrapper : Managed
{
	protected IEntity m_pTarget;
	protected SCR_DamageManagerComponent m_pDamageManager;
	protected KSC_PlayerPresenceTriggerEntity m_pPlayerTrigger;
	protected KSC_CounterTask m_pTask;
		
	protected static const ResourceName PLAYER_PRESENCE_TRIGGER_PREFAB_NAME = "{57EFA65FB424C4F3}Prefabs/ScenarioFramework/Triggers/KSC_PlayerPresenceTrigger.et";
	
	//------------------------------------------------------------------------------------------------
	void ORS_TargetToDestroyWrapper(notnull SCR_DamageManagerComponent targetDamageManager, notnull KSC_CounterTask task)
	{
		m_pTarget = targetDamageManager.GetOwner().GetRootParent();
		m_pDamageManager = targetDamageManager;
		m_pTask = task;	
		m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		m_pTarget.GetWorldTransform(params.Transform);
		m_pPlayerTrigger = KSC_PlayerPresenceTriggerEntity.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(PLAYER_PRESENCE_TRIGGER_PREFAB_NAME), null, params));
		m_pPlayerTrigger.GetOnActivate().Insert(OnTargetFound);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn destroy task if a player is nearby
	void OnTargetFound()
	{
		KSC_DestroyObjectTaskSupportEntity destroyTaskSupportEntity = KSC_DestroyObjectTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(KSC_DestroyObjectTaskSupportEntity));
		if (destroyTaskSupportEntity)
		{
			ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
			if (factionManager)
				destroyTaskSupportEntity.CreateTask(factionManager.GetPlayerFaction(), m_pTarget);
		}
		
		m_pPlayerTrigger.GetOnActivate().Remove(OnTargetFound);
		SCR_EntityHelper.DeleteEntityAndChildren(m_pPlayerTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDamageStateChanged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
				
		m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
		m_pTask.IncrementCounter();
		
		if (m_pPlayerTrigger)
		{
			m_pPlayerTrigger.GetOnActivate().Remove(OnTargetFound);
			SCR_EntityHelper.DeleteEntityAndChildren(m_pPlayerTrigger);
		}
	}
}
