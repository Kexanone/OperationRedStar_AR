//------------------------------------------------------------------------------------------------
class ORS_EnemySupportComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Enemy support data for an ORS_ObjectiveArea
class ORS_EnemySupportComponent : ScriptComponent
{
	protected ref array<ref ORS_EnemySupportContext> m_aSupports = {};
	
	//------------------------------------------------------------------------------------------------
	void AddSupport(ORS_EnemySupportContext context)
	{
		m_aSupports.Insert(context);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveSupport(ORS_EnemySupportContext context)
	{
		m_aSupports.RemoveItem(context);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref ORS_EnemySupportContext> GetAllSupports()
	{
		return m_aSupports;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref ORS_EnemySupportContext> GetSupportsOfType(ORS_EEnemySupportType type)
	{
		array<ref ORS_EnemySupportContext> filteredSupports = {};
		filteredSupports.Reserve(m_aSupports.Count());
		
		foreach (ORS_EnemySupportContext support : m_aSupports)
		{
			if (support.m_eType == type)
				filteredSupports.Insert(support);
		}
		
		return filteredSupports;
	}
	
	//------------------------------------------------------------------------------------------------
	ORS_EnemySupportContext GetRandomSupportOfType(ORS_EEnemySupportType type)
	{
		array<ref ORS_EnemySupportContext> filteredSupports = GetSupportsOfType(type);
		if (filteredSupports.IsEmpty())
			return null;
		
		return filteredSupports.GetRandomElement();
	}
}

//------------------------------------------------------------------------------------------------
class ORS_EnemySupportContext : Managed
{
	ORS_EEnemySupportType m_eType;
	IEntity m_pVehicle;
	AIGroup m_pGroup;
	
	//------------------------------------------------------------------------------------------------
	bool IsAlive()
	{
		if (!m_pGroup)
		{
			if (!m_pVehicle)
				return false;
			
			array<SCR_AIGroup> groups = KSC_VehicleHelper.GetOccupantGroups(m_pVehicle);
			if (groups.IsEmpty())
				return false;
			
			m_pGroup = groups[0];
		}
		
		array<SCR_ChimeraCharacter> units = KSC_GroupHelper.GetUnits(m_pGroup);
		if (units.IsEmpty())
			return false;
		
		foreach (SCR_ChimeraCharacter unit : units)
		{
			if (!unit)
				continue;
			
			if (!unit.GetDamageManager().IsDestroyed())
				return true;
		}
		
		return false;
	}
}

//------------------------------------------------------------------------------------------------
[EnumLinear()]
enum ORS_EEnemySupportType
{
	MORTAR,
	IFV,
	AT,
	INFANTRY
}
