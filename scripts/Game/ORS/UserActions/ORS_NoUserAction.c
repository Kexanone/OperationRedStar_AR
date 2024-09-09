//------------------------------------------------------------------------------------------------
//! Can be used to disable user actions when overriding a prefab
class ORS_NoUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return false;
	}
}
