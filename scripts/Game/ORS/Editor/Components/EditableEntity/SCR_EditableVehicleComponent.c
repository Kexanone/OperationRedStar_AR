//------------------------------------------------------------------------------------------------
modded class SCR_EditableVehicleComponent : SCR_EditableEntityComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnCreatedServer(notnull SCR_PlacingEditorComponent placedEditorComponent)
	{
		super.OnCreatedServer(placedEditorComponent);
		
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;
		
		physics.SetActive(ActiveState.ACTIVE);
	}
}
