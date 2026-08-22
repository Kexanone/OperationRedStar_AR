//------------------------------------------------------------------------------------------------
modded class SCR_Global
{
	//------------------------------------------------------------------------------------------------
	//! This method is mostly used for disallowing editor actions outside of the world bound
	//! In ORS the carrier can end up outside of the world bound, so we have to enable building outside
	override static bool IsPositionWithinTerrainBounds(vector pos)
	{
		return true;
	}
}
