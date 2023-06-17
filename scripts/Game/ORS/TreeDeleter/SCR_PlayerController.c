modded class SCR_PlayerController : PlayerController
{
	protected SCR_BaseGameMode m_GameMode;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
	};
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void ORS_DeleteInitialEntityPositions(array<vector> entityPositions)
	{
		m_GameMode.ORS_DeleteEntityPositionsLocal(entityPositions);
	};
	
	// Request delete at runtime
	void ORS_RequestDeleteEntityPosition(vector pos)
	{
		Rpc(ORS_RpcAsk_DeleteEntityPositionServer, pos);
	};
	
	// Request delete at runtime
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ORS_RpcAsk_DeleteEntityPositionServer(vector pos)
	{
		m_GameMode.ORS_DeleteEntityPositionsGlobal({pos});
	};
};
