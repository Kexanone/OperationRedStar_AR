//------------------------------------------------------------------------------------------------
class ORS_BaseTaskClass: SCR_BaseTaskClass
{
}

//------------------------------------------------------------------------------------------------
//! Basically SCR_EditorTask, but with custom titles
class ORS_BaseTask : SCR_BaseTask
{
	protected string m_sFormatParam1 = string.Empty;
	protected string m_sFormatParam2 = string.Empty;
	protected string m_sFormatParam3 = string.Empty;
	
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
			SCR_PopUpNotification.GetInstance().PopupMsg(prefix + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_sFormatParam1, param2: m_sFormatParam2, param3: m_sFormatParam3, sound: SCR_SoundEvent.TASK_SUCCEED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowPopUpNotification(string subtitle)
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(GetTitle(), text2: subtitle, param1: m_sFormatParam1, param2: m_sFormatParam2, param3: m_sFormatParam3);
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
	
	//------------------------------------------------------------------------------------------------
	void SetFormatParams(string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty)
	{
		m_sFormatParam1 = param1;
		m_sFormatParam2 = param2;
		m_sFormatParam3 = param3;
		PrintFormat("|||a|%1|%2|%3||||", m_sFormatParam1, m_sFormatParam2, m_sFormatParam3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
	{
		PrintFormat("|||%1|%2|%3|%4|||", taskText, m_sFormatParam1, m_sFormatParam2, m_sFormatParam3);
		textWidget.SetTextFormat(taskText, m_sFormatParam1, m_sFormatParam2, m_sFormatParam3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
	{
		textWidget.SetTextFormat(taskText, m_sFormatParam1, m_sFormatParam2, m_sFormatParam3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		writer.WriteString(m_sFormatParam1);
		writer.WriteString(m_sFormatParam2);
		writer.WriteString(m_sFormatParam3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		reader.ReadString(m_sFormatParam1);
		reader.ReadString(m_sFormatParam2);
		reader.ReadString(m_sFormatParam3);
	}
}