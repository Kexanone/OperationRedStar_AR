/*
	Methods for requesting deletion of loadtime entities from a client
*/

modded class SCR_PlayerController : PlayerController
{
	// Request delete at runtime
	// Can only be called on the owner
	void ORS_RequestDeleteEntityPosition(vector pos)
	{
		Rpc(ORS_RpcAsk_DeleteEntityPositionServer, pos);
	};
	
	// Request delete at runtime
	// Can only be called on the server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ORS_RpcAsk_DeleteEntityPositionServer(vector pos)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.ORS_DeleteEntityPositionsGlobal({pos});
	};
};
