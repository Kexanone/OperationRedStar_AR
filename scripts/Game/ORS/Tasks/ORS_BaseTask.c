modded class SCR_BaseTaskManager : GenericEntity
{
	//------------------------------------------------------------------------------------------------
	override void LoadTasksForRpl(ScriptBitReader reader, int count)
	{
		reader.ReadInt(count);
		
		ResourceName resourceName;
		Resource resource;
		SCR_BaseTask task;
		bool readTask;
		for (int i = 0; i < count; i++)
		{
			reader.ReadBool(readTask);
			if (!readTask)
				continue;
			
			reader.ReadString(resourceName);
			
			resource = Resource.Load(resourceName);
			if (!resource.IsValid())
				continue;
			
			task = SCR_BaseTask.Cast(GetGame().SpawnEntityPrefab(resource, GetWorld()));
			if (!task)
				continue;
			
			task.Deserialize(reader);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void SaveTasksForRpl(ScriptBitWriter writer, array<SCR_BaseTask> taskArray)
	{
		int count = taskArray.Count();
		writer.WriteInt(count);
		
		for (int i = 0; i < count; i++)
		{
			SCR_BaseTask task = taskArray[i];
			if (!task || task.FindComponent(RplComponent)) //--- Assume that tasks with their own replication will sync data themselves
			{
				writer.WriteBool(false);
				continue;
			}
			
			writer.WriteBool(true);
			
			EntityPrefabData prefabData = task.GetPrefabData();
			ResourceName resourceName;
			if (prefabData)
				resourceName = prefabData.GetPrefabName();
			
			writer.WriteString(prefabData.GetPrefabName()); //Write prefab, then read it in load & spawn correct task
			
			task.Serialize(writer);
		}
	}
}

//------------------------------------------------------------------------------------------------
class ORS_BaseTaskClass: SCR_BaseTaskClass
{
}

//------------------------------------------------------------------------------------------------
//! Basically SCR_EditorTask, but with custom titles
class ORS_BaseTask : SCR_BaseTask
{
	//------------------------------------------------------------------------------------------------
	protected void PopUpNotification(string prefix, bool alwaysInEditor)
	{
		//--- Get player faction (prioritize respawn faction, because it's defined even when player is waiting for respawn)
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();		
				
		if (!playerFaction)
			playerFaction = SCR_PlayerController.GetLocalMainEntityFaction();
		
		//--- Show notification when player is assigned, of the same faction, or has unlimited editor (i.e., is Game Master)
		if (IsAssignedToLocalPlayer() || playerFaction == GetTargetFaction() || (alwaysInEditor && !SCR_EditorManagerEntity.IsLimitedInstance()))
		{
			//--- SCR_PopUpNotification.GetInstance() is never null, as it creates the instance if it doesn't exist yet
			SCR_PopUpNotification.GetInstance().PopupMsg(prefix + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: SCR_SoundEvent.TASK_SUCCEED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowPopUpNotification(string subtitle)
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(GetTitle(), text2: subtitle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show notification related to the task state. Delete notification is called by SCR_EditableTaskComponent
	//! param taskNotification notification to show
	void ShowTaskNotification(ENotification taskNotification, bool SendOverNetwork = false)
	{
		SCR_EditableEntityComponent editableTask = SCR_EditableEntityComponent.Cast(FindComponent(SCR_EditableEntityComponent));
		if (!editableTask)
			return;
	
		int taskID = Replication.FindId(editableTask);
		
		Faction faction = GetTargetFaction();
		if (!faction)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		vector position;
	 	editableTask.GetPos(position);
		
		int factionIndex = factionManager.GetFactionIndex(faction);
		
		//Send local GM
		if (!SendOverNetwork)
		{
			if (taskNotification == ENotification.EDITOR_TASK_PLACED)
				GetGame().GetCallqueue().CallLater(DelayedPlacedNotification, 1, false, position, taskID, factionIndex);
			else
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(taskNotification, position, taskID, factionIndex);
		}
		else 
		{
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(taskNotification, position, taskID, factionIndex);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		// Delete the task once it's finished
		if (newState == SCR_TaskState.FINISHED || newState == SCR_TaskState.CANCELLED)
		{
			GetTaskManager().DeleteTask(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedPlacedNotification(vector position, int taskID, int factionIndex)
	{
		SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.EDITOR_TASK_PLACED, position, taskID, factionIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Create(bool showMsg = true)
	{
		super.Create(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_AVAILABLE_TEXT, true);
			
		ShowTaskNotification(ENotification.EDITOR_TASK_PLACED);	
	}
	
	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		super.Finish(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_COMPLETED_TEXT, true);
			
		ShowTaskNotification(ENotification.EDITOR_TASK_COMPLETED);
			
	}
	
	//------------------------------------------------------------------------------------------------
	override void Fail(bool showMsg = true)
	{
		super.Fail(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_FAILED_TEXT, true);
			
		ShowTaskNotification(ENotification.EDITOR_TASK_FAILED);
			
	}
	
	//------------------------------------------------------------------------------------------------
	override void Cancel(bool showMsg = true)
	{
		super.Cancel(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_CANCELLED_TEXT, true);
		
		ShowTaskNotification(ENotification.EDITOR_TASK_CANCELED);	
	}
};