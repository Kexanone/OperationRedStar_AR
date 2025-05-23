//------------------------------------------------------------------------------------------------
modded class SCR_CampaignBuildingPlacingObstructionEditorComponent : SCR_BaseEditorComponent
{
	//------------------------------------------------------------------------------------------------
	// Check if the preview is at a valid area.
	// Cannot build inside locked AOs
	override bool IsPreviewOutOfRange(SCR_EditorPreviewParams instantPlacingParam, out ENotification outNotification = -1)
	{
		bool result = super.IsPreviewOutOfRange(instantPlacingParam, outNotification);
		if (result)
			return true;
		
		if (!m_AreaTrigger || !m_PreviewEnt)
			return false;
		
		foreach (ORS_ObjectiveArea area : ORS_ObjectiveArea.GetInstances())
		{
			if (area.GetState() != ORS_EObjectiveAreaState.LOCKED)
				continue;
			
			if (vector.DistanceXZ(area.GetOrigin(), m_PreviewEnt.GetOrigin()) <= area.GetAreaRadius())
			{
				outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
				return true;
			}				
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disable obstruction checks
	override protected bool TraceEntityOnPosition(vector position, notnull BaseWorld world, float safeZoneRadius)
	{
		return false;
	}
}
