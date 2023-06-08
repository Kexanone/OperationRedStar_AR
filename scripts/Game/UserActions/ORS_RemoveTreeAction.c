class ORS_RemoveTreeAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		PlayerController userCtrl = GetGame().GetPlayerController();
		if (!userCtrl)
			return;
		
		ORS_LoadtimeEntityDeleterController deleterCtrl = ORS_LoadtimeEntityDeleterController.Cast(userCtrl.FindComponent(ORS_LoadtimeEntityDeleterController));
		if (!deleterCtrl)
			return;
		
		deleterCtrl.RequestDeleteEntityPosition(pOwnerEntity.GetOrigin());
	};
	
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	};
};