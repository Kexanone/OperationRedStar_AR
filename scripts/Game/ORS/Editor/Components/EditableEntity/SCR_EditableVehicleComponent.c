//------------------------------------------------------------------------------------------------
modded class SCR_EditableVehicleComponent : SCR_EditableEntityComponent
{
	protected static const float HELICOPTER_SPAWN_POS_TRACER_LENGTH_M = 5;
	
	//------------------------------------------------------------------------------------------------
	//! Fix spawning of helos on carrier
	override void OnCreatedServer(notnull SCR_PlacingEditorComponent placedEditorComponent)
	{
		super.OnCreatedServer(placedEditorComponent);
		
		if (!GetOwner().FindComponent(SCR_HelicopterSoundComponent))
			return;
		
		vector transform[4];
		GetTransform(transform);
		TraceParam params = new TraceParam();
		params.Exclude = GetOwner();
		params.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		params.Start = transform[3] + HELICOPTER_SPAWN_POS_TRACER_LENGTH_M * vector.Up;
		params.End = transform[3];
		
		float percentage = GetGame().GetWorld().TraceMove(params, null);
		transform[3] = transform[3] + (1 - percentage) * HELICOPTER_SPAWN_POS_TRACER_LENGTH_M * vector.Up;
		SetTransform(transform);
		
		Physics physics = GetOwner().GetPhysics();
		if (physics)
			physics.SetActive(ActiveState.ACTIVE);
	}
}
