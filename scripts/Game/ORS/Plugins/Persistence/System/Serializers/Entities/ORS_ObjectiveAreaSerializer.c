//------------------------------------------------------------------------------------------------
class ORS_ObjectiveAreaSerializer : GenericEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ORS_ObjectiveArea;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull SaveContext context)
	{
		const ORS_ObjectiveArea area = ORS_ObjectiveArea.Cast(entity);
		context.WriteValue("version", 1);
		context.WriteValue("state", area.GetState());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull LoadContext context)
	{
		ORS_ObjectiveArea area = ORS_ObjectiveArea.Cast(entity);

		int version;
		context.Read(version);
		
		ORS_EObjectiveAreaState state;
		context.Read(state);
		area.SetState(state);
		return true;
	}
}
