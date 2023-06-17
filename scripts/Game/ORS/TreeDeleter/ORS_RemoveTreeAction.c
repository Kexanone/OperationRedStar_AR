/*
	Tree deletion user action
*/

class ORS_RemoveTreeAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		SCR_PlayerController userCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!userCtrl)
			return;
		
		userCtrl.Rpc(userCtrl.ORS_RpcAsk_DeleteEntityPosition, pOwnerEntity.GetOrigin());
	};
	
	// Trees have no RplComponent, hence only local scripts will work
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	};
};
