//------------------------------------------------------------------------------------------------
class ORS_DestroyTaskClass: ORS_BaseTaskClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_DestroyTask : ORS_BaseTask
{
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		ORS_DestroyTaskSupportEntity supportEntity = ORS_DestroyTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ORS_DestroyTaskSupportEntity));
		if (!supportEntity)
			return;
		
		supportEntity.FinishTask(this);				
	}
}