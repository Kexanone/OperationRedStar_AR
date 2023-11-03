//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class ORS_SessionStruct : SCR_CampaignStruct
{
	protected int m_iCurrentObjectiveIdx;
	
	//------------------------------------------------------------------------------------------------
	void ORS_SessionStruct()
	{
		RegV("m_iCurrentObjectiveIdx");
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
	int GetCurrentObjectiveAreaIdx()
	{
		return m_iCurrentObjectiveIdx;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurrentObjectiveAreaIdx(int idx)
	{
		m_iCurrentObjectiveIdx = idx;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		super.Clear();
		m_iCurrentObjectiveIdx = 0;
	}
}