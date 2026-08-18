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
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Prefab for seize task", params: "et", category: "Tasks")]
	protected ResourceName m_sSeizeTaskPrefabName;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Prefab for build FOB task", params: "et", category: "Tasks")]
	protected ResourceName m_sBuildFobTaskPrefabName;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Prefab for destroy all comm nodes task", params: "et", category: "Tasks")]
	protected ResourceName m_sDestroyCommNodesTaskPrefabName;
	
	protected ORS_ObjectiveArea m_pCurrentObjectiveArea;
	
	protected ORS_MissionHeader m_pMissionHeader;
	protected ORS_FactionManager m_pFactionManager;
	
	protected ResourceName m_sCommandPostPrefabName;
	protected ref array<vector> m_aFobPositions = {};
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
		
		SaveGameManager saveManager = GetGame().GetSaveGameManager();
		saveManager.SetEnabledSaveTypes(saveManager.GetEnabledSaveTypes() | ESaveGameType.AUTO | ESaveGameType.SHUTDOWN); // Force auto saves
		if (saveManager.GetActiveSave())
		{
			foreach (ORS_ObjectiveArea area : ORS_ObjectiveArea.GetInstances())
			{
				if (area.GetState() == ORS_EObjectiveAreaState.CONTESTED)
				{
					m_pCurrentObjectiveArea = area;
					break;
				}
			}
			
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
		PolylineShapeEntity outerBorderPicker = PolylineShapeEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sCarrierSpawnOuterBorderPickerName));
		if (!outerBorderPicker)
			return;
		
		PolylineShapeEntity innerBorderPicker = PolylineShapeEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sCarrierSpawnInnerBorderPickerName));
		if (!innerBorderPicker)
			return;
		
		array<vector> outerPoints = {};
		outerBorderPicker.GetPointsPositions(outerPoints);
		KSC_PolygonArea outerBorder = KSC_PolygonArea.FromPoints(outerPoints);
		array<vector> innerPoints = {};
		innerBorderPicker.GetPointsPositions(innerPoints);
		KSC_PolygonArea innerBorder = KSC_PolygonArea.FromPoints(innerPoints);
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
			.SpawnMortar()
			.SpawnMortar()
			.SpawnMortar()
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
		
		ORS_EnemyReinforcementComponent reinforcementComponent = ORS_EnemyReinforcementComponent.Cast(m_pCurrentObjectiveArea.FindComponent(ORS_EnemyReinforcementComponent));
		ORS_EnemyReinforcementSystem reinforcementSystem = ORS_EnemyReinforcementSystem.GetInstance();
		if (reinforcementSystem && reinforcementComponent)
			reinforcementSystem.Register(reinforcementComponent);
		
		ORS_EnemySupportComponent supportComponent = ORS_EnemySupportComponent.Cast(m_pCurrentObjectiveArea.FindComponent(ORS_EnemySupportComponent));
		ORS_EnemySupportSystem supportSystem = ORS_EnemySupportSystem.GetInstance();
		if (supportSystem && supportComponent)
			supportSystem.Register(supportComponent);
		
		GetGame().GetCallqueue().CallLater(GetGame().GetSaveGameManager().RequestSavePoint, 15000, param1: ESaveGameType.AUTO);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateFOBBuildTask(vector pos)
	{
		KSC_BuildTask task = KSC_BuildTask.Cast(KSC_GameTools.SpawnPrefab(m_sBuildFobTaskPrefabName, m_pCurrentObjectiveArea.GetOrigin()));
		task.SetParams(m_pFactionManager.GetPlayerFaction(), m_sCommandPostPrefabName);
		task.GetOnStateChanged().Insert(OnFOBBuildTaskCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFOBBuildTaskCompleted(KSC_BaseTask task, SCR_ETaskState newState)
	{
		if (newState != SCR_ETaskState.COMPLETED)
			return;
		
		task.GetOnStateChanged().Remove(OnFOBBuildTaskCompleted);
		SetUpNextArea();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateSeizeAreaTask()
	{
		KSC_ClearAreaTask task = KSC_ClearAreaTask.Cast(KSC_GameTools.SpawnPrefab(m_sSeizeTaskPrefabName, m_pCurrentObjectiveArea.GetOrigin()));
		task.SetParams(m_pFactionManager.GetPlayerFaction(), m_pCurrentObjectiveArea.GetAreaRadius(), 0.75);
		task.GetOnStateChanged().Insert(OnObjectiveAreaSeized);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectiveAreaSeized(KSC_BaseTask task, SCR_ETaskState newState)
	{
		if (newState != SCR_ETaskState.COMPLETED)
			return;
		
		ORS_EnemySupportComponent supportComponent = ORS_EnemySupportComponent.Cast(m_pCurrentObjectiveArea.FindComponent(ORS_EnemySupportComponent));
		ORS_EnemySupportSystem supportSystem = ORS_EnemySupportSystem.GetInstance();
		if (supportSystem && supportComponent)
			supportSystem.Unregister(supportComponent);
		
		task.GetOnStateChanged().Remove(OnObjectiveAreaSeized);
		m_pDestroyCommNodesTask.GetOnStateChanged().Remove(OnCommNodesDestroyed);
		
		if (m_pDestroyCommNodesTask.GetTaskState() != SCR_ETaskState.COMPLETED)
			SCR_TaskSystem.GetInstance().SetTaskState(m_pDestroyCommNodesTask, SCR_ETaskState.CANCELLED);
		
		m_pCurrentObjectiveArea.SetState(ORS_EObjectiveAreaState.CAPTURED);
		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.AUTO);
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
			
			if (!(info.HasEntityLabel(EEditableEntityLabel.SERVICE_ANTENNA) && info.HasEntityLabel(EEditableEntityLabel.TRAIT_DESTRUCTABLE)) && 
				!(info.HasEntityLabel(EEditableEntityLabel.VEHICLE_TRUCK) && info.HasEntityLabel(EEditableEntityLabel.TRAIT_RADIO))
			)
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
		
		m_pDestroyCommNodesTask = KSC_CounterTask.Cast(KSC_GameTools.SpawnPrefab(m_sDestroyCommNodesTaskPrefabName, m_pCurrentObjectiveArea.GetOrigin() + Vector(100, 0, 0)));
		m_pDestroyCommNodesTask.SetParams(m_pFactionManager.GetPlayerFaction(), 0, damageManagers.Count());
		m_aCommNodes = {};
		
		foreach (SCR_DamageManagerComponent damageManager : damageManagers)
		{
			m_aCommNodes.Insert(new ORS_TargetToDestroyWrapper(damageManager, m_pDestroyCommNodesTask));
		}
		
		m_pDestroyCommNodesTask.GetOnStateChanged().Insert(OnCommNodesDestroyed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disable reinforcements when all comm nodes are destroyed
	void OnCommNodesDestroyed(KSC_BaseTask task, SCR_ETaskState newState)
	{
		if (newState != SCR_ETaskState.COMPLETED)
			return;
		
		task.GetOnStateChanged().Remove(OnCommNodesDestroyed);
		
		m_pCurrentObjectiveArea.RevealEnemyPositions();
		
		ORS_EnemyReinforcementComponent reinforcementComponent = ORS_EnemyReinforcementComponent.Cast(m_pCurrentObjectiveArea.FindComponent(ORS_EnemyReinforcementComponent));
		ORS_EnemyReinforcementSystem reinforcementSystem = ORS_EnemyReinforcementSystem.GetInstance();
		if (reinforcementSystem && reinforcementComponent)
			reinforcementSystem.Unregister(reinforcementComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveArea GetCurrentObjectiveArea()
	{
		return m_pCurrentObjectiveArea;
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
	protected static const ResourceName DESTROY_TASK_PREFAB_NAME = "{08E48A63D4CC7CBE}Prefabs/Tasks/ORS_DestroyTask.et";
	
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
		ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			KSC_DestroyObjectTask task = KSC_DestroyObjectTask.Cast(KSC_GameTools.SpawnPrefab(DESTROY_TASK_PREFAB_NAME, m_pTarget.GetOrigin()));
			SCR_TaskSystem.GetInstance().AddChildTaskTo(m_pTask, task);
			task.SetParams(factionManager.GetPlayerFaction(), m_pTarget);
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
