//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class ORS_SessionStruct : SCR_JsonApiStruct
{
	protected ref array<int> m_aObjectiveAreaStates;
	
	//------------------------------------------------------------------------------------------------
	void ORS_SessionStruct()
	{
		RegV("m_aObjectiveAreaStates");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		super.Serialize();
		ORS_GameMode.Cast(GetGame().GetGameMode()).SaveSessionData(this);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		super.Deserialize();
		ORS_GameMode.Cast(GetGame().GetGameMode()).LoadSessionData(this);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	array<int> GetObjectiveAreaStates()
	{
		return m_aObjectiveAreaStates;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetObjectiveAreaStates(array<int> states)
	{
		m_aObjectiveAreaStates = states;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ClearCache()
	{
		super.ClearCache();
		
		if (m_aObjectiveAreaStates)
			m_aObjectiveAreaStates.Clear();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class ORS_EmptyStruct: SCR_JsonApiStruct
{
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		return true;
	}
}
