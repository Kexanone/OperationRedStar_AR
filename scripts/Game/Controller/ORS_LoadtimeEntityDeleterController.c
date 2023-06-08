[EntityEditorProps(category: "GameScripted/Controller", description: "Handles client > server communication for deleting loaddtime entities at runtime. Should be attached to PlayerController.", color: "0 0 255 255")]
class ORS_LoadtimeEntityDeleterControllerClass: ScriptComponentClass
{
};

class ORS_LoadtimeEntityDeleterController : ScriptComponent
{
	protected ORS_LoadtimeEntityDeleterComponent m_Deleter;
	protected RplComponent m_Rpl;
	
	override protected void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_Deleter = ORS_LoadtimeEntityDeleterComponent.Cast(GetGame().GetGameMode().FindComponent(ORS_LoadtimeEntityDeleterComponent));
		m_Rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		// Execute on owner only
		if (!m_Rpl || !m_Rpl.IsOwner())
			return;
		
		// Delete entities for JIPs
		GetGame().GetCallqueue().CallLater(OwnerInit);
	};
	
	protected void OwnerInit()
	{
		Rpc(RpcAsk_DeleteInitialEntityPositions);
	};
	
	// Fetches initial entity positions to delete from server and sends it to owner
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeleteInitialEntityPositions()
	{
		Rpc(RpcDo_DeleteInitialEntityPositions, m_Deleter.GetDeletedEntityPositions());
	};
	
	// Deletes initial entity positions received from server
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_DeleteInitialEntityPositions(array<vector> entityPositions)
	{
		m_Deleter.DeleteEntityPositionsLocal(entityPositions);
	};
	
	// Request delete at runtime
	void RequestDeleteEntityPosition(vector pos)
	{
		Rpc(RpcAsk_DeleteEntityPositionServer, pos);
	};
	
	// Request delete at runtime
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeleteEntityPositionServer(vector pos)
	{
		m_Deleter.DeleteEntityPositionsGlobal({pos});
	};
};
