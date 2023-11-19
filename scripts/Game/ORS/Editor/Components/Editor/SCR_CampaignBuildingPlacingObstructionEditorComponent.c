//------------------------------------------------------------------------------------------------
modded class SCR_CampaignBuildingPlacingObstructionEditorComponent : SCR_BaseEditorComponent
{
	//------------------------------------------------------------------------------------------------
	// Check if the preview is outisde of the current objective area.
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

		ORS_ObjectiveArea area = gameMode.GetCurrentObjectiveArea();
		if (!area)
		{
			outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
			return true;
		};
		
		// Can only build if it's in the current objective area
		if (vector.DistanceXZ(m_PreviewEnt.GetOrigin(), area.GetOrigin()) > area.GetBuildingPlacingRadius())
		{
			outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
			return true;
		};

		return false;
	}
}
