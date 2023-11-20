//------------------------------------------------------------------------------------------------
modded class SCR_CampaignBuildingPlacingObstructionEditorComponent : SCR_BaseEditorComponent
{
	//------------------------------------------------------------------------------------------------
	// Check if the preview is at a valid area.
	// Can only build if it is inside the current or previous objective area.
	override bool IsPreviewOutOfRange(out ENotification outNotification = -1)
	{
		bool result = super.IsPreviewOutOfRange(outNotification);
		if (result)
			return true;
		
		if (!m_AreaTrigger || !m_PreviewEnt)
			return false;
		
		ORS_GameMode gameMode = ORS_GameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return false;

		ORS_ObjectiveArea currentArea = gameMode.GetCurrentObjectiveArea();
		if (!currentArea)
		{
			outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
			return true;
		};
		
		if (vector.DistanceXZ(m_PreviewEnt.GetOrigin(), currentArea.GetOrigin()) <= currentArea.GetBuildingPlacingRadius())
			return false;
		
		ORS_ObjectiveArea previousArea = gameMode.GetPreviousObjectiveArea();
		if (!previousArea)
		{
			outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
			return true;
		};
		
		
		if (vector.DistanceXZ(m_PreviewEnt.GetOrigin(), previousArea.GetOrigin()) <= previousArea.GetBuildingPlacingRadius())
			return false;
		

		outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
		return true;
	}
}
