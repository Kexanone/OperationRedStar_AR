/*
	Tree deletion user action
*/

class ORS_RemoveTreeAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		PlayerController userCtrl = GetGame().GetPlayerController();
		if (!userCtrl)
			return;
		
		SCR_PlayerController deleterCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!deleterCtrl)
			return;
		
		deleterCtrl.ORS_RequestDeleteEntityPosition(pOwnerEntity.GetOrigin());
	};
	
	// Trees have no RplComponent, hence only local scripts will work
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	};
};
