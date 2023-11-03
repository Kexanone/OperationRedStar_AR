//------------------------------------------------------------------------------------------------
class ORS_LocationBaseTaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
{
}

//------------------------------------------------------------------------------------------------
class ORS_LocationBaseTaskSupportEntity : SCR_BaseTaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask CreateTask(vector pos)
	{
		SCR_BaseTask task = super.CreateTask();
		LocalizedString taskTitle = FormatTaskTitle(task, pos);
		int taskID = task.GetTaskID();
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
	
	LocalizedString FormatTaskTitle(SCR_BaseTask task, vector pos)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return task.GetTitle();
		
		SCR_EditableEntityComponent nearestLocation = core.FindNearestEntity(pos, EEditableEntityType.COMMENT, EEditableEntityFlag.LOCAL);
		if (!nearestLocation)
			return task.GetTitle();
		
		return string.Format(task.GetTitle(), nearestLocation.GetDisplayName());
		
	}
}