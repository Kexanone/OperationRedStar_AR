//------------------------------------------------------------------------------------------------
class ORS_SubjectBaseTaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class ORS_SubjectBaseTaskSupportEntity : SCR_BaseTaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask CreateTask(IEntity subject)
	{
		SCR_BaseTask task = super.CreateTask();
		LocalizedString taskTitle = FormatTaskTitle(task, subject);
		int taskID = task.GetTaskID();
		vector pos = subject.GetOrigin();
		Rpc(RPC_CreateTask, taskID, pos, taskTitle);
		RPC_CreateTask(taskID, pos, taskTitle);
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateTask(int taskID, vector pos, LocalizedString taskTitle)
	{
		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
		if (!task)
			return;
		
		task.SetOrigin(pos);
		task.SetTitle(taskTitle);
		task.Create();
	}
	
	LocalizedString FormatTaskTitle(SCR_BaseTask task, IEntity subject)
	{
		SCR_EditableEntityComponent edit = SCR_EditableEntityComponent.Cast(subject.FindComponent(SCR_EditableEntityComponent));
		return string.Format(task.GetTitle(), edit.GetDisplayName());
		
	}
};