//------------------------------------------------------------------------------------------------
class ORS_ObjectiveAreaBuilder : Managed
{
	protected static KSC_WorldSlotsConfig s_WorldSlotsConfig;
	protected static ORS_GameMode s_pGameMode;
	protected static ORS_FactionManager s_pFactionManager;
	protected static SCR_GarbageSystem s_pGarabageSystem;
	protected static ref map<EEditableEntityLabel, float> s_mSlotRadii;
	
	protected ORS_ObjectiveArea m_pObjectiveArea;
	protected ref map<EEditableEntityLabel, ref array<vector>> m_mTerrainSlots;
	
	protected static const ResourceName FIRE_DIRECTION_CENTER_PREFAB_NAME = "{3FBC7F5559D51E55}Prefabs/Compositions/Installation/FIA/ORS_FireDirectionCenter.et";
	protected static const ResourceName VEHICLE_BASE_PREFAB_NAME = "{4085446E2B406849}Prefabs/Vehicles/Core/Vehicle_Base.et";
	protected static const float OCCUPIED_SLOT_BLOCKING_RADIUS = 50;
	
	//------------------------------------------------------------------------------------------------
	void ORS_ObjectiveAreaBuilder(ORS_ObjectiveArea objectiveArea)
	{
		if (!s_WorldSlotsConfig)
		{
			s_pGameMode = ORS_GameMode.GetInstance();
			s_WorldSlotsConfig = s_pGameMode.KSC_GetWorldSlotsConfig();
			s_pFactionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
			ChimeraWorld world = GetGame().GetWorld();
			s_pGarabageSystem = SCR_GarbageSystem.Cast(world.FindSystem(SCR_GarbageSystem));
		}
		
		m_pObjectiveArea = objectiveArea;
		m_mTerrainSlots = s_WorldSlotsConfig.CompileTerrainSlotsInArea(m_pObjectiveArea.GetArea());
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveAreaBuilder SpawnFireDirectionCenter()
	{
		IEntity structure = KSC_TerrainSlotTools.SpawnInRandomFlatSlot(FIRE_DIRECTION_CENTER_PREFAB_NAME, m_mTerrainSlots, EEditableEntityLabel.SLOT_FLAT_MEDIUM);
		
		if (structure)
			KSC_TerrainSlotTools.BlockSlots(m_mTerrainSlots, structure.GetOrigin(), OCCUPIED_SLOT_BLOCKING_RADIUS);
		
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveAreaBuilder SpawnCommNode()
	{
		array<ResourceName> prefabNames = {};
		s_pFactionManager.GetFactionEntityListWithLabel(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, EEditableEntityLabel.KSC_TRAIT_HVT, prefabNames);
		//s_pFactionManager.GetFactionEntityListWithLabel(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.VEHICLE, EEditableEntityLabel.TRAIT_RADIO, prefabNames);
		if (prefabNames.IsEmpty())
			return this;
		
		ResourceName prefabName = prefabNames.GetRandomElement();
		vector pos;
		
		if (SCR_BaseContainerTools.IsKindOf(prefabName, VEHICLE_BASE_PREFAB_NAME))
		{
			float yaw;
			
			if (!KSC_TerrainSlotTools.GetRandomParkingSlot(pos, yaw, m_mTerrainSlots))
				return this;
			
			IEntity vehicle = KSC_GameTools.SpawnVehiclePrefab(prefabName, pos, yaw);
			AIGroup group = SpawnGroup(vehicle.GetOrigin() - 10 * vector.FromYaw(yaw), {EEditableEntityLabel.GROUPSIZE_LARGE});
			KSC_AITasks.Defend(group, vehicle.GetOrigin() - 10 * vector.FromYaw(yaw), KSC_TerrainSlotTools.s_mSlotRadii[EEditableEntityLabel.SLOT_FLAT_LARGE]);
		}
		else
		{
			IEntity node = KSC_TerrainSlotTools.SpawnInRandomFlatSlot(prefabName, m_mTerrainSlots, EEditableEntityLabel.SLOT_FLAT_SMALL);
			pos = node.GetOrigin();
			AIGroup group = SpawnGroup(pos, {EEditableEntityLabel.GROUPSIZE_MEDIUM});
			KSC_AITasks.Defend(group, pos, KSC_TerrainSlotTools.s_mSlotRadii[EEditableEntityLabel.SLOT_FLAT_SMALL]);
			
			float blockRadius = KSC_TerrainSlotTools.s_mSlotRadii[EEditableEntityLabel.SLOT_FLAT_SMALL];
			map<EEditableEntityLabel, ref array<vector>> nearbyTerrainSlots = s_WorldSlotsConfig.CompileTerrainSlotsInArea(KSC_CircleArea(pos, OCCUPIED_SLOT_BLOCKING_RADIUS - blockRadius));
			KSC_TerrainSlotTools.BlockSlots(nearbyTerrainSlots, pos, blockRadius);
			prefabNames.Clear();
			s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, {EEditableEntityLabel.TRAIT_FORTIFICATION, EEditableEntityLabel.SLOT_FLAT_SMALL}, prefabNames);
			
			if (!prefabNames.IsEmpty())
			{
				for (int i = 0; i < 3; i++)
				{
					IEntity fortification = KSC_TerrainSlotTools.SpawnInRandomFlatSlot(prefabNames.GetRandomElement(), nearbyTerrainSlots, EEditableEntityLabel.SLOT_FLAT_SMALL, pos);
					if (!fortification)
						break;
					
					vector center = fortification.GetOrigin();
					float radius = KSC_TerrainSlotTools.s_mSlotRadii[EEditableEntityLabel.SLOT_FLAT_SMALL];
					KSC_TerrainSlotTools.BlockSlots(m_mTerrainSlots, center, radius);
					group = SpawnGroup(center, {EEditableEntityLabel.GROUPSIZE_SMALL});
					KSC_AITasks.Defend(group, center, radius);
				}
			}
		}
		
		KSC_TerrainSlotTools.BlockSlots(m_mTerrainSlots, pos, OCCUPIED_SLOT_BLOCKING_RADIUS);
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveAreaBuilder SpawnServiceStructure()
	{
		array<ResourceName> entries = {};
		s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, {EEditableEntityLabel.TRAIT_SUPPLYSTORAGE_LARGE, EEditableEntityLabel.SLOT_FLAT_MEDIUM}, entries);
		s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, {EEditableEntityLabel.TRAIT_SUPPLYSTORAGE_LARGE, EEditableEntityLabel.SLOT_FLAT_LARGE}, entries);
		
		if (entries.IsEmpty())
			return this;
		
		IEntity service = KSC_TerrainSlotTools.SpawnInRandomFlatSlot(entries.GetRandomElement(), m_mTerrainSlots, EEditableEntityLabel.SLOT_FLAT_LARGE);
		// Search for medium slots instead if failed
		if (!service)
		{
			entries.Clear();
			s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, {EEditableEntityLabel.TRAIT_SUPPLYSTORAGE_LARGE, EEditableEntityLabel.SLOT_FLAT_MEDIUM}, entries);
			
			if (entries.IsEmpty())
				return this;
			
			service = KSC_TerrainSlotTools.SpawnInRandomFlatSlot(entries.GetRandomElement(), m_mTerrainSlots, EEditableEntityLabel.SLOT_FLAT_MEDIUM);
		}
		
		if (!service)
			return this;
		
		KSC_SuppliesHelper.SetSupplyPercentage(service, 1);
		
		AIGroup group = SpawnGroup(service.GetOrigin(), {EEditableEntityLabel.GROUPSIZE_LARGE});
		if (group)
			KSC_AITasks.Defend(group, service.GetOrigin(), KSC_TerrainSlotTools.s_mSlotRadii[EEditableEntityLabel.SLOT_FLAT_LARGE]);
		
		entries.Clear();
		s_pFactionManager.GetFactionEntityListWithLabel(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.VEHICLE, EEditableEntityLabel.VEHICLE_CAR, entries);
		s_pFactionManager.GetFactionEntityListWithLabel(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.VEHICLE, EEditableEntityLabel.VEHICLE_TRUCK, entries);
		
		if (!entries.IsEmpty())
		{
			vector pos = service.GetOrigin();
			if (SCR_WorldTools.FindEmptyTerrainPosition(pos, pos, OCCUPIED_SLOT_BLOCKING_RADIUS, 5))
			{
				IEntity vehicle = KSC_GameTools.SpawnVehiclePrefab(entries.GetRandomElement(), pos, Math.RandomFloat(0, 360));
				if (vehicle)
				{
					KSC_SuppliesHelper.SetSupplyPercentage(vehicle, 1);
					s_pGarabageSystem.Withdraw(vehicle);
				}
			}
		}
		
		KSC_TerrainSlotTools.BlockSlots(m_mTerrainSlots, service.GetOrigin(), OCCUPIED_SLOT_BLOCKING_RADIUS);		
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveAreaBuilder SpawnFortification()
	{
		array<ResourceName> prefabNames = {};
		s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, {EEditableEntityLabel.TRAIT_FORTIFICATION, EEditableEntityLabel.SLOT_FLAT_SMALL}, prefabNames);
		if (prefabNames.IsEmpty())
			return this;
		
		KSC_TerrainSlotTools.SpawnInRandomFlatSlot(prefabNames.GetRandomElement(), m_mTerrainSlots, EEditableEntityLabel.SLOT_FLAT_SMALL);		
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveAreaBuilder SpawnCheckpoint()
	{
		IEntity checkpoint;
		array<EEditableEntityLabel> labels = {EEditableEntityLabel.SLOT_ROAD_SMALL, EEditableEntityLabel.SLOT_ROAD_MEDIUM, EEditableEntityLabel.SLOT_ROAD_LARGE};
		Math.Randomize(-1);
		SCR_ArrayHelperT<EEditableEntityLabel>.Shuffle(labels);
		
		foreach (EEditableEntityLabel label : labels)
		{
			array<ResourceName> entries = {};
			s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.COMPOSITION, {EEditableEntityLabel.TRAIT_ARMED, label}, entries);
			
			if (entries.IsEmpty())
				continue;
			
			checkpoint = KSC_TerrainSlotTools.SpawnInRandomRoadSlot(entries.GetRandomElement(), m_mTerrainSlots, label);
			if (!checkpoint)
				continue;
			
			AIGroup group = SpawnGroup(checkpoint.GetOrigin(), {EEditableEntityLabel.TRAIT_ARMORPIERCING});	
			KSC_AITasks.Defend(group, checkpoint.GetOrigin(), KSC_TerrainSlotTools.s_mSlotRadii[label]);
			break;
		}
		
		if (checkpoint)
			KSC_TerrainSlotTools.BlockSlots(m_mTerrainSlots, checkpoint.GetOrigin(), OCCUPIED_SLOT_BLOCKING_RADIUS);
		
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_ObjectiveAreaBuilder SpawnArmoredVehicle()
	{
		array<ResourceName> prefabNames = {};
		s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.VEHICLE, {EEditableEntityLabel.TRAIT_ARMOR, EEditableEntityLabel.TRAIT_ARMED}, prefabNames);
		
		vector pos;
		float yaw;
		
		if (!KSC_TerrainSlotTools.GetRandomParkingSlot(pos, yaw, m_mTerrainSlots))
			return this;
		
		IEntity armor = KSC_GameTools.SpawnVehiclePrefab(prefabNames.GetRandomElement(), pos, yaw);
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(armor.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (compartmentManager)
			compartmentManager.SpawnDefaultOccupants({ECompartmentType.TURRET});
		
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	protected AIGroup SpawnGroup(vector pos, array<EEditableEntityLabel> labels)
	{
		array<ResourceName> entries = {};
		s_pFactionManager.GetFactionEntityListWithLabels(s_pFactionManager.GetEnemyFaction(), EEntityCatalogType.GROUP, labels, entries);
		if (!entries)
			return null;
		
		SCR_WorldTools.FindEmptyTerrainPosition(pos, pos, 25);
		AIGroup group = KSC_GameTools.SpawnGroupPrefab(entries.GetRandomElement(), pos);
		m_pObjectiveArea.AddAIGroup(group);
		return group;
	}
}
