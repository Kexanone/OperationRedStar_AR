//------------------------------------------------------------------------------------------------
//! This array behaves like an urn problem
class COE_UrnArray<Class T> : Managed
{
	protected ref array<T> m_aElements = {};
	protected bool m_bShuffled = false;
	
	void Insert(T value)
	{
		m_aElements.Insert(value);
		m_bShuffled = false;
	}
	
	void InsertAll(array<T> values)
	{
		m_aElements.InsertAll(values);
		m_bShuffled = false;
	}
	
	int Count()
	{
		return m_aElements.Count();
	}
	
	T GetRandomElement()
	{
		if (m_aElements.IsEmpty())
			return null;
		
		if (!m_bShuffled)
		{
			COE_ArrayHelperT<T>.Shuffle(m_aElements);
			m_bShuffled = true;
		}
		
		T element = m_aElements.Get(0);
		m_aElements.Remove(0);
		return element;
	}
}

//------------------------------------------------------------------------------------------------
class COE_SlotsManagerComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class COE_SlotsManagerComponent : ScriptComponent
{
	protected vector m_vQueryCenter;
	protected float m_fQueryRadius;
	protected ref COE_UrnArray<SCR_SiteSlotEntity> m_aRoadSlots = new COE_UrnArray<SCR_SiteSlotEntity>();
	protected ref COE_UrnArray<SCR_SiteSlotEntity> m_aFlatSlots = new COE_UrnArray<SCR_SiteSlotEntity>();
	protected ref COE_UrnArray<SCR_SiteSlotEntity> m_aMediumFlatSlots = new COE_UrnArray<SCR_SiteSlotEntity>();
	protected ref COE_UrnArray<COE_AISlotConfig> m_aRiflemanSlots = new COE_UrnArray<COE_AISlotConfig>();
	protected ref COE_UrnArray<COE_AISlotConfig> m_aMarksmanSlots = new COE_UrnArray<COE_AISlotConfig>();
	
	//------------------------------------------------------------------------------------------------
	void Init(vector queryCenter, float queryRadius)
	{
		m_vQueryCenter = queryCenter;
		m_fQueryRadius = queryRadius;
		GetGame().GetWorld().QueryEntitiesBySphere(m_vQueryCenter, m_fQueryRadius, QueryRoadSlotsCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetRandomFlatSlot()
	{
		Math.Randomize(-1);
		return m_aFlatSlots.GetRandomElement();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetRandomMediumFlatSlot()
	{
		Math.Randomize(-1);
		return m_aMediumFlatSlots.GetRandomElement();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetRandomRoadSlot()
	{
		Math.Randomize(-1);
		return m_aRoadSlots.GetRandomElement();		
	}
	
	//------------------------------------------------------------------------------------------------
	COE_AISlotConfig GetRandomMarksmanSlot()
	{
		Math.Randomize(-1);
		return m_aMarksmanSlots.GetRandomElement();		
	}
	
	//------------------------------------------------------------------------------------------------
	COE_AISlotConfig GetRandomRiflemanSlot()
	{
		Math.Randomize(-1);
		return m_aRiflemanSlots.GetRandomElement();		
	}
		
	//------------------------------------------------------------------------------------------------	
	EEditableEntityLabel GetSiteSlotLabel(SCR_SiteSlotEntity slot)
	{
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(slot.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent)
			return EEditableEntityLabel.NONE;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(editableComponent.GetInfo());
		if (!editableEntityUIInfo)
			return EEditableEntityLabel.NONE;
		
		array<EEditableEntityLabel> labels = {};
		editableEntityUIInfo.GetEntityLabels(labels);
		
		if (labels.IsEmpty())
			return EEditableEntityLabel.NONE;
		
		return labels[0];
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryRoadSlotsCallback(IEntity entity)
	{
		SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(entity);
		
		if (!slot)
		{
			COE_AISlotsComponent aiSlot = COE_AISlotsComponent.Cast(entity.FindComponent(COE_AISlotsComponent));
			if (!aiSlot)
				return true;
			
			m_aMarksmanSlots.InsertAll(aiSlot.GetSlotConfigs(COE_EEntityLabel.MARKSMAN));
			m_aRiflemanSlots.InsertAll(aiSlot.GetSlotConfigs(COE_EEntityLabel.RIFLEMAN));
			return true;
		};
		
		if (slot.IsOccupied())
			return true;
		
		EEditableEntityLabel label = GetSiteSlotLabel(slot);
		
		if (label == EEditableEntityLabel.SLOT_ROAD_SMALL || label == EEditableEntityLabel.SLOT_ROAD_MEDIUM || label == EEditableEntityLabel.SLOT_ROAD_LARGE)
		{
			m_aRoadSlots.Insert(slot);
		}
		else if (label == EEditableEntityLabel.SLOT_FLAT_SMALL)
		{
			m_aFlatSlots.Insert(slot);
		}
		else if (label == EEditableEntityLabel.SLOT_FLAT_MEDIUM)
		{
			m_aFlatSlots.Insert(slot);
			m_aMediumFlatSlots.Insert(slot);
		}
		else if (label == EEditableEntityLabel.SLOT_FLAT_LARGE)
		{
			m_aFlatSlots.Insert(slot);
			m_aMediumFlatSlots.Insert(slot);
		};
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void PopulateTurrets()
	{
		GetGame().GetWorld().QueryEntitiesBySphere(m_vQueryCenter, m_fQueryRadius, SpawnTurretOccupantsCallback);
	}
	
		
	//------------------------------------------------------------------------------------------------
	protected bool SpawnTurretOccupantsCallback(IEntity entity)
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(entity.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return true;
				
		compartmentManager.SpawnDefaultOccupants({ECompartmentType.Turret});
		return true;
	}
}
