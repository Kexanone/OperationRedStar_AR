//------------------------------------------------------------------------------------------------
modded class SCR_AIGroup : ChimeraAIGroup
{
	protected bool m_bORS_MembersInitialized = false;
	
	//------------------------------------------------------------------------------------------------
	override protected void InvokeEventOnInit()
	{
		super.InvokeEventOnInit();
		m_bORS_MembersInitialized = true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ORS_AreMembersInitialized()
	{
		return m_bORS_MembersInitialized;
	}
}
