//------------------------------------------------------------------------------------------------
class ORS_AreaMarker : Managed
{
	protected vector m_vWorldPos;
	protected float m_fWorldRadius;
	protected Color m_Color;
	protected Widget m_wRoot;
	protected Widget m_wOverlay;
	protected ImageWidget m_wImage;
	protected const ResourceName MARKER_LAYOUT_NAME = "{74E89B1FC6349C6B}UI/layouts/Map/ORS_CircleArea.layout";
	protected const string MARKER_IMAGE_NAME = "AreaMarkerImage";
	
	//------------------------------------------------------------------------------------------------
	void ORS_AreaMarker(vector pos, float radius, Color color)
	{
		m_vWorldPos = pos;
		m_fWorldRadius = radius;
		m_Color = color;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetColor(Color color)
	{
		m_Color = color;
		
		if (!m_wImage)
			return;
		
		m_wImage.SetColor(m_Color);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCreate(Widget mapFrame)
	{
		Widget mapWidget = mapFrame.FindAnyWidget("DrawingContainer");
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(MARKER_LAYOUT_NAME, mapWidget);
		m_wImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("AreaMarkerImage"));
		m_wImage.SetColor(m_Color);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnUpdate(SCR_MapEntity mapEntity, float timeSlice)
	{
		float screenX, screenY;
		mapEntity.WorldToScreen(m_vWorldPos[0], m_vWorldPos[2], screenX, screenY, true);
		FrameSlot.SetPos(m_wRoot, GetGame().GetWorkspace().DPIUnscale(screenX), GetGame().GetWorkspace().DPIUnscale(screenY));
		float size = GetGame().GetWorkspace().DPIUnscale(2*m_fWorldRadius) * mapEntity.GetCurrentZoom();
		m_wImage.SetSize(size, size);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRemove(Widget mapFrame)
	{
		if (!m_wRoot)
			return;
		
		m_wRoot.RemoveFromHierarchy();
	}
}