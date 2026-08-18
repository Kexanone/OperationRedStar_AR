//------------------------------------------------------------------------------
class ORS_MapObjectiveAreaMarkersUI : SCR_MapUIBaseComponent
{
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1", desc: "Alpha value of the area marker color")]
	protected float m_fColorAlpha;
	
	[Attribute(defvalue: "0.5 0 0.5 0.5", uiwidget: UIWidgets.ColorPicker, desc: "Color of objective area marker when contested")]
	protected ref Color m_ContestedColor;
	protected ref Color m_LockedColor;
	protected ref Color m_CapturedColor;	
	
	[Attribute(defvalue: "32", params: "3 inf", desc: "Number of verices a circle should be approximated with")]
	protected int m_iCircleVertexCount;
	
	protected CanvasWidget m_Canvas;
	protected ref array<ref CanvasWidgetCommand> m_DrawingCommands;
		
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		m_Canvas = CanvasWidget.Cast(config.RootWidgetRef.FindAnyWidget("ORS_Canvas"));
		m_DrawingCommands = {};
		m_DrawingCommands.Reserve(ORS_ObjectiveArea.GetInstances().Count());
		
		ORS_FactionManager factionManager = ORS_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
			
		m_LockedColor = factionManager.GetEnemyFaction().GetFactionColor();
		m_LockedColor.SetA(m_fColorAlpha);
		m_CapturedColor = factionManager.GetPlayerFaction().GetFactionColor();
		m_CapturedColor.SetA(m_fColorAlpha);
	}			
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		m_DrawingCommands.Clear();

		foreach (ORS_ObjectiveArea area : ORS_ObjectiveArea.GetInstances())
		{
			CanvasWidgetCommand command = GetDrawCommand(area);
			if (command)
				m_DrawingCommands.Insert(command);
		}
			
		m_Canvas.SetDrawCommands(m_DrawingCommands);
	}
	
	//------------------------------------------------------------------------------------------------
	protected PolygonDrawCommand GetDrawCommand(ORS_ObjectiveArea area)
	{
		PolygonDrawCommand command = new PolygonDrawCommand();	
		
		switch (area.GetState())
		{
			case ORS_EObjectiveAreaState.LOCKED: {command.m_iColor = m_LockedColor.PackToInt(); break; }
			case ORS_EObjectiveAreaState.CONTESTED: {command.m_iColor = m_ContestedColor.PackToInt(); break; }
			case ORS_EObjectiveAreaState.CAPTURED: {command.m_iColor = m_CapturedColor.PackToInt(); break; }
		}
		
		vector center = area.GetOrigin();		
		int x, y;
		m_MapEntity.WorldToScreen(center[0], center[2], x, y, true);
		float radius = area.GetAreaRadius() * m_MapEntity.GetCurrentZoom();
		command.m_Vertices = {};
		
		for (float angle = 0; angle < Math.PI2; angle += Math.PI2 / m_iCircleVertexCount)
		{
			command.m_Vertices.Insert(x + radius * Math.Cos(angle));
			command.m_Vertices.Insert(y + radius * Math.Sin(angle));
		}
		
		return command;
	}
}
