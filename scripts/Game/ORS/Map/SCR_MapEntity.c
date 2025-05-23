//------------------------------------------------------------------------------------------------
//! Add handlings for area markers
modded class SCR_MapEntity: MapEntity
{
	protected ref array<ref ORS_AreaMarker> m_aORS_AreaMarkers = {};
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		foreach (ORS_AreaMarker marker : m_aORS_AreaMarkers)
		{
			marker.OnCreate(m_wMapRoot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMapClose()
	{
		super.OnMapClose();
		
		foreach (ORS_AreaMarker marker : m_aORS_AreaMarkers)
		{
			marker.OnRemove(m_wMapRoot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateMap(float timeSlice)
	{
		super.UpdateMap(timeSlice);
							
		foreach (ORS_AreaMarker marker : m_aORS_AreaMarkers)
		{
			marker.OnUpdate(this, timeSlice);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	void ORS_AddAreaMarker(ORS_AreaMarker marker)
	{
		m_aORS_AreaMarkers.Insert(marker);
		
		if (m_bIsOpen)
		{
			marker.OnCreate(m_wMapRoot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ORS_RemoveAreaMarker(ORS_AreaMarker marker)
	{
		m_aORS_AreaMarkers.RemoveItem(marker);
		
		if (m_bIsOpen)
		{
			marker.OnRemove(m_wMapRoot);
		}
	}
}