//------------------------------------------------------------------------------------------------
//! Fix writing/reading of task prefab name
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