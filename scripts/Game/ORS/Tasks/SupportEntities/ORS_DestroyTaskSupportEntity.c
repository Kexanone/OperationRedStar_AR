//------------------------------------------------------------------------------------------------
class ORS_DestroyTaskSupportEntityClass: ORS_SubjectBaseTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class ORS_DestroyTaskSupportEntity : ORS_SubjectBaseTaskSupportEntity
{
	override SCR_BaseTask CreateTask(IEntity subject)
	{
		ORS_DestroyTask task = ORS_DestroyTask.Cast(super.CreateTask(subject));
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(subject.FindComponent(ScriptedDamageManagerComponent));
		
		// Set subjet to child if parent is not destructible
		if (!objectDmgManager)
		{
			subject = subject.GetChildren();
			objectDmgManager = ScriptedDamageManagerComponent.Cast(subject.FindComponent(ScriptedDamageManagerComponent));
		};
		
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(task.OnObjectDamage);
		
		return task;
	}
};