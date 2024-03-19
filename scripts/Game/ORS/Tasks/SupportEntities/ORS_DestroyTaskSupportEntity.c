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
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.Cast(subject.FindComponent(SCR_DamageManagerComponent));
		
		// Set subjet to child if parent is not destructible
		if (!objectDmgManager)
		{
			subject = subject.GetChildren();
			objectDmgManager = SCR_DamageManagerComponent.Cast(subject.FindComponent(SCR_DamageManagerComponent));
		};
		
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(task.OnObjectDamage);
		
		return task;
	}
};